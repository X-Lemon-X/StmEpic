#pragma once
#include "Timing.hpp"
#include <cstdint>

/**
 * @file filter.hpp
 * @brief Base class for all filters.
 */

/**
 * @defgroup Filters
 * @brief Filters module for base filet interface with filter implementations for filtering data.
 * @{
 */


namespace stmepic::filters {

/**
 * @class FilterBase
 * @brief Base class for all filters.
 *
 * Used as base class for all filters
 * Making it simple to change and test new filters for sensors.
 */
class FilterBase {
public:
  /// @brief Construct a new Filter Base object
  FilterBase() = default;

  /// @brief Update the filter with new data and exceute the filter to calculate the output of the filter
  /// @param calculate the new data to update the filter with.
  /// @return the output of the filter
  virtual float calculate(float calculate) = 0;

  /// @brief sets the amount of samples to skip, wby prbably the most bacis filter seting
  virtual void set_samples_to_skip(uint16_t sample_amount) = 0;

  /// @brief Sets the initial value of the filter usefull when
  /// you want to avoid disturbance during the start of the filter.
  /// @param value the initial value of the filter
  virtual void set_init_value(float value) = 0;
};


/**
 * @class FilterSampleSkip
 * @brief Filter that skips samples.
 * Simple sample skip filter, to take every N-th sample.
 */
class FilterSampleSkip : public FilterBase {
public:
  /// @brief Construct a new Filter Sample Skip object
  /// simple filter that skips samples
  FilterSampleSkip();
  virtual float calculate(float x) override;

  /// @brief sets the amount of samples to skip
  void set_samples_to_skip(uint16_t sample_amount) override;

  /// @brief sets the initial value of the filter
  void set_init_value(float value) override;

private:
  uint16_t samples_to_skip;
  float last_value;
  uint16_t sample_count;
};

} // namespace stmepic::filters