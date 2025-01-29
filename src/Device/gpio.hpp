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
// #define WRITE_GPIO(gpio_pin, value) \
//   HAL_GPIO_WritePin(&(gpio_pin.port), gpio_pin.pin, static_cast<GPIO_PinState>(value))

// /// @brief  read gpio pin binary value 1 or 0
// /// @param gpio_pin  the pin to read
// #define READ_GPIO(gpio_pin) (uint8_t) HAL_GPIO_ReadPin(&(gpio_pin.port), gpio_pin.pin)

// /// @brief  toggle gpio pin
// /// @param gpio_pin  the pin to toggle
// #define TOGGLE_GPIO(gpio_pin) HAL_GPIO_TogglePin(&(gpio_pin.port), gpio_pin.pin)

// /// @brief  get the voltage value from the analog pin only for 12 bit ADC and 3.3V reference voltage
// /// @param gpio_pin  the pin to read the voltage from
// #define VOLTAGE_VALUE(gpio_pin) (float)gpio_pin.analog_value *ANALOG_ADC_VAKUE_TO_VOLTAGE

namespace stmepic {

/**
 * @struct GpioPin
 * @brief Structure representing the GPIO pin.
 *
 * This structure provides the GPIO pin representation with the port and pin number.
 * and analog value if the pin is analog.
 * This allows to define the pinout configuration in a single file
 * Making code more portable and easier to maintain.
 */

static const uint16_t GPIO_ANALOG_RESOLUTION_8BIT  = 255;
static const uint16_t GPIO_ANALOG_RESOLUTION_10BIT = 1023;
static const uint16_t GPIO_ANALOG_RESOLUTION_12BIT = 4095;
static const uint16_t GPIO_ANALOG_RESOLUTION_14BIT = 16383;
static const uint16_t GPIO_ANALOG_RESOLUTION_16BIT = 65535;


class GpioPin {
public:
  /**
   * @brief Construct a new Gpio Pin object to easyly pass gpio pins to other peripherals
   *
   * @param port GPIO port
   * @param pin GPIO pin
   */
  GpioPin(GPIO_TypeDef &port, uint16_t pin);


  /// @brief Write 1 or 0 to the gpio pin
  /// @param value 1 or 0
  void write(uint8_t value);

  /// @brief Read the value of the gpio pin
  /// @return 1 or 0
  uint8_t read();

  /// @brief Toggles the gpio pin from 1->0 or 0->1 respectively
  void toggle();

  uint16_t analog_value;
  GPIO_TypeDef &port;
  uint16_t pin;
};

class GpioAnalog : public GpioPin {
public:
  GpioAnalog(GPIO_TypeDef &port, uint16_t pin, const float ref_voltage = 3.3f, const uint16_t _resolution = GPIO_ANALOG_RESOLUTION_12BIT);

  float get_voltage();


private:
  const uint32_t resolution;
  const float value_to_voltage_multiplayer;
};

} // namespace stmepic
