
#ifndef __MCP9700AT_HPP
#define __MCP9700AT_HPP


#define MCP9700AT_V0 0.54f //  offset voltage 500mV 
#define MCP9700AT_revTC 100.0f // 1/TC,  TC = Temperature Coefficient 10mV/°C -> 0.1V/°C

namespace MCP9700AT
{

float get_temperature(float adc_value);
  
} // namespace MCP9700AT


#endif