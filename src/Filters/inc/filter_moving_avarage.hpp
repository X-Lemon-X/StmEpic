
#include "filter.hpp"
#include "Timing.hpp"
#include <deque>

#ifndef MOVING_AVARAGE_HPP
#define MOVING_AVARAGE_HPP

namespace stmepic::filters{

class FilterMovingAvarage : public FilterBase{
private:
  std::deque<float> samples;
  uint16_t size;
  uint32_t sample_count;
  uint16_t samples_to_skip;
  float last_value_sample;
  float last_value;

  float calculate_moving_avarage(float calculate);
public:
  FilterMovingAvarage();
  float calculate(float calculate) override;
  void set_size(uint16_t size);
  void set_samples_to_skip(uint16_t sample_amount) override;
  void set_init_value(float value) override;

};

} // namespace FILTERS
#endif