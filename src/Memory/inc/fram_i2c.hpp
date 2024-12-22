#pragma once

#include "stmepic.hpp"
#include "stmepic_status.hpp"
#include "memory_fram.hpp"
#include <cstdint>
#include <cstdlib>
#include <vector>

namespace stmepic::memory{


class FramI2C: public FRAM {
  public:
  FramI2C(I2C_HandleTypeDef &hi2c,uint8_t device_address, uint16_t begin_address, uint32_t fram_size);
  Status init() override;
  virtual Status write(uint32_t address, const std::vector<uint8_t> &data) override;
  virtual Result<std::vector<uint8_t>> read(uint32_t address) override;
  
  Result<bool> device_is_connected() override final;
  bool device_ok() override final;
  Result<DeviceStatus> device_get_status() override final;
  Status device_reset() override final;
  Status device_start() override final;
  Status device_stop() override final;

  protected:
  I2C_HandleTypeDef *hi2c;
  uint16_t begin_address;
  uint32_t fram_size;
  uint8_t device_address;

};

class FramI2CFM24CLxx: public FramI2C {
  public:
  FramI2CFM24CLxx(I2C_HandleTypeDef &hi2c, uint16_t begin_address, uint32_t fram_size);
  Status write(uint32_t address, const std::vector<uint8_t> &data) override;
  Result<std::vector<uint8_t>> read(uint32_t address) override;
};

} // namespace SRAMM