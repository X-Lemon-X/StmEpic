
#include "ntc_termistors.hpp"
#include <cmath>
#include <limits>

using namespace NTCTERMISTORS;


NtcTermistors::NtcTermistors(float termistor_supply_voltage,float termistor_divider_resistance):
  termistor_supply_voltage(termistor_supply_voltage),
  termistor_divider_resisitor(termistor_divider_resistance) {}

float NtcTermistors::get_temperature(float termistor_voltage) {
  // float voltage = (float)(adc_value / 4095) * termistor_supply_voltage;
  float ntc_resistance = termistor_divider_resisitor * termistor_voltage / ( termistor_supply_voltage - termistor_voltage);
  float logR = std::log(ntc_resistance);
  float T = ( 1.0 / (NTC_TERMISTOR_C1 +  NTC_TERMISTOR_C2*logR + NTC_TERMISTOR_C3*logR*logR*logR) ) - 273.15;
  if (T < NTC_TERMISTOR_MIN_TEMPERATURE || T > NTC_TERMISTOR_MAX_TEMPERATURE ) T = std::numeric_limits<float>::quiet_NaN();
  return T;
}