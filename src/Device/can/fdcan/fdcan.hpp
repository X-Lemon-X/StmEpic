#pragma once

#include "stmepic.hpp"
#include "hardware.hpp"
#include "device.hpp"
#include <unordered_map>
#include "can.hpp"


/**
 * @file fdcan.hpp
 * @brief FDCAN interface wrapper class that allow to do handle the FDCAN interface with ease. By adding
 * callbacks for specific frame ids. with nice rx and tx tasks that handle the traffic in not blocking mode.
 */

namespace stmepic {

/**
 * @brief Class for controlling the FDCAN interface
 * automatically by allowing to add callbacks for specific frame ids.
 * as well as writing to interface from any task in a non blocking / thread safe fashion.
 */
class FDCAN : public HardwareInterface {

public:
  ~FDCAN() override;


  /**
   * @brief Make new FDCAN interface, the interface is added to the list of all FDCAN interfaces and will be automatically handled with other FDCAN interfaces
   *
   * @param hcan the FDCAN handle that will be used to communicate with the FDCAN device
   * @param filter the filter that will be used to filter the FDCAN messages if
   * @param tx_led the TX led that will be used to indicate the TX activity
   * @param rx_led the RX led that will be used to indicate the RX activity
   * @return Result<std::shared_ptr<FDCAN>> will return AlreadyExists if the FDCAN interface was already initialized.
   */
  static Result<std::shared_ptr<FDCAN>>
  Make(FDCAN_HandleTypeDef &hcan, const FDCAN_FilterTypeDef &filter, GpioPin *tx_led = nullptr, GpioPin *rx_led = nullptr);

  /**
   * @brief Reset the FDCAN interface
   * @return Status
   */
  Status hardware_reset() override;

  /**
   * @brief Starts the FDCAN interface allong with tasks handling the TX and RX traffic
   * @return Status
   */
  Status hardware_start() override;

  /**
   * @brief Stops the FDCAN interface along with the tasks handling the TX and RX traffic
   * @return Status
   */
  Status hardware_stop() override;

  /**
   * @brief Write a FDCAN data frame to quue that will be then send to the FDCAN interface.
   * @param msg data frame that will be send
   * @return Status::OK if the data frame was added successfully
   */
  Status write(const CanDataFrame &msg);

  /**
   * @brief Add a callback function to the FDCAN interface for specific frame id
   * The callback is run in a RX task there fore it don't have to bo super fast but it shouldn't be too slow either.
   * @note The FDCAN have default callback that runs for all IDs that don't have a registered callback.
   * You FDCAN change the default callback by adding your custom callback with the frame_id = 0
   * @param frame_id the ID of the FDCAN data frame on which the callback will be called
   * @param callback the callback function that will be called when the frame_id is received
   * @param args the arguments that will be passed to the callback function
   * @return Status OK if the callback was added successfully
   */
  Status add_callback(uint32_t frame_id, internall::hardware_can_function_pointer callback, void *args = nullptr);

  /**
   * @brief Remove the callback function for the specific frame id
   *
   * @param frame_id the ID of the FDCAN data frame on which the callback will be removed
   * @return Status OK if the callback was removed successfully
   */
  Status remove_callback(uint32_t frame_id);

  /**
   * @brief Run this in  TX callbacks from the IT or DMA interrupt like HAL_CAN_TxMailboxXCompleteCallback
   * @param hi2c the FDCAN handle that triggered the interrupt
   * @note This function runs over all I2C initialized interfaces
   */
  static void run_tx_callbacks_from_irq(FDCAN_HandleTypeDef *hcan);

  /**
   * @brief Run the RX callbacks from the IT or DMA interrupt like HAL_CAN_RxFifoXMsgPendingCallback
   * @param hi2c the I2C handle that triggered the interrupt
   * @note This function runs over all I2C initialized interfaces
   */
  static void run_rx_callbacks_from_irq(FDCAN_HandleTypeDef *hcan);

private:
  FDCAN(FDCAN_HandleTypeDef &hcan, const FDCAN_FilterTypeDef &filter, GpioPin *tx_led, GpioPin *rx_led);
  FDCAN(const FDCAN &)            = delete;
  FDCAN &operator=(const FDCAN &) = delete;

  bool is_initiated;
  FDCAN_HandleTypeDef *_hcan;
  uint32_t last_tx_mailbox;
  uint32_t can_fifo;
  FDCAN_FilterTypeDef filter;
  GpioPin *_gpio_tx_led;
  GpioPin *_gpio_rx_led;
  TaskHandle_t task_handle_tx;
  TaskHandle_t task_handle_rx;
  QueueHandle_t tx_queue_handle;
  QueueHandle_t rx_queue_handle;
  std::unordered_map<uint32_t, internall::CanCallbackTask> callbacks;
  internall::CanCallbackTask default_callback_task_data;
  static std::vector<std::shared_ptr<FDCAN>> can_instances;
  static const uint32_t CAN_QUEUE_SIZE = 64;

  /// @brief Task for handling the TX traffic for specific FDCAN interface
  void tx_callback(FDCAN_HandleTypeDef *hcan);

  /// @brief Task for handling the RX traffic for specific FDCAN interface
  void rx_callback(FDCAN_HandleTypeDef *hcan);

  /// @brief task that handles the TX traffic
  static void task_tx(void *arg);

  /// @brief task that handles the RX traffic
  static void task_rx(void *arg);

  /**
   * @brief Default callback function for the FDCAN interface. Doesn't do anything
   * @param frame the data frame that was received
   * @param args the arguments that will be passed to the callback function
   */
  static void default_callback_function(CanBase &can, CanDataFrame &frame, void *args);
};


} // namespace stmepic