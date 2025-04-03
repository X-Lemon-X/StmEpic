#pragma once

#include "stmepic.hpp"
#include "hardware.hpp"

namespace stmepic {

/**
 * @brief Class for controlling the UART interface using  DMA, IT or blocking mode
 * The best mode is DMA and IT since they allows other other tasks to run while the UART is reading or
 * writing all function to read and write data are blocking with timeout
 * .
 */
class UART : public HardwareInterface {
public:
  ~UART() override;

  /**
   * @brief Make new UART interface, the interface is added to the list of all UART interfaces and will be automatically handled
   *
   * @param huart the UART handle that will be used to communicate with the UART device
   * @param type the type of the UART interface mode, DMA, IT or BLOCKING
   * @return Result<std::shared_ptr<UART>> will return AlreadyExists if the interface was already initialized.
   */
  static Result<std::shared_ptr<UART>> Make(UART_HandleTypeDef &huart, const HardwareType type);

  /**
   * @brief Reset the UART interface
   * This will reset also UART lines by pulling them low and high to reset the UART devices
   * @return Status
   */
  Status hardware_reset() override;

  /**
   * @brief Start the UART interface init all required settings for the UART interface
   * Like setings for DMA, IT or blocking mode
   * @return Status
   */
  Status hardware_start() override;

  /**
   * @brief Stop the UART interface
   * @return Status
   */
  Status hardware_stop() override;


  /**
   * @brief Read data from the UART device in blocking mode with other tasks beeing able to freely run in the
   * meantime returns the data read from the device after the read is done
   *
   * @param data the data that will be read from the UART device
   * @param size the size of the data that will be read
   * @param timeout_ms the timeout for the read operation works in all modes.
   * Note its beter to use higher timeout them small one otherwise weird things might happen.
   * @return Result<uint8_t *>
   */
  Status read(uint8_t *data, uint16_t size, uint16_t timeout_ms = 300);

  /**
   * @brief Write data to the UART device in blocking mode with other tasks beeing able to freely run in the
   *
   * @param data the data that will be written to the UART device
   * @param size the size of the data that will be written
   * @param timeout_ms the timeout for the read operation works in all modes.
   * Note its beter to use higher timeout them small one otherwise weird things might happen.
   * @return Status
   */
  Status write(uint8_t *data, uint16_t size, uint16_t timeout_ms = 100);

  /**
   * @brief Run the TX callbacks from the IT or DMA interrupt
   * @param huart the UART handle that triggered the interrupt
   * @note This function runs over all UART initialized interfaces
   */
  static void run_tx_callbacks_from_isr(UART_HandleTypeDef *huart, bool half);

  /**
   * @brief Run the RX callbacks from the IT or DMA interrupt
   * @param huart the UART handle that triggered the interrupt
   * @note This function runs over all UART initialized interfaces
   */
  static void run_rx_callbacks_from_isr(UART_HandleTypeDef *huart, bool half);

private:
  UART(UART_HandleTypeDef &huart, const HardwareType type);

  const HardwareType _hardwType;
  SemaphoreHandle_t _mutex;
  UART_HandleTypeDef *_huart;
  bool dma_lock;

  /// @brief Task handle for the specific UART interface
  TaskHandle_t task_handle;
  /// @brief  List of all UART interfaces initialized
  static std::vector<std::shared_ptr<UART>> uart_instances;

  void tx_callback(UART_HandleTypeDef *huart, bool half);
  void rx_callback(UART_HandleTypeDef *huart, bool half);

  Status _read(uint8_t *data, uint16_t size, uint16_t timeout_ms = 100);
  Status _write(uint8_t *data, uint16_t size, uint16_t timeout_ms = 100);
};

} // namespace stmepic