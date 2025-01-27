#pragma once

#include "stmepic.hpp"


/**
 * @file gpio.hpp
 * @brief Gpio pin definitions for quicker implementation.
 */


#ifndef ANALOG_ADC_VAKUE_TO_VOLTAGE
#define ANALOG_ADC_VAKUE_TO_VOLTAGE 0.000805861f
#endif


/// @brief  write gpio pin binary value 1 or 0
/// @param gpio_pin  the pin to write to
/// @param value  the value to write
#define WRITE_GPIO(gpio_pin, value) \
  HAL_GPIO_WritePin(gpio_pin.port, gpio_pin.pin, static_cast<GPIO_PinState>(value))

/// @brief  read gpio pin binary value 1 or 0
/// @param gpio_pin  the pin to read
#define READ_GPIO(gpio_pin) (uint8_t) HAL_GPIO_ReadPin(gpio_pin.port, gpio_pin.pin)

/// @brief  toggle gpio pin
/// @param gpio_pin  the pin to toggle
#define TOGGLE_GPIO(gpio_pin) HAL_GPIO_TogglePin(gpio_pin.port, gpio_pin.pin)

/// @brief  get the voltage value from the analog pin only for 12 bit ADC and 3.3V reference voltage
/// @param gpio_pin  the pin to read the voltage from
#define VOLTAGE_VALUE(gpio_pin) (float)gpio_pin.analog_value *ANALOG_ADC_VAKUE_TO_VOLTAGE

namespace stmepic::gpio {

/**
 * @struct GpioPin
 * @brief Structure representing the GPIO pin.
 *
 * This structure provides the GPIO pin representation with the port and pin number.
 * and analog value if the pin is analog.
 * This allows to define the pinout configuration in a single file
 * Making code more portable and easier to maintain.
 */
struct GpioPin {
  uint16_t pin;
  GPIO_TypeDef *port;
  uint16_t analog_value;
};

} // namespace stmepic::gpio
