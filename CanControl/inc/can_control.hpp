#include "stm32f4xx_hal.h"
#include "pin.hpp"
#include "Timing.hpp"
#include "circular_buffor.hpp"

#ifndef CAN_CONTROL_HPP
#define CAN_CONTROL_HPP

#define CAN_DATA_FRAME_MAX_SIZE 8
#define CAN_QUEUE_SIZE 128
#define CAN_LED_BLINK_PERIOD_US 1000

namespace CAN_CONTROL {

struct CAN_MSG{
  uint32_t frame_id;
  bool remote_request;
  uint8_t data[CAN_DATA_FRAME_MAX_SIZE];
  uint8_t data_size;

};

class CanControl{
private:
  CAN_HandleTypeDef *can_interface;
  uint32_t can_fifo;
  TIMING::Ticker *ticker;
  TIMING::Timing *timing_led_rx;
  TIMING::Timing *timing_led_tx;
  uint8_t data[CAN_DATA_FRAME_MAX_SIZE];
  CAN_RxHeaderTypeDef header;
  // LIST_EMB::List<CAN_MSG*> rx_msg_buffer;
  // LIST_EMB::List<CAN_MSG*> tx_msg_buffer;
  CIRCULAR_BUFFOR::static_circular_buffor<CAN_MSG,CAN_QUEUE_SIZE> rx_msg_buffor;
  CIRCULAR_BUFFOR::static_circular_buffor<CAN_MSG,CAN_QUEUE_SIZE> tx_msg_buffor;

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
  void push_to_queue(CAN_MSG &msg);
public:
  
  /// @brief  Construct a new Can Control object
  CanControl();

  ~CanControl();

  /// @brief  init the CanControl object
  /// @param can_interface  CAN interface
  /// @param can_fifo  CAN fifo number
  /// @param ticker  main system ticker object
  /// @param pin_tx_led  pin object for the TX led
  /// @param pin_rx_led  pin object for the RX led
  void init(CAN_HandleTypeDef &can_interface,uint32_t can_fifo,TIMING::Ticker &ticker,const GpioPin &pin_tx_led,const GpioPin &pin_rx_led);

  /// @brief  Add a filter to the CAN interface. Why because i can't get the hardware filters to work properly!
  /// @param base_id  base id of the filter
  /// @param mask  mask of the filter
  void set_filter(uint32_t base_id, uint32_t mask);

  /// @brief  Handle the RX interrupt
  void irq_handle_rx();

  /// @brief  Handle the TX interrupt
  void irq_handle_tx();

  /// @brief  This function should be called in the main loop of the uc program
  ///         It will handle the RX and TX leds and other tasks that require some updates
  void handle();

  /// @brief  Send a message to a CAN bus
  /// @param msg  pointer to the CAN_MSG object
  uint8_t send_msg(CAN_MSG &msg);

  uint8_t send_msg_to_queue(CAN_MSG &msg);
  
  /// @brief  Get the message from the RX buffer
  /// @param msg  pointer to the CAN_MSG object
  /// @return 0 if success
  uint8_t get_message(CAN_MSG *msg);
};

} // namespace CAN_CONTROL

#endif // CAN_CONTROL_HPP