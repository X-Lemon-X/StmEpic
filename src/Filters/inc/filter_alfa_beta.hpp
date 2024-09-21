
#include "filter.hpp"
#include "Timing.hpp"

#ifndef FILTER_ALFA_BETA_HPP
#define FILTER_ALFA_BETA_HPP

namespace FILTERS{

class FilterAlfaBeta: public FilterBase{
private:
  float ypri;
  float ypost;
  float vpri;
  float vpost;
  float prev_time;
public:
  float alfa;
  float beta;
FilterAlfaBeta(TIMING::Ticker &ticker);
 float calculate(float x);
};

} // namespace FILTERS
#endif