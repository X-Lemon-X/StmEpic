#include "stm32f4xx_hal.h"
#include <vector>
#include <string>

#ifndef FRAMMENAGER_HPP
#define FRAMMENAGER_HPP

#define FRAM_BASE_NAME_LEN 15

namespace FRAMM{

class FRAMfile{
  void *data;
  uint32_t data_size;
  char name[FRAM_BASE_NAME_LEN+1];
  uint32_t fram_address;
  uint32_t fram_size;
  public:
    FRAMfile();
    ~FRAMfile();
};

class FRAMMenager{
private:
  bool init_flag;
  I2C_HandleTypeDef *i2c;
  uint16_t address;
  uint32_t fram_size;
  std::vector<FRAMfile> files;

  int init_fram();
  int is_fram_ready();
  int clear_fram();

  int read_fram(uint32_t address, uint8_t *data, uint32_t size);
  int write_fram(uint32_t address, uint8_t *data, uint32_t size);

public:
  FRAMMenager();
  ~FRAMMenager();

  /// @brief Initialize the FRAM menager
  /// @param i2c a reference to the I2C_HandleTypeDef 
  /// @param address the address of the FRAM
  /// @param fram_size the size of the FRAM in bytes (8 bits)
  void init(I2C_HandleTypeDef &i2c, uint16_t address, uint32_t fram_size);

  std::vector<std::string> get_files();

  /// @brief write a file to the FRAM
  /// @param data a pointer to the data that will be written
  /// @param size the size of the data in bytes
  /// @param name the name of the file you wish to write it to
  /// @return a pointer to the data that was written or nullptr if the write failed
  void* write_file(void *data, uint32_t size, const char *name);

  /// @brief read a file from the FRAM
  /// @param name the name of the file you wish to read
  /// @return a pointer to the data that was read or nullptr if the fiele was not found
  void* read_file(const char *name);

};
  
} // namespace SRAMM

#endif