#include "stm32f4xx_hal.h"

#ifndef PIN_HPP
#define PIN_HPP

#ifndef ANALOG_ADC_VAKUE_TO_VOLTAGE
#define ANALOG_ADC_VAKUE_TO_VOLTAGE 0.000805861f
#endif


#define WRITE_GPIO(gpio_pin, value) HAL_GPIO_WritePin(gpio_pin.port, gpio_pin.pin, static_cast<GPIO_PinState>(value))
#define READ_GPIO(gpio_pin) (uint8_t)HAL_GPIO_ReadPin(gpio_pin.port, gpio_pin.pin)
#define TOGGLE_GPIO(gpio_pin) HAL_GPIO_TogglePin(gpio_pin.port, gpio_pin.pin)

/// @brief  get the voltage value from the analog pin only for 12 bit ADC and 3.3V reference voltage
/// @param gpio_pin  the pin to read the voltage from
#define VOLTAGE_VALUE(gpio_pin) (float)gpio_pin.analog_value * 0.000805861f



struct GpioPin {
  uint16_t pin;
  GPIO_TypeDef *port;
  uint16_t analog_value;
};






#endif