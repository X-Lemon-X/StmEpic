#include "filter.hpp"

using namespace FILTERS;

FilterBase::FilterBase(TIMING::Ticker &_ticker): ticker(_ticker){}

float FilterBase::calculate(float x){return x;}

