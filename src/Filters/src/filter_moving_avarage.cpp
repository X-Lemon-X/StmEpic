#include "filter_moving_avarage.hpp"

using namespace stmepic::filters;


FilterMovingAvarage::FilterMovingAvarage() {
  samples = std::deque<float>();
}

float FilterMovingAvarage::calculate(float calculate) {
  if(samples_to_skip == 0)
    return calculate_moving_avarage(calculate);

  if(++sample_count >= samples_to_skip) {
    sample_count      = 0;
    last_value_sample = calculate;
  } else {
    return last_value;
  }

  last_value = calculate_moving_avarage(last_value_sample);
  return last_value;
}


void FilterMovingAvarage::set_size(uint16_t size) {
  this->size = size;
  samples.clear();
  for(uint16_t i = 0; i < size; ++i)
    samples.push_back(0);
}

void FilterMovingAvarage::set_samples_to_skip(uint16_t sample_amount) {
  samples_to_skip = sample_amount;
}

void FilterMovingAvarage::set_init_value(float value) {
  samples.clear();
  for(uint16_t i = 0; i < size; ++i)
    samples.push_back(value);
  last_value        = calculate_moving_avarage(value);
  last_value_sample = value;
}

float FilterMovingAvarage::calculate_moving_avarage(float calculate) {
  samples.push_back(calculate);
  samples.pop_front();
  float sum = 0;
  for(int i = 0; i < samples.size(); ++i)
    sum += samples[i];
  return (float)sum / samples.size();
}
