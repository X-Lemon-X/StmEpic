#pragma once

#include "stmepic.hpp"
#include "hardware.hpp"

namespace stmepic {

/**
 * @brief Class for controlling the I2C interface with automatic DMA, IT or blocking mode
 * The best mode is DMA and ISR since they allows other other tasks to run while the I2C is reading or writing
 * all function to read and write data are blocking.
 */
class I2C : public HardwareInterface {
public:
  ~I2C() override;

  /**
   * @brief Make new I2C interface, the interface is added to the list of all I2C interfaces and will be automatically handled
   *
   * @param hi2c the I2C handle that will be used to communicate with the I2C device
   * @param sda the SDA pin of the I2C interface
   * @param scl the SCL pin of the I2C interface
   * @param type the type of the I2C interface mode, DMA, ISR or BLOCKING
   * @return Result<std::shared_ptr<I2C>> will return AlreadyExists if the I2C interface was already initialized.
   */
  static Result<std::shared_ptr<I2C>> Make(I2C_HandleTypeDef &hi2c, GpioPin &sda, GpioPin &scl, const HardwareType type);

  /**
   * @brief Reset the I2C interface
   * This will reset also I2C lines by pulling them low and high to reset the I2C devices
   * @return Status
   */
  Status hardware_reset() override;

  /**
   * @brief Start the I2C interface init all required settings for the I2C interface
   * Like setings for DMA, ISR or blocking mode
   * @return Status
   */
  Status hardware_start() override;

  /**
   * @brief Stop the I2C interface
   * @return Status
   */
  Status hardware_stop() override;


  /**
   * @brief Read data from the I2C device from memory in blocking mode with other tasks beeing able to freely
   * run in the meantime returns the data read from the device after the read is done
   *
   * @param address the address of the I2C device
   * @param mem_address the memory address from which the data will be read
   * @param data the data that will be read from the I2C device
   * @param size the size of the data that will be read
   * @return Result<uint8_t *>
   */
  Status read(uint16_t address, uint16_t mem_address, uint8_t *data, uint16_t size, uint16_t mem_size = 1, uint16_t timeout_ms = 300);


  /**
   * @brief Write data to the I2C device in blocking mode with other tasks beeing able to freelu run in the
   *
   * @param address the address of the I2C device in 7 bir format it will be automatically by 1
   * @param mem_address the memory address to which the data will be written
   * @param mem_size the size of the memory address
   * @param data the data that will be written to the I2C device
   * @param size the size of the data that will be written
   * @return Status
   */
  Status write(uint16_t address, uint16_t mem_address, uint8_t *data, uint16_t size, uint16_t mem_size = 1, uint16_t timeout_ms = 300);

  /**
   * @brief Check if the device is ready to communicate with specified address
   * @param address the address of the I2C device
   * @param trials the number of trials to check if the device is ready
   * @param timeout the timeout for each trial
   * @return Status
   */
  Status is_device_ready(uint16_t address, uint32_t trials, uint32_t timeout);

  /**
   * @brief Scan for all I2C devices on the bus
   * @return  the addresses of the devices found on the bus
   */
  [[nodiscard]] Result<std::vector<uint8_t>> scan_for_devices();

  /**
   * @brief Run the TX callbacks from the ISR or DMA interrupt
   * @param hi2c the I2C handle that triggered the interrupt
   * @note This function runs over all I2C initialized interfaces
   */
  static void run_tx_callbacks_from_isr(I2C_HandleTypeDef *hi2c);

  /**
   * @brief Run the RX callbacks from the ISR or DMA interrupt
   * @param hi2c the I2C handle that triggered the interrupt
   * @note This function runs over all I2C initialized interfaces
   */
  static void run_rx_callbacks_from_isr(I2C_HandleTypeDef *hi2c);

private:
  I2C(I2C_HandleTypeDef &hi2c, GpioPin &sda, GpioPin &scl, const HardwareType type);

  const HardwareType _hardwType;
  GpioPin &_gpio_sda;
  GpioPin &_gpio_scl;
  SemaphoreHandle_t _mutex;
  I2C_HandleTypeDef *_hi2c;
  bool dma_lock;
  bool i2c_initialized;

  /// @brief Task handle for the specific I2C interface
  TaskHandle_t task_handle;
  /// @brief  List of all I2C interfaces initialized
  static std::vector<std::shared_ptr<I2C>> i2c_instances;

  void tx_callback(I2C_HandleTypeDef *hi2c);
  void rx_callback(I2C_HandleTypeDef *hi2c);

  Status _read(uint16_t address, uint16_t mem_address, uint8_t *data, uint16_t size, uint16_t mem_size, uint16_t timeout_ms = 300);
  Status _write(uint16_t address, uint16_t mem_address, uint8_t *data, uint16_t size, uint16_t mem_size, uint16_t timeout_ms = 300);
};

} // namespace stmepic