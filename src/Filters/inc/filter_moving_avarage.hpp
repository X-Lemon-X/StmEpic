
#include "filter.hpp"
#include "Timing.hpp"
#include <deque>

#ifndef MOVING_AVARAGE_HPP
#define MOVING_AVARAGE_HPP

namespace stmepic::filters{

class Filter_moving_avarage : public FilterBase{
private:
  std::deque<float> samples;
  uint16_t size;
public:
  Filter_moving_avarage(Ticker &ticker);
  float calculate(float calculate) override;

  void set_size(uint16_t size);

};

} // namespace FILTERS
#endif