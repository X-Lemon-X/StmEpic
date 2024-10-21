
#include "Timing.hpp"

#ifndef FILTER_HPP
#define FILTER_HPP

namespace stmepic::filters{

class FilterBase{
protected:
  Ticker &ticker;
public:
  FilterBase(Ticker &ticker);
  virtual float calculate(float calculate);
};


  
} // namespace FILTERS


#endif