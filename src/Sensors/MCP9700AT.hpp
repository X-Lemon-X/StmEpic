#pragma once
/**
 * @file MCP9700AT.hpp
 * @brief  MCP9700AT temperature sensor class definition.
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
 * @brief  MCP9700AT temperature sensor
 * MCP9700AT is a temperature sensor with a linear voltage output proportional to the temperature in Celsius.

 */

class MCP9700AT {
  static const float MCP9700AT_V0;
  static const float MCP9700AT_revTC;

public:
  /// @brief Get the temperature from the MCP9700AT sensor
  /// @param sensor_voltage the voltage output from the sensor
  /// @return the temperature in Celsius
  static float get_temperature(float sensor_voltage);
};

} // namespace stmepic::sensors::temperature
