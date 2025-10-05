#include "device.hpp"
#include "sha256.hpp"
#include "status.hpp"
#include <Timing.hpp>
#include <cstdint>
#include <cstring>
#include <etl/platform.h>
#include <etl/vector.h>
#include <stmepic.hpp>
#include <string>
#include <vector>
#include <span>

#ifndef FRAMMENAGER_HPP
#define FRAMMENAGER_HPP

// #define FRAM_BASE_NAME_LEN 15
// #define FRAM_BASE_NAME "stmepic_fram"

/**
 * @file memory_fram.hpp
 * @brief Base interface class for reading and writing data to the FRAM devices.
 */

/**
 * @defgroup Memory
 * @brief Memory module for reading and writing data to the data storage devices.
 * Storage devices supported for now is FRAM
 * @{
 */


namespace stmepic::memory {
/**
 *
 *  FRAM DATA STRUCTURE
 *  the the first bite of the begining byte is MSB for all fields.
 *  \verbatim
 *  | Byte Offset | Field Name       | Size (bytes) | Description                        |
 *  |-------------|------------------|--------------|------------------------------------|
 *  | 0           | Magic Number 1   | 1            | A unique identifier of all data    |
 *  | 1           | Checksum         | 2            | A checksum for data integrity      |
 *  | 3           | Encryption Res   | 4            | A data used for encryption algo    |
 *  | 7           | Data Size        | 2            | The size of the data               |
 *  | 9           | Magic Number 2   | 1            | A unique identifier for the data   |
 *  | 10          | Data             | N            | The actual data                    |
 *  |-------------|------------------|--------------|------------------------------------|
 *  |             | Total            | 10+N         | Total size of the data structure   |
 *  \endverbatim
 *
 *  // the actual size of the key used for encryption is 64 bits
 *  // however the key used by the user is 32 bits
 **/

/// @brief The fram module to save data to the fram devices
class FRAM : public DeviceBase {

public:
  virtual ~FRAM() = default;

  /// @brief Write data to the FRAM device, with encoding crc check and encryption if enabled
  /// @param address the address where the data will be written
  /// @param data the data that will be written
  /// @return the status of the write operation
  Status write(uint32_t address, uint8_t *data, size_t length);

  /// @brief Read data from the FRAM device, with decoding crc check and decryption if enabled
  /// @param address the address where the data will be read
  /// @return the data that was read or error if the data was not read
  Result<std::pair<std::shared_ptr<uint8_t[]>, size_t>> read(uint32_t address);

  /// @brief writes the raw data to the FRAM device without any encoding
  /// @param address the address where the data will be written
  /// @param data the data that will be written
  /// @param length the length of the data that will be written
  /// @return the status of the write operation
  virtual Status read_raw(uint32_t address, uint8_t *data, size_t length) = 0;

  /// @brief reads the raw data from the FRAM device without any decoding
  /// @param address the address where the data will be read
  /// @param data the data that will be read
  /// @param length the length of the data that will be read
  /// @return the status of the read operation
  virtual Status write_raw(uint32_t address, uint8_t *data, size_t length) = 0;


  /// @brief Read a struct from the FRAM
  /// the sturcture should't have pointers or any other dynamic data
  /// if the struct has pointers or dynamic data it will cause a memory leak if read
  /// @param address the address where the struct will be read from
  /// @return the struct that was read or error if the struct was not read
  template <typename T> Result<T> readStruct(uint32_t address) {
    STMEPIC_ASSING_OR_RETURN(decoded_data, read(address));
    if(decoded_data.second != sizeof(T))
      return Status::CapacityError("Data size is not the same as the struct size");
    return Result<T>::OK(reinterpret_cast<T>(decoded_data.first));
  }

  /// @brief Write a struct to the FRAM
  /// the structure should't have pointers or any other dynamicly allocated data
  /// if the struct has pointers or dynamic data it will cause a memory leak if read
  /// @param address the address where the struct will be written
  /// @param data the struct that will be written
  template <typename T> Status writeStruct(uint32_t address, T &data) {
    return write(address, static_cast<uint8_t *>(&data), sizeof(T));
  }

  /// @brief Write a vector of structs to the FRAM
  /// the structure should't have pointers or any other dynamic data
  /// the vector can be any size
  /// @param address the address where the vector will be written
  /// @param data the vector that will be written
  /// @return the status of the write operation
  template <typename T> Status writeVector(uint32_t address, const std::vector<T> &data) {
    std::vector<uint8_t> data_vec;
    uint32_t vector_size = (uint32_t)data.size();
    STMEPIC_RETURN_ON_ERROR(writeStruct(address, vector_size));
    address += sizeof(uint32_t) + data_frame_size;
    for(auto &d : data) {
      STMEPIC_RETURN_ON_ERROR(writeStruct(address, d));
      address += sizeof(T) + data_frame_size;
    }
    return Status::OK();
  }

  /// @brief Read a vector of structs from the FRAM
  /// the structure should't have pointers or any other dynamic data
  /// the vector can be any size and the size of the vector will be read from the FRAM
  /// @param address the address where the vector will be read
  /// @return the vector that was read or error if the vector was not read
  template <typename T> Result<std::vector<T>> readVector(uint32_t address) {
    STMEPIC_ASSING_OR_RETURN(size, readStruct<uint32_t>(address));
    std::vector<T> data;
    data.resize(size);
    address += sizeof(uint32_t) + data_frame_size;
    for(size_t i = 0; i < size; i++) {
      STMEPIC_ASSING_OR_RETURN(str, readStruct<T>(address));
      data[i] = str;
      address += sizeof(T) + data_frame_size;
    }
    return Result<std::vector<T>>::OK(data);
  }


  /// @brief Set the encryption key
  /// @param key the key that will be used for encrypting data
  void set_encryption_key(const std::string &key);

  /// @brief Get the encryption key
  /// @return the key that is used for encryption
  const std::string &get_encryption_key();

protected:
  FRAM() : encryption_key(base_encryption_key){};


  /// @brief the base encryption key if used no encryption will be used
  static constexpr const std::string base_encryption_key = "stmepic";
  // static const uint32_t base_encryption_key = 0xFFFFFFFF;
  /// @brief size of the base frame structure
  static constexpr const uint16_t data_frame_size   = 10;
  static constexpr const uint16_t frame_offset_size = 7;
  /// @brief the magic numbers that are used to identify the data structure
  static constexpr const uint8_t magic_number_1 = 0x96;
  /// @brief the magic numbers that are used to identify the data structure
  static constexpr const uint8_t magic_number_2 = 0x69;


private:
  /// @brief encodes the data to be written to the FRAM device
  /// @param data the data that will be encoded
  /// @param length the length of the data to be encoded
  /// @param dest_data the destination where the encoded data will be stored the size have to be size(data) + frame_size
  /// @return the encoded data with additional parameters from the FRAM data structure
  Status encode_data(const uint8_t *data_src, size_t length, uint8_t *data_dest);

  /// @brief decodes the data that expects FRAM data structure
  /// @param data the data that will be decoded should be in FRAM data structure format
  /// @param length the length of the data to be decoded
  /// @param dest_data the destination where the decoded data will be stored the size have to be size(data) - frame_size
  /// @return the decoded data only the Data part of the FRAM data structure
  Status decode_data(uint8_t *frame_data_ptr, uint8_t *data_src, size_t length);

  uint16_t calculate_checksum(const uint8_t *data_src, size_t length);
  Status encrypt_data(uint8_t *data_src, size_t length, std::string key);
  Status decrypt_data(uint8_t *data_src, size_t length, std::string key);
  // uint8_t encryption_key[encryption_key_size];
  std::string encryption_key;
};


} // namespace stmepic::memory

#endif