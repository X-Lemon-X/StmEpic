#pragma once

#include "stmepic.hpp"
#include "device.hpp"


/**
 * @file hardware.hpp
 * @brief This file contains the HardwareInterface base class definition.
 * HardwareInterface is used to provide a common interface for hardware control. Like I2C, SPI, UART, etc.
 */


namespace stmepic {

/**
 * @enum HardwareType
 * @brief Enumeration representing various types of hardware.
 *
 * @var HardwareType::DMA
 * Direct Memory Access hardware.
 *
 * @var HardwareType::IT
 * Interrupt Request hardware.
 *
 * @var HardwareType::BLOCKING
 * Blocking hardware.
 */
enum class HardwareType { DMA, IT, BLOCKING };


/**
 * @class HardwareInterface
 * @brief Abstract base class for all hardware interfaces.
 * This class provides the interface for hardware operations such as resetting, starting, and stopping the hardware.
 * ass well as other functionalities for example
 * like task that runa long to handle the trafice and what knot
 */
class HardwareInterface {
public:
  HardwareInterface() = default;

  virtual ~HardwareInterface(){};

  /**
   * @brief Restarts the hardware interface
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
};

} // namespace stmepic