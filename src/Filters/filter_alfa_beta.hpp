#pragma once
#include "Timing.hpp"
#include "filter.hpp"


/**
 * @file filter_alfa_beta.hpp
 * @brief Simple alfa beta filter implementation.
 */

/**
 * @defgroup Filters
 * @{
 */

namespace stmepic::filters {

/**
 * @class FilterAlfaBeta
 * @brief Simple alfa beta filter implementation.
 *
 * Simple alfa beta filter implementation.
 * For simple data filtration with super simple setup.
 */
class FilterAlfaBeta : public FilterSampleSkip {
public:
  FilterAlfaBeta();

  float calculate(float x) override;

  void set_init_value(float value) override;

  /// @brief Set the alfa value for the filter
  /// @param alfa the alfa value
  void set_alfa(float alfa);

  /// @brief Set the beta value for the filter
  /// @param beta the beta value
  void set_beta(float beta);

private:
  float alfa;
  float beta;
  float ypri;
  float ypost;
  float vpri;
  float vpost;
  float prev_time;
};

} // namespace stmepic::filters