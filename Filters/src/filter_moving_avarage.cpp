#include "filter_moving_avarage.hpp"

using namespace FILTERS;


Filter_moving_avarage::Filter_moving_avarage(TIMING::Ticker &ticker): FilterBase(ticker){
  this->samples = std::deque<float>();
}

float Filter_moving_avarage::calculate(float calculate){
  this->samples.push_back(calculate);
  this->samples.pop_front();
  float sum = 0;
  for(int i=0; i < this->samples.size(); ++i) sum += this->samples[i];
  return (float)sum/this->samples.size();
}


void Filter_moving_avarage::set_size(uint16_t size){
  this->size = size;
  this->samples.clear();
  for (uint16_t i = 0; i < size; ++i) this->samples.push_back(0);
}