#pragma once

#include "stmepic.hpp"
#include "hardware.hpp"
#include "multiplexer.hpp"
#include <optional>
#include <vector>

/**
 * @defgroup hardware Hardware
 * @{
 */

/**
 * @defgroup i2c_hardware_devices I2C
 * @brief I2C driver
 * @{
 */

namespace stmepic {


class I2cBase : public HardwareInterface {
public:
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
  virtual Status
  read(uint16_t address, uint16_t mem_address, uint8_t *data, uint16_t size, uint16_t mem_size = 1, uint16_t timeout_ms = 300) = 0;


  /**
   * @brief Write data to the I2C device in blocking mode with other tasks beeing able to freelu run in the
   *
   * @param address the address of the I2C device in 7 bir format it will be automatically by 1
   * @param mem_address the memory address to which the data will be written
   * @param mem_size the size of the memory address
   * @param data the data that will be written to the I2C device
   * @param size the size of the data that will be written
   * @param mem_size the size of the memory address
   * @param timeout_ms the timeout for the write operation
   * @return Status
   */
  virtual Status
  write(uint16_t address, uint16_t mem_address, uint8_t *data, uint16_t size, uint16_t mem_size = 1, uint16_t timeout_ms = 300) = 0;

  /**
   * @brief Check if the device is ready to communicate with specified address
   * @param address the address of the I2C device
   * @param trials the number of trials to check if the device is ready
   * @param timeout the timeout for each trial
   * @return Status
   */
  virtual Status is_device_ready(uint16_t address, uint32_t trials, uint32_t timeout) = 0;

  /**
   * @brief Scan for all I2C devices on the bus
   * @return  the addresses of the devices found on the bus
   */
  [[nodiscard]] virtual Result<std::vector<uint16_t>> scan_for_devices() = 0;
};


/**
 * @brief Class for controlling the I2C interface with automatic DMA, IT or blocking mode
 * The best mode is DMA and ISR since they allows other other tasks to run while the I2C is reading or writing
 * all function to read and write data are blocking.
 */
class I2C : public I2cBase {
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
  virtual Status
  read(uint16_t address, uint16_t mem_address, uint8_t *data, uint16_t size, uint16_t mem_size = 1, uint16_t timeout_ms = 300) override;


  /**
   * @brief Write data to the I2C device in blocking mode with other tasks beeing able to freelu run in the
   *
   * @param address the address of the I2C device in 7 bir format it will be automatically by 1
   * @param mem_address the memory address to which the data will be written
   * @param mem_size the size of the memory address
   * @param data the data that will be written to the I2C device
   * @param size the size of the data that will be written
   * @param mem_size the size of the memory address
   * @param timeout_ms the timeout for the write operation
   * @return Status
   */
  virtual Status
  write(uint16_t address, uint16_t mem_address, uint8_t *data, uint16_t size, uint16_t mem_size = 1, uint16_t timeout_ms = 300) override;

  /**
   * @brief Check if the device is ready to communicate with specified address
   * @param address the address of the I2C device
   * @param trials the number of trials to check if the device is ready
   * @param timeout the timeout for each trial
   * @return Status
   */
  virtual Status is_device_ready(uint16_t address, uint32_t trials, uint32_t timeout) override;

  /**
   * @brief Scan for all I2C devices on the bus
   * @return  the addresses of the devices found on the bus
   */
  [[nodiscard]] virtual Result<std::vector<uint16_t>> scan_for_devices() override;

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

class I2cMultiplexerChannel;

class I2cMultiplexerChannel : public I2cBase {
public:
  I2cMultiplexerChannel(std::shared_ptr<I2C> i2c, uint8_t channel, MultiplexerBase &multiplexer)
  : _i2c(i2c), channel(channel), _multiplexer(multiplexer){};
  Status hardware_start() override;
  Status hardware_stop() override;
  Status hardware_reset() override;
  Status read(uint16_t address, uint16_t mem_address, uint8_t *data, uint16_t size, uint16_t mem_size = 1, uint16_t timeout_ms = 300) override;
  Status write(uint16_t address, uint16_t mem_address, uint8_t *data, uint16_t size, uint16_t mem_size = 1, uint16_t timeout_ms = 300) override;
  Status is_device_ready(uint16_t address, uint32_t trials, uint32_t timeout) override;
  Result<std::vector<uint16_t>> scan_for_devices() override;

private:
  uint8_t channel;
  std::shared_ptr<I2C> _i2c;
  MultiplexerBase &_multiplexer;
};

/// @brief Class for using an I2C with a multiplexer with selectable address pins
/// with auto handling of the channel switching depending on the requested channel by the driver using the I2C interface.
class I2cMultiplexerGpioID : public MultiplexerBase {
public:
  /// @brief Make new I2C multiplexer interface with selectable address pins
  ///
  /// @param i2c the I2C interface that will be used to communicate with the multiplexer
  /// @param channels the number of channels of the multiplexer
  /// @param address_pin_1 the first address pin of the multiplexer address pin 1 is mandatory.
  /// @param address_pin_2 the second address pin of the multiplexer
  /// @param address_pin_3 the third address pin of the multiplexer
  /// @param address_pin_4 the fourth address pin of the multiplexer
  /// @param pin_reset optional reset pin for the multiplexer
  Result<std::shared_ptr<I2cMultiplexerGpioID>> Make(std::shared_ptr<I2C> i2c,
                                                     uint8_t channels,
                                                     GpioPin address_pin_1,
                                                     std::optional<GpioPin> address_pin_2 = std::nullopt,
                                                     std::optional<GpioPin> address_pin_3 = std::nullopt,
                                                     std::optional<GpioPin> address_pin_4 = std::nullopt,
                                                     uint8_t switch_delay_us              = 1);

  /// @brief Get the I2C interface for the specific channel,
  /// Which should be passed to the device driver which is connected to the multiplexer on this specific channel.
  /// @param channel the channel number to which the device is connected
  /// @return the I2C interface for the specific channel this should be passed to the device driver which is connected to the multiplexer on the specific channel.
  Result<std::shared_ptr<I2cBase>> get_i2c_interface_for_channel(uint8_t channel);

private:
  I2cMultiplexerGpioID(std::shared_ptr<I2C> i2c,
                       GpioPin address_pin_1,
                       std::optional<GpioPin> address_pin_2 = std::nullopt,
                       std::optional<GpioPin> address_pin_3 = std::nullopt,
                       std::optional<GpioPin> address_pin_4 = std::nullopt,
                       uint8_t switch_delay_us              = 10);


  virtual Status select_channel(uint8_t channel) override;
  virtual uint8_t get_selected_channel() const override;
  virtual uint8_t get_total_channels() const override;

  std::shared_ptr<I2C> _i2c;
  std::vector<std::shared_ptr<I2cBase>> _i2c_channels;
  uint8_t _channels;
  uint8_t _selected_channel;
  GpioPin _address_pin_1;
  std::optional<GpioPin> _address_pin_2;
  std::optional<GpioPin> _address_pin_3;
  std::optional<GpioPin> _address_pin_4;
  uint8_t _switch_delay_us;
};

} // namespace stmepic