#pragma once

#include "stmepic.hpp"

/**
 * @file ntc_termistors.hpp
 * @brief  NtcTermistor class definition for read temperature from NTC termistors.
 */

/**
 * @defgroup Sensors
 * @brief Functions related to different sensors.
 * @{
 */

/**
 * @defgroup Thermometers_Sensors Thermometers
 * @brief Functions related to temperature sensors.
 * @{
 */


/**
 * @defgroup NtcTermistor_imu_sensors Ntc Termistor
 * @brief Generic Ntc Termistor.
 * @{
 */

namespace stmepic::sensors::temperature {

/**
 * @brief Get the temperature from the NTC termistor
 * Like 10k or 100k or any other NTC termistor
 */
class NtcTermistor {
public:
  /**
   * @brief Construct a new NtcTermistor object
   * Vcc |------[R1]----------[NTC]--------------|| GND
   * The libarby expects that the NTC is termistor resistance is about the same as R1-resistor
   * @param termistor_supply_voltage the supply voltage of the termistor
   * @param termistor_divider_resistance the resistance of the divider Resistor R1
   */
  NtcTermistor(float termistor_supply_voltage, float termistor_divider_resistance);
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
