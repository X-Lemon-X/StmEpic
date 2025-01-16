#include "filter.hpp"

using namespace stmepic::filters;

FilterSampleSkip::FilterSampleSkip() {
  samples_to_skip = 0;
  last_value      = 0;
  sample_count    = 0;
}

float FilterSampleSkip::calculate(float x) {
  if(samples_to_skip == 0)
    return x;

  if(++sample_count >= samples_to_skip) {
    sample_count = 0;
    last_value   = x;
  }
  return last_value;
}

void FilterSampleSkip::set_samples_to_skip(uint16_t sample_amount) {
  samples_to_skip = sample_amount;
}

void FilterSampleSkip::set_init_value(float value) {
  last_value = value;
}
