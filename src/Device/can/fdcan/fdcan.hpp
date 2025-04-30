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

enum class FDCAN_FIFO {
  FDCAN_FIFO0,
  FDCAN_FIFO1,
};

/**
 * @brief FDCAN filter configuration
 * @note The filter is used to filter the FDCAN messages
 * @param filters the filters that will be used to filter the FDCAN messages the vector size shoule not exceed
 * the constraint of the FDCAN number of maximum filters.
 * @param fifo_number the FIFO number that will be used to receive the FDCAN messages
 * @param globalFilter_NonMatchingStd Defines how received messages with 11-bit IDs that do not match any
 * element of the filter list are treated. This parameter can be a value of FDCAN_Non_Matching_Frames.
 * @param globalFilter_NonMatchingExt Defines how received messages with 29-bit IDs that do not match any
 * element of the filter list are treated. This parameter can be a value of FDCAN_Non_Matching_Frames.
 * @param globalFilter_RejectRemoteStd Filter or reject all the remote 11-bit IDs frames. This parameter can
 * be a value of FDCAN_Reject_Remote_Frames.
 * @param globalFilter_RejectRemoteExt Filter or reject all the remote 29-bit IDs frames. This parameter can
 * be a value of FDCAN_Reject_Remote_Frames.
 */
struct FDcanFilterConfig {
  std::vector<FDCAN_FilterTypeDef> filters;
  FDCAN_FIFO fifo_number;
  uint32_t globalFilter_NonMatchingStd;
  uint32_t globalFilter_NonMatchingExt;
  uint32_t globalFilter_RejectRemoteStd;
  uint32_t globalFilter_RejectRemoteExt;
};


/**
 * @brief Class for controlling the FDCAN interface
 * automatically by allowing to add callbacks for specific frame ids.
 * as well as writing to interface from any task in a non blocking / thread safe fashion.
 *
 * @note The FDCAN interface does not support the BUFFER mode only FIFO mode.
 */
class FDCAN : public CanBase {

public:
  ~FDCAN() override;


  /**
   * @brief Make new FDCAN interface, the interface is added to the list of all FDCAN interfaces and will be automatically handled with other FDCAN interfaces
   *
   * @param hcan the FDCAN handle that will be used to communicate with the FDCAN device
   * @param filter the filter that will be used to filter the FDCAN messages
   * @param tx_led the TX led that will be used to indicate the TX activity
   * @param rx_led the RX led that will be used to indicate the RX activity
   * @return Result<std::shared_ptr<FDCAN>> will return AlreadyExists if the FDCAN interface was already initialized.
   */
  static Result<std::shared_ptr<FDCAN>>
  Make(FDCAN_HandleTypeDef &hcan, const FDcanFilterConfig &filter, GpioPin *tx_led = nullptr, GpioPin *rx_led = nullptr);

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
  static void run_tx_callbacks_from_irq(FDCAN_HandleTypeDef *hcan, uint32_t RxFifo0ITs);

  /**
   * @brief Run the RX callbacks from the IT or DMA interrupt like HAL_CAN_RxFifoXMsgPendingCallback
   * @param hi2c the I2C handle that triggered the interrupt
   * @note This function runs over all I2C initialized interfaces
   */
  static void run_rx_callbacks_from_irq(FDCAN_HandleTypeDef *hcan, uint32_t BufferIndexes);

private:
  FDCAN(FDCAN_HandleTypeDef &hcan, const FDcanFilterConfig &filter, GpioPin *tx_led, GpioPin *rx_led);
  FDCAN(const FDCAN &)            = delete;
  FDCAN &operator=(const FDCAN &) = delete;

  bool is_initiated;
  FDCAN_HandleTypeDef *_hcan;
  uint32_t last_tx_mailbox;
  FDCAN_FIFO fifo;
  uint32_t can_fifo;
  FDcanFilterConfig filter;
  GpioPin *_gpio_tx_led;
  GpioPin *_gpio_rx_led;
  TaskHandle_t task_handle_tx;
  TaskHandle_t task_handle_rx;
  QueueHandle_t tx_queue_handle;
  QueueHandle_t rx_queue_handle;
  bool fdcan_in_fd_mode;
  bool fdcan_in_bitrate_switching_mode;

  std::unordered_map<uint32_t, internall::CanCallbackTask> callbacks;
  internall::CanCallbackTask default_callback_task_data;
  static std::vector<std::shared_ptr<FDCAN>> can_instances;
  static const uint32_t CAN_QUEUE_SIZE = 64;

  /// @brief Task for handling the TX traffic for specific FDCAN interface
  void tx_callback(FDCAN_HandleTypeDef *hcan, uint32_t BufferIndexes);

  /// @brief Task for handling the RX traffic for specific FDCAN interface
  void rx_callback(FDCAN_HandleTypeDef *hcan, uint32_t RxFifo0ITs);

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