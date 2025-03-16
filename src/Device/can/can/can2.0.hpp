#pragma once

#include "stmepic.hpp"
#include "hardware.hpp"
#include "device.hpp"
#include <unordered_map>
#include "can.hpp"


/**
 * @file can2.0.hpp
 * @brief CAN2.0 interface wrapper class that alow to do handle the CAN interface with ease. By adding
 * callbacks for specific frame ids. with nice rx and tx tasks that handle the traffic in not blocking mode.
 */

namespace stmepic {

/**
 * @brief Class for controlling the CAN interface
 * automatically by allowing to add callbacks for specific frame ids.
 * as well as writing to interface from any task in a non blocking / thread safe fashion.
 */
class CAN : public CanBase {

public:
  ~CAN() override;


  /**
   * @brief Make new CAN interface, the interface is added to the list of all CAN interfaces and will be automatically handled with other CAN interfaces
   *
   * @param hcan the CAN handle that will be used to communicate with the CAN device
   * @param filter the filter that will be used to filter the CAN messages if
   * @param tx_led the TX led that will be used to indicate the TX activity
   * @param rx_led the RX led that will be used to indicate the RX activity
   * @return Result<std::shared_ptr<CAN>> will return AlreadyExists if the CAN interface was already initialized.
   */
  static Result<std::shared_ptr<CAN>>
  Make(CAN_HandleTypeDef &hcan, const CAN_FilterTypeDef &filter, GpioPin *tx_led = nullptr, GpioPin *rx_led = nullptr);

  /**
   * @brief Reset the CAN interface
   * @return Status
   */
  Status hardware_reset() override;

  /**
   * @brief Starts the CAN interface allong with tasks handling the TX and RX traffic
   * @return Status
   */
  Status hardware_start() override;

  /**
   * @brief Stops the CAN interface along with the tasks handling the TX and RX traffic
   * @return Status
   */
  Status hardware_stop() override;

  /**
   * @brief Write a CAN data frame to quue that will be then send to the CAN interface.
   * @param msg data frame that will be send
   * @return Status::OK if the data frame was added successfully
   */
  Status write(const CanDataFrame &msg) override;

  /**
   * @brief Add a callback function to the CAN interface for specific frame id
   * The callback is run in a RX task there fore it don't have to bo super fast but it shouldn't be too slow either.
   * @note The CAN have default callback that runs for all IDs that don't have a registered callback.
   * You CAN change the default callback by adding your custom callback with the frame_id = 0
   * @param frame_id the ID of the CAN data frame on which the callback will be called
   * @param callback the callback function that will be called when the frame_id is received
   * @param args the arguments that will be passed to the callback function
   * @return Status OK if the callback was added successfully
   */
  Status add_callback(uint32_t frame_id, internall::hardware_can_function_pointer callback, void *args = nullptr) override;

  /**
   * @brief Remove the callback function for the specific frame id
   *
   * @param frame_id the ID of the CAN data frame on which the callback will be removed
   * @return Status OK if the callback was removed successfully
   */
  Status remove_callback(uint32_t frame_id) override;

  /**
   * @brief Run this in  TX callbacks from the IT or DMA interrupt like HAL_CAN_TxMailboxXCompleteCallback
   * @param hi2c the CAN handle that triggered the interrupt
   * @note This function runs over all I2C initialized interfaces
   */
  static void run_tx_callbacks_from_irq(CAN_HandleTypeDef *hcan);

  /**
   * @brief Run the RX callbacks from the IT or DMA interrupt like HAL_CAN_RxFifoXMsgPendingCallback
   * @param hi2c the I2C handle that triggered the interrupt
   * @note This function runs over all I2C initialized interfaces
   */
  static void run_rx_callbacks_from_irq(CAN_HandleTypeDef *hcan);

private:
  CAN(CAN_HandleTypeDef &hcan, const CAN_FilterTypeDef &filter, GpioPin *tx_led, GpioPin *rx_led);
  CAN(const CAN &)            = delete;
  CAN &operator=(const CAN &) = delete;

  bool is_initiated;
  CAN_HandleTypeDef *_hcan;
  uint32_t last_tx_mailbox;
  uint32_t can_fifo;
  CAN_FilterTypeDef filter;
  GpioPin *_gpio_tx_led;
  GpioPin *_gpio_rx_led;
  TaskHandle_t task_handle_tx;
  TaskHandle_t task_handle_rx;
  QueueHandle_t tx_queue_handle;
  QueueHandle_t rx_queue_handle;
  std::unordered_map<uint32_t, internall::CanCallbackTask> callbacks;
  internall::CanCallbackTask default_callback_task_data;
  static std::vector<std::shared_ptr<CAN>> can_instances;
  static const uint32_t CAN_QUEUE_SIZE = 64;

  /// @brief Task for handling the TX traffic for specific CAN interface
  void tx_callback(CAN_HandleTypeDef *hcan);

  /// @brief Task for handling the RX traffic for specific CAN interface
  void rx_callback(CAN_HandleTypeDef *hcan);

  /// @brief task that handles the TX traffic
  static void task_tx(void *arg);

  /// @brief task that handles the RX traffic
  static void task_rx(void *arg);

  /**
   * @brief Default callback function for the CAN interface. Doesn't do anything
   * @param frame the data frame that was received
   * @param args the arguments that will be passed to the callback function
   */
  static void default_callback_function(CanBase &can, CanDataFrame &frame, void *args);
};


} // namespace stmepic