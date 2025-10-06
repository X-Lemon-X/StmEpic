#include "ws2812b.hpp"

WS2812B::WS2812B(TIM_HandleTypeDef &htim, unsigned int timer_channel) : WS28Base(htim, timer_channel) {
  t1h_ns        = 800;
  t1l_ns        = 450;
  t0h_ns        = 400;
  t0l_ns        = 850;
  reset_time_ns = 50000;
}