#pragma once

#include "Timing.hpp"
#include "circular_buffor.hpp"
#include "gpio.hpp"
#include "stmepic.hpp"
#include "stmepic_status.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
// #include <unordered_map>
#include "etl/unordered_map.h"
#include <cstring>
#include <memory> // Add this line


#ifndef CAN_DATA_FRAME_MAX_SIZE
#define CAN_DATA_FRAME_MAX_SIZE 8
#endif

#ifndef CAN_QUEUE_SIZE
#define CAN_QUEUE_SIZE 128
#endif

#ifndef CAN_LED_BLINK_PERIOD_US
#define CAN_LED_BLINK_PERIOD_US 1000
#endif

#define CAN_CONTROL_CAN_CALLBACK_LIST_SIZE_DEFAULT 64


namespace stmepic {


/**
 * @brief CanMsg struct contains the data of the CAN frame
 *
 */
struct CanMsg {
  public:
  /// @brief frame_id of the message
  uint32_t frame_id;

  /// @brief remote request flag
  bool remote_request;

  /// @brief data of the message
  uint8_t data[CAN_DATA_FRAME_MAX_SIZE];

  /// @brief size of the data
  uint8_t data_size;
};

/**
 * @brief CanControl class for handling the CAN interface
 *
 * @tparam CAN_CALLBACK_LIST_SIZE size of the callback list
 *
 * This class provides functionalities for handling the CAN interface
 * with easy to implement callbacks for each frame id.
 */
template <uint32_t CAN_CALLBACK_LIST_SIZE = CAN_CONTROL_CAN_CALLBACK_LIST_SIZE_DEFAULT>
class CanControl {
  public:
  using function_pointer = void (*)(CanMsg&);

  static const uint32_t CAN_DEFAULT_FRAME_ID = 0x0;

  /// @brief  Construct a new Can Control object
  CanControl() {
    filter_mask    = 0;
    filter_base_id = 0;
  };

  /// @brief  init the CanControl object
  /// @param can_interface  CAN interface
  /// @param can_fifo  CAN fifo number
  /// @param ticker  main systeerror ticker object
  /// @param pin_tx_led  pin object for the TX led
  /// @param pin_rx_led  pin object for the RX ledCAN_QUEUE_SIZE
  void init(CAN_HandleTypeDef& _can_interface,
            uint32_t _can_fifo,
            gpio::GpioPin* _pin_tx_led,
            gpio::GpioPin* _pin_rx_led) {
    can_interface = &_can_interface;
    can_fifo      = _can_fifo;
    pin_rx_led    = _pin_rx_led;
    pin_tx_led    = _pin_tx_led;
    timing_led_rx = Timing::Make(CAN_LED_BLINK_PERIOD_US, false).valueOrDie();
    timing_led_tx = Timing::Make(CAN_LED_BLINK_PERIOD_US, false).valueOrDie();
    timing_led_rx->set_behaviour(CAN_LED_BLINK_PERIOD_US, false);
    timing_led_tx->set_behaviour(CAN_LED_BLINK_PERIOD_US, false);
    add_callback(CAN_DEFAULT_FRAME_ID, base_callback);
  };

  /// @brief  Add a callback function to the CAN interface
  /// @param frame_id  frame id of the message, set to 0 to receive all messages
  /// @param function_pointer  pointer to the function that will be called when the message with the frame_id is received
  auto add_callback(uint32_t frame_id, function_pointer function) {
    if(function == nullptr)
      return Status::Invalid("Function pointer is null");
    callback_map[frame_id] = function;
    return Status::OK();
  };

  /// @brief  Add a filter to the CAN interface. Why because i can't get the hardware filters to work properly!
  /// @param base_id  base id of the filter
  /// @param mask  mask of the filter
  void set_filter(uint32_t base_id, uint32_t mask) {
    filter_mask    = mask;
    filter_base_id = base_id;
  };

  /// @brief  Handle the RX interrupt
  void irq_handle_rx() {

    blink_rx_led();
    if(HAL_CAN_GetRxMessage(can_interface, can_fifo, &header, data) != HAL_OK)
      return;
    if(((header.StdId & filter_mask) != filter_base_id) && ((header.ExtId & filter_mask) != filter_base_id))
      return;
    CanMsg msg;
    if(header.IDE == CAN_ID_EXT)
      msg.frame_id = header.ExtId;
    else
      msg.frame_id = header.StdId;

    msg.remote_request = header.RTR == CAN_RTR_REMOTE;
    msg.data_size      = header.DLC;
    if(!msg.remote_request)
      memcpy(msg.data, data, msg.data_size);

    auto callback = callback_map.find(msg.frame_id);
    if(callback != callback_map.end())
      callback->second(msg);
    else
      callback_map[0](msg);
  };

  /// @brief  Handle the TX interrupt
  void irq_handle_tx() {
    __NOP();
  };

  /// @brief  This function should be called in the main loop of the uc program
  ///         It will handle the RX and TX incoming data, leds and other tasks that require updating
  void handle() {
    handle_leds();
    // handle_receive();
    handle_send();
  };

  Result<std::shared_ptr<Timing>> GetCanHandlerTask(std::string name, uint32_t period = 2000) {
    if(timing_can_task != nullptr)
      return Result<std::shared_ptr<Timing>>::OK(timing_can_task);

    timing_can_task = Timing::Make(period, true, handle());
    return Result<std::shared_ptr<Timing>>::OK(timing_can_task);
  }

  /// @brief  Adds a message to the queue, the messages will be send only when handle is called
  /// @param msg  CAN_MSG to be sent
  Status send_can_msg_to_queue(CanMsg& msg) {
    return tx_msg_buffor.push_back(msg);
  };

  /// @brief  Get the message from the RX buffer
  /// @param msg  pointer to the CAN_MSG object
  /// @return 0 if success
  // Result<CanMsg> get_can_msg_from_queue(){
  //   __disable_irq();
  //   auto status = rx_msg_buffor.get_front();
  //   if(!status.ok())
  //     return status;
  //   rx_msg_buffor.pop_front();
  //   __enable_irq();
  //   return status;
  // };

  /// @brief  Send a message to a CAN bus
  /// @param msg  CanMsg to be sent
  Status send_can_msg(CanMsg& msg) {
    if(HAL_CAN_GetTxMailboxesFreeLevel(can_interface) == 0)
      return Status::OutOfMemory("CAN TX buffer is full");

    CAN_TxHeaderTypeDef tx_header;
    tx_header.StdId              = msg.frame_id;
    tx_header.DLC                = msg.data_size;
    tx_header.RTR                = CAN_RTR_DATA;
    tx_header.IDE                = CAN_ID_STD;
    tx_header.TransmitGlobalTime = DISABLE;
    if(HAL_CAN_AddTxMessage(can_interface, &tx_header, msg.data, &last_tx_mailbox) != HAL_OK)
      return Status::IOError("CAN TX error");
    blink_tx_led();
    return Status::OK();
  };

  private:
  CAN_HandleTypeDef* can_interface;
  uint32_t can_fifo;
  std::shared_ptr<Timing> timing_led_rx;
  std::shared_ptr<Timing> timing_led_tx;
  std::shared_ptr<Timing> timing_can_task;
  uint8_t data[CAN_DATA_FRAME_MAX_SIZE];
  CAN_RxHeaderTypeDef header;
  // static_circular_buffor<CanMsg,CAN_QUEUE_SIZE> rx_msg_buffor;
  static_circular_buffor<CanMsg, CAN_QUEUE_SIZE> tx_msg_buffor;
  // std::unordered_map<uint32_t, void (*)(CanMsg&)> callback_map;
  etl::unordered_map<uint32_t, function_pointer, CAN_CALLBACK_LIST_SIZE> callback_map;

  const gpio::GpioPin* pin_tx_led;
  const gpio::GpioPin* pin_rx_led;
  uint32_t last_tx_mailbox;

  uint32_t filter_mask;
  uint32_t filter_base_id;

  /// @brief  Handle the leds
  void handle_leds() {
    if(pin_rx_led == nullptr || pin_tx_led == nullptr)
      return;
    if(timing_led_rx->triggered())
      WRITE_GPIO((*pin_rx_led), GPIO_PIN_RESET);

    if(timing_led_tx->triggered())
      WRITE_GPIO((*pin_tx_led), GPIO_PIN_RESET);
  };

  /// @brief  Handle the send of can messages if queie is used
  void handle_send() {
    auto result = tx_msg_buffor.get_front();
    if(!result.ok())
      return;
    if(send_can_msg(result.valueOrDie()).ok()) {
      (void)tx_msg_buffor.pop_front();
    }
  };

  /// @brief  Handle the receive of can messages if callback are used
  // void handle_receive(){
  //   // CanMsg rx_msg;
  //   auto meybe_msg = get_can_msg_from_queue();
  //   if(!meybe_msg.ok()) return;
  //   auto rx_msg = meybe_msg.valueOrDie();
  //   auto callback = callback_map.find(rx_msg.frame_id);
  //   if(callback != callback_map.end())
  //     callback->second(rx_msg);
  //   else
  //     callback_map[0](rx_msg);
  // };

  /// @brief  turn on the TX led for a short period
  void blink_tx_led() {
    WRITE_GPIO((*pin_tx_led), GPIO_PIN_SET);
    timing_led_tx->reset();
  };

  /// @brief  turn on the RX led for a short period
  void blink_rx_led() {
    WRITE_GPIO((*pin_rx_led), GPIO_PIN_SET);
    timing_led_rx->reset();
  };

  // /// @brief  push a message to the RX buffer
  // /// @param msg  CAN_MSG to be pushed to the buffer
  // void push_to_queue(CanMsg &msg){
  //   (void)rx_msg_buffor.push_back(msg);
  // }

  static void base_callback(CanMsg& msg) {
    __NOP();
  };
};

} // namespace stmepic
