
#include "MCP9700AT.hpp"

using namespace stmepic::sensors::temperature;

const float MCP9700AT::MCP9700AT_V0    = 0.54f;
const float MCP9700AT::MCP9700AT_revTC = 100.0f;

float stmepic::sensors::temperature::MCP9700AT::get_temperature(float sensor_voltage) {
  // Vout = TC X Ta + V0
  // Ta = (Vout - V0) / TC
  return (sensor_voltage - MCP9700AT_V0) * MCP9700AT_revTC;
}