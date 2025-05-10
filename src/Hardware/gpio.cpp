
#include "stmepic.hpp"
#include "gpio.hpp"

using namespace stmepic;

static const uint16_t GPIO_ANALOG_RESOLUTION_8BIT  = 255;
static const uint16_t GPIO_ANALOG_RESOLUTION_10BIT = 1023;
static const uint16_t GPIO_ANALOG_RESOLUTION_12BIT = 4095;
static const uint16_t GPIO_ANALOG_RESOLUTION_14BIT = 16383;
static const uint16_t GPIO_ANALOG_RESOLUTION_16BIT = 65535;

GpioPin::GpioPin(GPIO_TypeDef &port, uint16_t pin) : analog_value(0), port(port), pin(pin) {
}
void GpioPin::write(uint8_t value) {
  HAL_GPIO_WritePin(&port, pin, static_cast<GPIO_PinState>(value));
}

uint8_t GpioPin::read() {
  return (uint8_t)HAL_GPIO_ReadPin(&port, pin);
}

void GpioPin::toggle() {
  HAL_GPIO_TogglePin(&port, pin);
}


GpioAnalog::GpioAnalog(GPIO_TypeDef &port, uint16_t pin, const float ref_voltage, const uint16_t _resolution)
: GpioPin(port, pin), resolution(_resolution), value_to_voltage_multiplayer(ref_voltage / (float)_resolution){};

float GpioAnalog::get_voltage() {
  return (float)analog_value * value_to_voltage_multiplayer;
}
