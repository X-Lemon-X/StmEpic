#include "Timing.hpp"
// #include "main.h"
#include "stm32f4xx_hal.h"


using namespace TIMING;

uint32_t TIMING::frequency_to_period(float frequency){
  return (uint32_t)(1000000.0f/frequency);
}

Ticker::Ticker(){
  tick_millis = 0;
  tick_micros = 0;
}

void Ticker::irq_update_ticker(){
  tick_millis++;
  tick_micros = tick_millis*1000;
}

uint32_t Ticker::get_micros() {
  __disable_irq();
  uint32_t mic =  (uint32_t)TIM10->CNT + tick_micros;
  __enable_irq();
  return mic;
}

uint32_t Ticker::get_millis() const {
  return tick_millis;
}

float Ticker::get_seconds() {
  return (float)get_micros() * 0.000001f;
}

void Timing::set_behaviour(uint32_t _period, bool _repeat){
  period = _period;
  repeat = _repeat;
}

Timing::Timing(Ticker &_ticker): ticker(_ticker){
  period = 0;
  last_time = ticker.get_micros();
  repeat = true;
  timer_enabled=true;
}

void Timing::reset(){
  this->last_time = ticker.get_micros() - 10;
}

void Timing::enable(bool timer_enabled){
  this->timer_enabled = timer_enabled;
}

bool Timing::triggered(){
  uint32_t dif;
  uint32_t current_time = ticker.get_micros();

  if(!timer_enabled){
    this->last_time = current_time;
    return false;
  }
  // why this is here?
  // because some times last_value is higher than the current_time why is that?
  // because the timer have  irq problems when the vale of the time is rapidly checked
  // which means that the timer has overflowed and the difference is gretaer than the period
  dif = current_time > this->last_time? current_time - this->last_time : this->last_time - current_time;
  if (dif < this->period) return false;
  if (repeat) this->last_time = current_time;
  return true;
}