
#include "filter_alfa_beta.hpp"
#include "Timing.hpp"
#include "filter.hpp"

using namespace stmepic::filters;

FilterAlfaBeta::FilterAlfaBeta(float _alfa, float _beta) {
  set_alfa(_alfa);
  set_beta(_beta);
  ypri  = 0;
  ypost = 0;
  vpri  = 0;
  vpost = 0;
}


float FilterAlfaBeta::calculate(float x) {
  x          = FilterSampleSkip::calculate(x);
  float time = Ticker::get_instance().get_seconds();
  float dt   = prev_time - time;
  ypri       = ypost + dt * vpost;
  vpri       = vpost;
  ypost      = ypri + alfa * (x - ypri);
  vpost      = vpri + beta * (x - ypri) / dt;
  prev_time  = time;
  return ypost;
}

void FilterAlfaBeta::set_init_value(float value) {
  FilterSampleSkip::set_init_value(value);
  ypri  = value;
  ypost = value;
  vpri  = 0;
  vpost = 0;
}

void FilterAlfaBeta::set_alfa(float _alfa) {
  alfa = _alfa;
}

void FilterAlfaBeta::set_beta(float _beta) {
  beta = _beta;
}