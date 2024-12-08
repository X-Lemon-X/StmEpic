
#include "Timing.hpp"
#include <cstdint>

#ifndef FILTER_HPP
#define FILTER_HPP

namespace stmepic::filters{

class FilterBase{
public:
  
  FilterBase() = default;
  /// @brief Construct a new Filter Base object
  /// used as base class for all filters
  /// ir simple sample skipp filter
  virtual float calculate(float calculate) =0;
  virtual void set_samples_to_skip(uint16_t sample_amount) = 0;
  virtual void set_init_value(float value) = 0;
};


class FilterSampleSkip : public FilterBase{
public:

  /// @brief Construct a new Filter Sample Skip object
  /// simple filter that skips samples
  FilterSampleSkip();
  float calculate(float x) override;

  /// @brief sets the amount of samples to skip
  void set_samples_to_skip(uint16_t sample_amount) override;

  /// @brief sets the initial value of the filter
  void set_init_value(float value) override;

private:
  uint16_t samples_to_skip;
  float last_value;
  uint16_t sample_count;
};


  
} // namespace FILTERS


#endif