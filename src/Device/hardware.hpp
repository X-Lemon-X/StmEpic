#pragma once

#include "stmepic.hpp"
#include "device.hpp"


namespace stmepic {

enum class HardwareType { DMA, IRQ, BLOCKING };

class HardwareInterface {
public:
  HardwareInterface() = default;

  virtual ~HardwareInterface();

  /**
   * @brief Restarts the hardware interface
   *
   * @return Status if the hardware was restarted successfully.
   */
  [[nodiscard]] virtual Status hardware_reset() = 0;

  /**
   * @brief Starts the hardware interface
   *
   * @return Status if the hardware was started successfully.
   */
  [[nodiscard]] virtual Status hardware_start() = 0;

  /**
   * @brief Stops the hardware interface
   *
   * @return Status if the hardware was stopped successfully.
   */
  [[nodiscard]] virtual Status hardware_stop() = 0;

private:
  // wrapper for virtual stop function
  Status h_stop();
};

} // namespace stmepic