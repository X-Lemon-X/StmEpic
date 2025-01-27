#pragma once
#include "Timing.hpp"
#include "filter.hpp"
#include <deque>


/**
 * @file filter_moving_avarage.hpp
 * @brief Moving avarage filter implementation.
 */

/**
 * @defgroup Filters
 * @{
 */


namespace stmepic::filters {

/**
 * @class FilterMovingAvarage
 * @brief Moving avarage filter implementation.
 *
 * Moving avarage filter implementation.
 * For simple data filtration with super simple setup.
 */
class FilterMovingAvarage : public FilterBase {

public:
  FilterMovingAvarage();
  float calculate(float calculate) override;
  void set_size(uint16_t size);
  void set_samples_to_skip(uint16_t sample_amount) override;
  void set_init_value(float value) override;

private:
  float calculate_moving_avarage(float calculate);

  std::deque<float> samples;
  uint16_t size;
  uint32_t sample_count;
  uint16_t samples_to_skip;
  float last_value_sample;
  float last_value;
};

} // namespace stmepic::filters