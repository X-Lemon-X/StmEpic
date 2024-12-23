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


namespace stmepic::sensors::MCP9700AT
{
static const float MCP9700AT_V0 = 0.54f;
static const float MCP9700AT_revTC = 100.0f;

/**
  * @brief Get the temperature from the MCP9700AT sensor
  * 
  * @param adc_value the value read from the ADC
  * @return float the temperature in Celsius
  */
float get_temperature(float adc_value);
  
} // namespace MCP9700AT
