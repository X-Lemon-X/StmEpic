#pragma once

#include <string>
// #include "usb_device.h"
// #include "usbd_cdc_if.h"
// #include "usbd_def.h"
#include "logger.hpp"
#include "gpio.hpp"


#define USB_PROGRAMER_REBOOT "SB_reboot\n"
#define USB_PROGRAMER_PROGRAM "SB_enterdfu\n"
#define USB_PROGRAMER_INFO "SB_info\n"

#ifndef APP_RX_DATA_SIZE
#define APP_RX_DATA_SIZE 512
#endif
#define USB_PROGRAMER_BUFFER_SIZE APP_RX_DATA_SIZE

namespace stmepic::dfu {


/**
 * @brief Class for handling the USB programing of the stm32 uC using DFU mode over USB
 *
 */
class UsbProgramer {
private:
  GpioPin &boot_device;
  uint8_t buffer[USB_PROGRAMER_BUFFER_SIZE];
  std::string usb_programer_info;

public:
  UsbProgramer(GpioPin &boot_device);

  /// @brief should be called in the main loop to handle the usb programing
  void handler();

  /// @brief resets the stm32 uC
  void reset_device();

  /// @brief returns the info string
  void set_info(std::string info);

  /// @brief restart stm32 device and enters DFU  mode for USB programing
  void enter_dfu_mode();
};

} // namespace stmepic::dfu
