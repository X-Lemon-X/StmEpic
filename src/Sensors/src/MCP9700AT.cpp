
#include "MCP9700AT.hpp"

using namespace stmepic::sensors::MCP9700AT;

float stmepic::sensors::MCP9700AT::get_temperature(float  sensor_voltage){
  // Vout = TC X Ta + V0
  // Ta = (Vout - V0) / TC
  return (sensor_voltage - MCP9700AT_V0) * MCP9700AT_revTC; 
}