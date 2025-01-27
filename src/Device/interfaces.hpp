#pragma once

#include "stmepic.hpp"
#include "device.hpp"


namespace stmepic {

enum class HardwareTy { DMA, IRQ, BLOCKING };

class HardwareInterface {
public:
  HardwareInterface()                  = default;
  virtual ~HardwareInterface()         = default;
  [[nodiscard]] virtual Status reset() = 0;
  [[nodiscard]] virtual Status start() = 0;
  [[nodiscard]] virtual Status stop()  = 0;
};

} // namespace stmepic