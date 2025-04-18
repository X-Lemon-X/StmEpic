#include "Timing.hpp"
#include "stmepic.hpp"
#include "status.hpp"
#include <cstdint>
#include <memory>
#include <string>


using namespace stmepic;

uint32_t stmepic::frequency_to_period_us(float frequency) {
  return (uint32_t)(1000000.0f / frequency);
}

Ticker::Ticker() : tick_millis(0), tick_micros(0), timer(nullptr), timer2(nullptr) {
}

void Ticker::irq_update_ticker() {
  tick_millis++;
  tick_micros = tick_millis * 1000;
}

Ticker &Ticker::get_instance() {
  static Ticker *ticker;
  if(ticker == nullptr) {
    ticker = new Ticker();
  }
  return *ticker;
}

void Ticker::init(TIM_HandleTypeDef *_timer, TIM_HandleTypeDef *_timer2) {
  timer       = _timer;
  timer2      = _timer2;
  tick_micros = 0;
  tick_millis = 0;

  //
}

uint32_t Ticker::get_micros() {
  if(timer == nullptr)
    return 0;
  vPortEnterCritical();
  uint32_t mic = (uint32_t)timer->Instance->CNT + tick_micros;
  vPortExitCritical();
  return mic;
}

uint32_t Ticker::get_millis() const {
  return tick_millis;
}

float Ticker::get_seconds() {
  return (float)get_micros() * 0.000001f;
}

void Ticker::delay(uint32_t miliseconds) {
  uint32_t start = get_micros();
  while(get_micros() - start < miliseconds) {
  }
}

void Ticker::delay_nop(uint32_t miliseconds) {
  miliseconds *= HAL_RCC_GetHCLKFreq() / 1000;
  for(uint32_t i = 0; i < miliseconds; i++) {
    __NOP();
  }
}

void Timer::set_behaviour(uint32_t _period, bool _repeat) {
  period = _period;
  repeat = _repeat;
}

Timer::Timer(Ticker &_ticker) : ticker(_ticker) {
  period         = 0;
  last_time      = ticker.get_micros();
  repeat         = true;
  timer_enabled  = true;
  function       = nullptr;
  triggered_flag = false;
}

Result<std::shared_ptr<Timer>> Timer::Make(uint32_t period, bool repeat, void (*function)(Timer &), Ticker &ticker) {
  auto new_timer = new Timer(ticker);
  new_timer->set_behaviour(period, repeat);
  new_timer->function = function;
  auto timer          = std::shared_ptr<Timer>(new_timer);
  return Result<decltype(timer)>::OK(timer);
}

void Timer::reset() {
  this->last_time      = ticker.get_micros() - 1001;
  this->triggered_flag = false;
}

void Timer::enable(bool timer_enabled) {
  this->timer_enabled = timer_enabled;
}

bool Timer::triggered() {
  uint32_t dif;
  uint32_t current_time = ticker.get_micros();

  if(!timer_enabled) {
    this->last_time = current_time;
    return false;
  }
  // why this is here?
  // because some times last_value is higher than the current_time why is that?
  // because the timer have  irq problems when the vale of the time is rapidly checked
  // which means that the timer has overflowed and the difference is gretaer than the period
  dif = current_time > this->last_time ? current_time - this->last_time : this->last_time - current_time;
  if(dif < this->period)
    return false;
  if(!repeat && triggered_flag)
    return false;
  this->triggered_flag = true;
  this->last_time      = current_time;

  return true;
}

void Timer::run_function() {
  if(!triggered())
    return;
  if(this->function == nullptr)
    return;
  this->function(*this);
}
