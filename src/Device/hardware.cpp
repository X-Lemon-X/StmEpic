#include "stmepic.hpp"
#include "hardware.hpp"

using namespace stmepic;


HardwareInterface::~HardwareInterface() {
  (void)h_stop();
}


Status HardwareInterface::h_stop() {
  return hardware_stop();
}