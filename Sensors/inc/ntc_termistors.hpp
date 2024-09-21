
#ifndef __NTC_TERMISTORS_HPP
#define __NTC_TERMISTORS_HPP

#define NTC_TERMISTOR_C1 1.009249522e-03f
#define NTC_TERMISTOR_C2 2.378405444e-04f
#define NTC_TERMISTOR_C3 2.019202697e-07f
#define NTC_TERMISTOR_MIN_TEMPERATURE -40.0f
#define NTC_TERMISTOR_MAX_TEMPERATURE 150.0f

namespace NTCTERMISTORS{

class NtcTermistors {
private:
  const float termistor_supply_voltage;
  const float termistor_divider_resisitor;
public:
  NtcTermistors(float termistor_supply_voltage,float termistor_divider_resistance);
  float get_temperature(float voltage_value);
};


} // namespace NTCTERMISOTRS


#endif