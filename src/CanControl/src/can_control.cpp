
#include "can_control.hpp"
#include "circular_buffor.hpp"
#include "list.hpp"
#include <cstring>
#include "stmepic.hpp"

using namespace stmepic;

CanControl::CanControl(){
  filter_mask = 0;
  filter_base_id = 0;
}

void CanControl::init(CAN_HandleTypeDef &_can_interface, uint32_t _can_fifo,Ticker &_ticker,const GpioPin &_pin_tx_led,const GpioPin &_pin_rx_led){
  can_interface = &_can_interface;
  can_fifo = _can_fifo;
  ticker = &_ticker;
  pin_rx_led = &_pin_rx_led;
  timing_led_rx = Timing::Make(*ticker,CAN_LED_BLINK_PERIOD_US,false);
  timing_led_tx = Timing::Make(*ticker,CAN_LED_BLINK_PERIOD_US,false);
  timing_led_rx->set_behaviour(CAN_LED_BLINK_PERIOD_US,false);
  timing_led_tx->set_behaviour(CAN_LED_BLINK_PERIOD_US,false);
  add_callback(0, base_callback);
}

void CanControl::push_to_queue(can_msg &msg){
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
  can_msg msg;
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

void CanControl::base_callback(can_msg &msg){
  __NOP();
}

void CanControl::handle_leds(){
  if(timing_led_rx->triggered())
    WRITE_GPIO((*pin_rx_led),GPIO_PIN_RESET);
  
  if(timing_led_tx->triggered())
    WRITE_GPIO((*pin_tx_led),GPIO_PIN_RESET);
}

void CanControl::handle_send(){
  auto result = tx_msg_buffor.get_front();
  if(!result.ok()) return;
  if(send_can_msg(result.valueOrDie()).ok()){
    (void)tx_msg_buffor.pop_front();
  }
}

void CanControl::handle_receive(){
  // can_msg rx_msg;
  __disable_irq();
  auto status = rx_msg_buffor.get_front();
  __enable_irq();
  if(!status.ok()) return;
  auto rx_msg = status.valueOrDie();
  auto callback = callback_map.find(rx_msg.frame_id);
  if(callback != callback_map.end()) 
    callback->second(rx_msg);
  else
    callback_map[0](rx_msg);
    
  rx_msg_buffor.pop_front();
}

void CanControl::handle(){
  handle_leds();
  handle_send();
  handle_receive();
}

Status CanControl::add_callback(uint32_t frame_id, function_pointer function){
  if(function == nullptr) return Status::ERROR("function pointer is null");
  callback_map[frame_id] = function;
}

Status CanControl::send_can_msg(can_msg &msg){
  if(HAL_CAN_GetTxMailboxesFreeLevel(can_interface)==0) 
    return Status::ERROR("CAN TX mailboxes are full");

  CAN_TxHeaderTypeDef tx_header;
  tx_header.StdId = msg.frame_id;
  tx_header.DLC = msg.data_size;
  tx_header.RTR = CAN_RTR_DATA;
  tx_header.IDE = CAN_ID_STD;
  tx_header.TransmitGlobalTime = DISABLE;
  if(HAL_CAN_AddTxMessage(can_interface,&tx_header,msg.data,&last_tx_mailbox)!=HAL_OK) 
    return Status::ERROR("CAN TX failed");
  blink_tx_led();
  return Status::OK();
}

Status CanControl::send_can_msg_to_queue(can_msg &msg){
  return tx_msg_buffor.push_back(msg);
}

Result<can_msg> CanControl::get_can_queue_message(){   
  auto status = rx_msg_buffor.get_front();
  if(!status.ok())
    return status;
  rx_msg_buffor.pop_front();
  return status;
}