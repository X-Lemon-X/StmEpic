#include "filter.hpp"

using namespace stmepic::filters;

FilterBase::FilterBase(Ticker &_ticker): ticker(_ticker){}

float FilterBase::calculate(float x){return x;}

