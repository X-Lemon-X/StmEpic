#pragma once

#include "stmepic.hpp"

/**
 * @file ntc_termistors.hpp
 * @brief  NtcTermistors class definition for read temperature from NTC termistors.
 */

/**
 * @defgroup Sensors
 * @brief Functions related to different sensors.
 * @{
 */

/**
 * @defgroup Temperature_Sensors Temperature Sensors
 * @brief Functions related to temperature sensors.
 * @{
 */

namespace stmepic::sensors::temperature {

/**
 * @brief Get the temperature from the NTC termistor
 * Like 10k or 100k or any other NTC termistor
 */
class NtcTermistors {
public:
  /**
   * @brief Construct a new NtcTermistors object
   * Vcc |------[R1]----------[NTC]--------------|| GND
   * The libarby expects that the NTC is termistor resistance is about the same as R1-resistor
   * @param termistor_supply_voltage the supply voltage of the termistor
   * @param termistor_divider_resistance the resistance of the divider Resistor R1
   */
  NtcTermistors(float termistor_supply_voltage, float termistor_divider_resistance);
  float get_temperature(float voltage_value);

private:
  const float termistor_supply_voltage;
  const float termistor_divider_resisitor;

  const static float NTC_TERMISTOR_C1;
  const static float NTC_TERMISTOR_C2;
  const static float NTC_TERMISTOR_C3;
  const static float NTC_TERMISTOR_MIN_TEMPERATURE;
  const static float NTC_TERMISTOR_MAX_TEMPERATURE;
};


} // namespace stmepic::sensors::temperature
