
#include "can_control.hpp"
#include "list.hpp"
#include "circular_buffor.hpp"
#include <cstring>

using namespace CAN_CONTROL;

CanControl::CanControl(){
  filter_mask = 0;
  filter_base_id = 0;
}

CanControl::~CanControl(){
  delete timing_led_rx;
  delete timing_led_tx;
}

void CanControl::init(CAN_HandleTypeDef &_can_interface, uint32_t _can_fifo,TIMING::Ticker &_ticker,const GpioPin &_pin_tx_led,const GpioPin &_pin_rx_led){
  can_interface = &_can_interface;
  can_fifo = _can_fifo;
  ticker = &_ticker;
  pin_tx_led = &_pin_tx_led;
  pin_rx_led = &_pin_rx_led;
  timing_led_rx = new TIMING::Timing(*ticker);
  timing_led_tx = new TIMING::Timing(*ticker);
  timing_led_rx->set_behaviour(CAN_LED_BLINK_PERIOD_US,false);
  timing_led_tx->set_behaviour(CAN_LED_BLINK_PERIOD_US,false);
}

void CanControl::push_to_queue(CAN_MSG &msg){
  (void)rx_msg_buffor.push_back(msg);
}

void CanControl::set_filter(uint32_t base_id, uint32_t mask){
  filter_mask = mask;
  filter_base_id = base_id;
}

void CanControl::blink_tx_led(){
  WRITE_GPIO((*pin_tx_led),GPIO_PIN_SET);
  timing_led_tx->reset();
}

void CanControl::blink_rx_led(){
  WRITE_GPIO((*pin_rx_led),GPIO_PIN_SET);
  timing_led_rx->reset();
}

void CanControl::irq_handle_rx(){
  blink_rx_led();
  if (HAL_CAN_GetRxMessage(can_interface, can_fifo, &header, data) != HAL_OK) return;
  if (((header.StdId & filter_mask) != filter_base_id) && ((header.ExtId & filter_mask) != filter_base_id)) return;
  CAN_MSG msg;
  if(header.IDE == CAN_ID_EXT)
    msg.frame_id = header.ExtId;
  else
    msg.frame_id = header.StdId;
  msg.remote_request = header.RTR == CAN_RTR_REMOTE;
  msg.data_size = header.DLC;
  if(!msg.remote_request) memcpy(msg.data,data,msg.data_size);
  (void)rx_msg_buffor.push_back(msg);
}

void CanControl::irq_handle_tx(){
  __NOP();
}

void CanControl::handle(){
  if(this->timing_led_rx->triggered())
    WRITE_GPIO((*pin_rx_led),GPIO_PIN_RESET);
  
  if(this->timing_led_tx->triggered())
    WRITE_GPIO((*pin_tx_led),GPIO_PIN_RESET);

  CAN_MSG msg;
  if(tx_msg_buffor.get_front(&msg)!=0)
    return;
  if(send_msg(msg) == 0){
    (void)tx_msg_buffor.pop_front();
  }
}

uint8_t CanControl::send_msg(CAN_MSG &msg){
  if(HAL_CAN_GetTxMailboxesFreeLevel(can_interface)==0) 
    return 1;

  CAN_TxHeaderTypeDef tx_header;
  tx_header.StdId = msg.frame_id;
  tx_header.DLC = msg.data_size;
  tx_header.RTR = CAN_RTR_DATA;
  tx_header.IDE = CAN_ID_STD;
  tx_header.TransmitGlobalTime = DISABLE;
  if(HAL_CAN_AddTxMessage(can_interface,&tx_header,msg.data,&last_tx_mailbox)!=HAL_OK) 
    return 2;
  blink_tx_led();
  return 0;
}

uint8_t CanControl::send_msg_to_queue(CAN_MSG &msg){
  return tx_msg_buffor.push_back(msg);
}

uint8_t CanControl::get_message(CAN_MSG *msg){   
  uint8_t status =  rx_msg_buffor.get_front(msg);
  if(status!=0)
    return status;
  rx_msg_buffor.pop_front();
  return status;
}