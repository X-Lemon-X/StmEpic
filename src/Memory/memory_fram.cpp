#include "memory_fram.hpp"
#include "device.hpp"
#include "sha256.hpp"
#include "stmepic.hpp"
#include "status.hpp"
#include <cstdint>
#include <cstdlib>

using namespace stmepic::memory;
using namespace stmepic;

// What is it hardcoding the encryption key?
const std::string FRAM::base_encryption_key = "stmepic";
const uint16_t FRAM::frame_size;
const uint8_t FRAM::magic_number_1;
const uint8_t FRAM::magic_number_2;

FRAM::FRAM() {
  encryption_key = base_encryption_key;
}

Result<std::vector<uint8_t>> FRAM::encode_data(const std::vector<uint8_t> &data) {
  if(data.size() == 0)
    return Status::CapacityError("Size of the data is 0");

  uint32_t encres = stmepic::Ticker::get_instance().get_micros();
  std::vector<uint8_t> encres_data;
  encres_data.resize(4);
  encres_data[0] = (encres >> 24) & 0xFF;
  encres_data[1] = (encres >> 16) & 0xFF;
  encres_data[2] = (encres >> 8) & 0xFF;
  encres_data[3] = encres & 0xFF;

  uint16_t checksum = calculate_checksum(data);
  std::vector<uint8_t> da;
  da.resize(data.size() + frame_size);
  da[0] = magic_number_1;
  da[1] = (checksum >> 8) & 0xFF;
  da[2] = checksum & 0xFF;
  da[9] = magic_number_2;

  // if encryption is disabled
  if(encryption_key == FRAM::base_encryption_key) {
    da[3] = 0;
    da[4] = 0;
    da[5] = 0;
    da[6] = 0;
    da[7] = (data.size() >> 8) & 0xFF;
    da[8] = data.size() & 0xFF;
    for(size_t i = 0; i < data.size(); i++)
      da[i + 10] = data[i];
  } else {
    STMEPIC_ASSING_OR_RETURN(encres_encrypted, encrypt_data(encres_data, encryption_key));
    std::string key = std::to_string(encres) + encryption_key;
    STMEPIC_ASSING_OR_RETURN(encrypted_data, encrypt_data(data, key));

    da[3] = encres_encrypted[0];
    da[4] = encres_encrypted[1];
    da[5] = encres_encrypted[2];
    da[6] = encres_encrypted[3];
    da[7] = (encrypted_data.size() >> 8) & 0xFF;
    da[8] = encrypted_data.size() & 0xFF;
    for(size_t i = 0; i < encrypted_data.size(); i++)
      da[i + 10] = encrypted_data[i];
  }
  return Result<std::vector<uint8_t>>::OK(da);
}

Result<std::vector<uint8_t>> FRAM::decode_data(const std::vector<uint8_t> &data) {
  if(data.size() == 0)
    return Status::CapacityError("Size of the data is 0");

  uint8_t mg1       = data[0];
  uint16_t checksum = (data[1] << 8) | data[2];
  // uint32_t encres = (data[3] << 24) | (data[4] << 16) | (data[5] << 8) | data[6];
  uint16_t size = (data[7] << 8) | data[8];
  uint8_t mg2   = data[9];
  if(mg1 != magic_number_1)
    return Status::Invalid("Magic number 1 is not correct");
  if(mg2 != magic_number_2)
    return Status::Invalid("Magic number 2 is not correct");
  if(size != data.size() - frame_size)
    return Status::Invalid("Size is not correct");

  // if encryption is disabled
  if(encryption_key == FRAM::base_encryption_key) {
    auto data_data = std::vector<uint8_t>(data.begin() + frame_size, data.end());
    if(calculate_checksum(data_data) != checksum)
      return Status::Invalid("Checksum is not correct");
    return Result<decltype(data_data)>::OK(data_data);

  } else {
    std::vector<uint8_t> encrypted_encres;
    encrypted_encres.resize(4);
    encrypted_encres[0] = data[3];
    encrypted_encres[1] = data[4];
    encrypted_encres[2] = data[5];
    encrypted_encres[3] = data[6];
    STMEPIC_ASSING_OR_RETURN(decrypted_encres, decrypt_data(encrypted_encres, encryption_key));
    uint32_t encres = (decrypted_encres[0] << 24) | (decrypted_encres[1] << 16) | (decrypted_encres[2] << 8) |
                      decrypted_encres[3];
    std::string key = std::to_string(encres) + encryption_key;

    auto data_data = std::vector<uint8_t>(data.begin() + frame_size, data.end());
    STMEPIC_ASSING_OR_RETURN(decrypted_data, decrypt_data(data_data, key));
    if(calculate_checksum(decrypted_data) != checksum)
      return Status::Invalid("Checksum is not correct");
    return Result<decltype(decrypted_data)>::OK(decrypted_data);
  }
}


uint16_t FRAM::calculate_checksum(const std::vector<uint8_t> &data) {
  uint16_t checksum = 0;
  for(auto &d : data)
    checksum += d;
  return checksum;
}

Result<std::vector<uint8_t>> FRAM::encrypt_data(const std::vector<uint8_t> &data, std::string key) {
  if(data.size() == 0)
    return Status::CapacityError("Size of the data is 0");
  if(key == FRAM::base_encryption_key)
    return Result<std::vector<uint8_t>>::OK(data);
  std::vector<uint8_t> encrypted_data;
  uint8_t shaout[algorithm::SHA256::SHA256_OUTPUT_SIZE];
  // uint32_t key_data[] = {key, encres};
  algorithm::SHA256::sha256((uint8_t *)(key.c_str()), key.size(), shaout);
  for(size_t i = 0; i < data.size(); i++)
    encrypted_data.push_back(data[i] ^ shaout[i % algorithm::SHA256::SHA256_OUTPUT_SIZE]);
  return Result<std::vector<uint8_t>>::OK(encrypted_data);
}

Result<std::vector<uint8_t>> FRAM::decrypt_data(const std::vector<uint8_t> &data, std::string key) {
  if(data.size() == 0)
    return Status::CapacityError("Size of the data is 0");
  if(key == FRAM::base_encryption_key)
    return Result<std::vector<uint8_t>>::OK(data);
  std::vector<uint8_t> decrypted_data;
  uint8_t shaout[algorithm::SHA256::SHA256_OUTPUT_SIZE];
  // uint32_t key_data[] = {key, encres};
  algorithm::SHA256::sha256((uint8_t *)(key.c_str()), key.size(), shaout);
  for(size_t i = 0; i < data.size(); i++)
    decrypted_data.push_back(data[i] ^ shaout[i % algorithm::SHA256::SHA256_OUTPUT_SIZE]);
  return Result<std::vector<uint8_t>>::OK(decrypted_data);
}

void FRAM::set_encryption_key(std::string key) {
  encryption_key = key;
}

std::string FRAM::get_encryption_key() {
  return encryption_key;
}
