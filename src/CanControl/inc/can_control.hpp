#include "pin.hpp"
#include "Timing.hpp"
#include "circular_buffor.hpp"
#include "stmepic.hpp"
#include "stmepic_status.hpp"
#include <cstdint>
#include <memory>
#include <unordered_map>

#ifndef CAN_CONTROL_HPP
#define CAN_CONTROL_HPP

#ifndef CAN_DATA_FRAME_MAX_SIZE
#define CAN_DATA_FRAME_MAX_SIZE 8
#endif

#ifndef CAN_QUEUE_SIZE
#define CAN_QUEUE_SIZE 128
#endif

#ifndef CAN_LED_BLINK_PERIOD_US
#define CAN_LED_BLINK_PERIOD_US 1000
#endif

namespace stmepic {

struct can_msg{
  uint32_t frame_id;
  bool remote_request;
  uint8_t data[CAN_DATA_FRAME_MAX_SIZE];
  uint8_t data_size;

};

class CanControl{
public:
  using function_pointer = void (*)(can_msg &);
  /// @brief  Construct a new Can Control object
  CanControl();
  /// @brief  init the CanControl object
  /// @param can_interface  CAN interface
  /// @param can_fifo  CAN fifo number
  /// @param ticker  main system ticker object
  /// @param pin_tx_led  pin object for the TX led
  /// @param pin_rx_led  pin object for the RX led
  void init(CAN_HandleTypeDef &can_interface,uint32_t can_fifo,Ticker &ticker,const GpioPin &pin_tx_led,const GpioPin &pin_rx_led);

  /// @brief  Add a callback function to the CAN interface
  /// @param frame_id  frame id of the message, set to 0 to receive all messages
  /// @param function_pointer  pointer to the function that will be called when the message with the frame_id is received
  Status add_callback(uint32_t frame_id, function_pointer);


  /// @brief  Add a filter to the CAN interface. Why because i can't get the hardware filters to work properly!
  /// @param base_id  base id of the filter
  /// @param mask  mask of the filter
  void set_filter(uint32_t base_id, uint32_t mask);

  /// @brief  Handle the RX interrupt
  void irq_handle_rx();

  /// @brief  Handle the TX interrupt
  void irq_handle_tx();

  /// @brief  This function should be called in the main loop of the uc program
  ///         It will handle the RX and TX incoming data, leds and other tasks that require updating
  void handle();

  /// @brief  Handle the leds
  void handle_leds();

  /// @brief  Handle the send of can messages if queie is used
  void handle_send();

  /// @brief  Handle the receive of can messages if callback are used
  void handle_receive();

  Status send_can_msg_to_queue(can_msg &msg);
  
  /// @brief  Get the message from the RX buffer
  /// @param msg  pointer to the CAN_MSG object
  /// @return 0 if success
  Result<can_msg> get_can_queue_message();

  /// @brief  Send a message to a CAN bus
  /// @param msg  pointer to the CAN_MSG object
  Status send_can_msg(can_msg &msg);

private:
  CAN_HandleTypeDef *can_interface;
  uint32_t can_fifo;
  Ticker *ticker;
  std::shared_ptr<Timing> timing_led_rx;
  std::shared_ptr<Timing> timing_led_tx;
  uint8_t data[CAN_DATA_FRAME_MAX_SIZE];
  CAN_RxHeaderTypeDef header;
  static_circular_buffor<can_msg,CAN_QUEUE_SIZE> rx_msg_buffor;
  static_circular_buffor<can_msg,CAN_QUEUE_SIZE> tx_msg_buffor;
  std::unordered_map<uint32_t, function_pointer> callback_map;

  const GpioPin *pin_tx_led;
  const GpioPin *pin_rx_led;
  uint32_t last_tx_mailbox;

  uint32_t filter_mask;
  uint32_t filter_base_id;

  /// @brief  turn on the TX led for a short period
 void blink_tx_led();

  /// @brief  turn on the RX led for a short period
  void blink_rx_led();

  /// @brief  push a message to the RX buffer
  /// @param msg  CAN_MSG to be pushed to the buffer
  void push_to_queue(can_msg &msg);

  static void base_callback(can_msg &msg);
};

} // namespace CAN_CONTROL

#endif // CAN_CONTROL_HPP