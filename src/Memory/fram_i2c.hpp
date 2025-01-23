#pragma once

#include "memory_fram.hpp"
#include "stmepic.hpp"
#include "stmepic_status.hpp"
#include <cstdint>
#include <cstdlib>
#include <vector>


/**
 * @file fram_i2c.hpp
 * @brief Base interface class for reading and writing data to FRAM ICs sonnected using I2C.
 */

/**
 * @defgroup Memory
 * @{
 */

namespace stmepic::memory {

/// @brief The FRAM class for I2C FRAM ICs
class FramI2C : public FRAM {
public:
  /**
   * @brief Construct a new FramI2C object
   *
   * @param hi2c the I2C handle that will be used to communicate with the FRAM device
   * @param device_address the address of the FRAM device
   * @param begin_address the begining address from which the memory will be used
   * @param fram_size the size of the memory of the FRAM device to avoid out of bounds errors
   */
  FramI2C(I2C_HandleTypeDef &hi2c, uint8_t device_address, uint16_t begin_address, uint32_t fram_size);

  /// @brief Init the FRAM device
  Status init() override;

  /// @brief Write data to the FRAM device
  /// @param address the address where the data will be written
  /// @param data the data that will be written
  virtual Status write(uint32_t address, const std::vector<uint8_t> &data) override;

  /// @brief Read data from the FRAM device
  /// @param address the address where the data will be read
  /// @return the data that was read or error if the data was not read
  virtual Result<std::vector<uint8_t>> read(uint32_t address) override;


  Result<bool> device_is_connected() override final;
  bool device_ok() override final;
  Status device_get_status() override final;
  Status device_reset() override final;
  Status device_enable() override final;
  Status device_disable() override final;

protected:
  I2C_HandleTypeDef *hi2c;
  uint16_t begin_address;
  uint32_t fram_size;
  uint8_t device_address;
};

/**
 * @brief FramI2C class for the FM24CLxx series of FRAM ICs
 *
 */
class FramI2CFM24CLxx : public FramI2C {
public:
  FramI2CFM24CLxx(I2C_HandleTypeDef &hi2c, uint16_t begin_address, uint32_t fram_size);
  Status write(uint32_t address, const std::vector<uint8_t> &data) override;
  Result<std::vector<uint8_t>> read(uint32_t address) override;
};

} // namespace stmepic::memory