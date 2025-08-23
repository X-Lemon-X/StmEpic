#include "ws28.hpp"

namespace stmepic {

class WS2812B : public WS28Base {
public:
  WS2812B(TIM_HandleTypeDef &htim, unsigned int timer_channel);
};

}