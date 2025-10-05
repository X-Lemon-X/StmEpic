#include "memory_fram.hpp"
#include "device.hpp"
#include "sha256.hpp"
#include "stmepic.hpp"
#include "status.hpp"
#include <cstdint>
#include <cstdlib>

using namespace stmepic::memory;
using namespace stmepic;


Status FRAM::write(uint32_t address, uint8_t *data, size_t length) {
  if(data == nullptr)
    return Status::Invalid("FRAM, Data to write is null");
  if(length == 0)
    return Status::CapacityError("FRAM, length of data to write is 0");

  std::unique_ptr<uint8_t[]> data_ptr(new uint8_t[length + data_frame_size]);
  if(data_ptr == nullptr)
    return Status::OutOfMemory("FRAM, Could not allocate memory for encoding of data ");

  STMEPIC_RETURN_ON_ERROR(encode_data(data, length, data_ptr.get()));
  return write_raw(address, data_ptr.get(), length + data_frame_size);
}

Result<std::pair<std::shared_ptr<uint8_t[]>, size_t>> FRAM::read(uint32_t address) {
  // read the frame size first to know how much data to read
  uint8_t frame_data_ptr[data_frame_size];
  // read size of the data
  STMEPIC_RETURN_ON_ERROR(read_raw(address, frame_data_ptr, data_frame_size));
  size_t size = frame_data_ptr[frame_offset_size] | (frame_data_ptr[frame_offset_size + 1] << 8);
  if(size == 0)
    return Status::CapacityError("FRAM, Size of the data to read is 0");
  std::shared_ptr<uint8_t[]> data_ptr(new uint8_t[size]);
  if(data_ptr == nullptr)
    return Status::OutOfMemory("FRAM, Could not allocate memory for decoding of data ");
  // read the data
  STMEPIC_RETURN_ON_ERROR(read_raw(address, data_ptr.get(), size));
  // decode the data
  STMEPIC_RETURN_ON_ERROR(decode_data(frame_data_ptr, data_ptr.get(), size));
  return Result<std::pair<std::shared_ptr<uint8_t[]>, size_t>>::OK(std::make_pair(std::move(data_ptr), size));
}


Status FRAM::encode_data(const uint8_t *data_src, size_t length, uint8_t *data_dest) {
  if(length == 0)
    return Status::CapacityError("Size of the data is 0");
  if(data_dest == nullptr)
    return Status::Invalid("Destination data pointer is null");
  if(data_src == nullptr)
    return Status::Invalid("Data to encode is null");


  uint16_t checksum = calculate_checksum(data_src, length);
  data_dest[0]      = magic_number_1;
  data_dest[1]      = (checksum >> 8) & 0xFF;
  data_dest[2]      = checksum & 0xFF;
  data_dest[7]      = (length >> 8) & 0xFF;
  data_dest[8]      = length & 0xFF;
  data_dest[9]      = magic_number_2;

  // if encryption is disabled
  if(encryption_key == FRAM::base_encryption_key) {
    data_dest[3] = 0;
    data_dest[4] = 0;
    data_dest[5] = 0;
    data_dest[6] = 0;
    std::memcpy(data_dest + data_frame_size, data_src, length);
    return Status::OK();
  }

  uint32_t encres = stmepic::Ticker::get_instance().get_micros();
  data_dest[3]    = (encres >> 24) & 0xFF;
  data_dest[4]    = (encres >> 16) & 0xFF;
  data_dest[5]    = (encres >> 8) & 0xFF;
  data_dest[6]    = encres & 0xFF;
  STMEPIC_RETURN_ON_ERROR(encrypt_data(data_dest + 3, 4, encryption_key));
  std::string key = std::to_string(encres) + encryption_key;
  std::memcpy(data_dest + data_frame_size, data_src, length);
  STMEPIC_RETURN_ON_ERROR(encrypt_data(data_dest + data_frame_size, length, key));
  return Status::OK();
}

Status FRAM::decode_data(uint8_t *frame_data_ptr, uint8_t *data_src, size_t length) {
  if(length == 0)
    return Status::CapacityError("Size of the data is 0");
  if(frame_data_ptr == nullptr)
    return Status::Invalid("Frame data pointer is null");
  if(data_src == nullptr)
    return Status::Invalid("Data to decode is null");

  uint8_t mg1       = frame_data_ptr[0];
  uint16_t checksum = (uint16_t)(frame_data_ptr[1] << 8) | frame_data_ptr[2];
  uint16_t size     = (uint16_t)(frame_data_ptr[7] << 8) | frame_data_ptr[8];
  uint8_t mg2       = frame_data_ptr[9];
  if(mg1 != magic_number_1)
    return Status::Invalid("Magic number 1 is not correct");
  if(mg2 != magic_number_2)
    return Status::Invalid("Magic number 2 is not correct");
  if(size != length)
    return Status::Invalid("Size is not correct");

  // if encryption is disabled
  if(encryption_key == FRAM::base_encryption_key) {
    if(calculate_checksum(data_src, length) != checksum)
      return Status::Invalid("Checksum is not correct");
    return Status::OK();
  }

  STMEPIC_RETURN_ON_ERROR(decrypt_data(frame_data_ptr + 3, 4, encryption_key));
  uint32_t encres =
  (frame_data_ptr[3] << 24) | (frame_data_ptr[4] << 16) | (frame_data_ptr[5] << 8) | frame_data_ptr[6];
  std::string key = std::to_string(encres) + encryption_key;
  STMEPIC_RETURN_ON_ERROR(decrypt_data(data_src, length, key));
  if(calculate_checksum(data_src, length) != checksum)
    return Status::Invalid("Checksum is not correct");
  return Status::OK();
}

uint16_t FRAM::calculate_checksum(const uint8_t *data_src, size_t length) {
  uint16_t crc = 0xFFFF;
  for(size_t i = 0; i < length; i++) {
    crc ^= (uint16_t)data_src[i] << 8;
    for(int i = 0; i < 8; ++i) {
      if(crc & 0x8000)
        crc = (crc << 1) ^ 0x1021;
      else
        crc <<= 1;
    }
  }
  return crc;
}

Status FRAM::encrypt_data(uint8_t *data_src, size_t length, std::string key) {
  if(length == 0)
    return Status::CapacityError("Size of the data is 0");
  if(key == FRAM::base_encryption_key)
    return Status::OK();
  uint8_t shaout[algorithm::SHA256::SHA256_OUTPUT_SIZE];
  algorithm::SHA256::get_instance().sha256((uint8_t *)(key.c_str()), key.size(), shaout);
  for(size_t i = 0; i < length; i++)
    data_src[i] = data_src[i] ^ shaout[i % algorithm::SHA256::SHA256_OUTPUT_SIZE];
  return Status::OK();
}

Status FRAM::decrypt_data(uint8_t *data_src, size_t length, std::string key) {
  return encrypt_data(data_src, length, key);
}

void FRAM::set_encryption_key(const std::string &key) {
  encryption_key = key;
}

const std::string &FRAM::get_encryption_key() {
  return encryption_key;
}
