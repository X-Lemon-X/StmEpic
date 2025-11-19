#pragma once

#include "stmepic.hpp"
#include "device.hpp"
#include "gpio.hpp"
#include "vectors3d.hpp"
#include "uart.hpp"
#include <memory>
#include "nmea.hpp"

using namespace stmepic;
using namespace stmepic::algorithm;

/**
 * @file atmodem.hpp
 *  @brief atmodem device handler.
 */

/**
 * @defgroup Modems
 * @brief Functions related to different modems.
 * @{
 */

namespace stmepic::modems::internal {

enum class at_status_t { AT_OK, AT_ERROR };
}


namespace stmepic::modems {

struct AtModemSettings : public DeviceSettings {
  bool enable_gps = true; // Enable GPS by default
  bool enable_gsm = true; // Enable GSM by default
};

class AtModem : public stmepic::DeviceThreadedBase {
public:
  static Result<std::shared_ptr<AtModem>> Make(std::shared_ptr<UartBase> huart);

  Result<bool> device_is_connected() override;
  bool device_ok() override;
  Status device_get_status() override;
  Status device_set_settings(const DeviceSettings &settings) override;

  const gps::NmeaParser &get_nmea_data();


private:
  AtModem(std::shared_ptr<UartBase> huart);

  stmepic::Status do_device_task_start() override;
  stmepic::Status do_device_task_stop() override;
  Status do_device_task_reset() override;

  Status init();
  Status stop();

  static Status task_before(SimpleTask &handler, void *arg);
  static Status task(SimpleTask &handler, void *arg);
  Status handle();

  Result<internal::at_status_t> send_command(const char *command, int expected_size = -1);


  std::unique_ptr<AtModemSettings> settings;
  gps::NmeaParser nmea_parser;
  Status nmea_status;

  std::shared_ptr<UartBase> huart;
  Status _device_status;
};

} // namespace stmepic::modems