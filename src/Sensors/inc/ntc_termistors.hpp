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

namespace stmepic::sensors::NTCTERMISTORS{

class NtcTermistors {
private:
  const float termistor_supply_voltage;
  const float termistor_divider_resisitor;
public:
  NtcTermistors(float termistor_supply_voltage,float termistor_divider_resistance);
  float get_temperature(float voltage_value);
};


} // namespace NTCTERMISOTRS

