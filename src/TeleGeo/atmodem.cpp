
#include "stmepic.hpp"
#include "device.hpp"
#include "gpio.hpp"
#include "atmodem.hpp"
#include <string>
#include <string.h>
#include "logger.hpp"

using namespace stmepic::modems::internal;
using namespace stmepic::modems;
using namespace stmepic;


Result<std::shared_ptr<AtModem>> AtModem::Make(std::shared_ptr<UART> huart) {
  if(huart == nullptr)
    return Status::ExecutionError("UART is nullpointer");
  auto a = std::shared_ptr<AtModem>(new AtModem(huart));
  return Result<std::shared_ptr<AtModem>>::OK(a);
}

AtModem::AtModem(std::shared_ptr<UART> _huart)
: huart(_huart), _device_status(Status::Disconnected("not started")),
  nmea_status(Status::Invalid("Nmea not enabled")), settings(std::make_unique<AtModemSettings>()) {

  DeviceThreadedSettings ts;
  ts.uxStackDepth = 4024; // Default stack size for the task
  ts.uxPriority   = 2;    // Default priority for the task
  ts.period       = 50;   // Default period for the task in ms
  device_task_set_settings(ts);
}

Status AtModem::device_get_status() {
  return Status::OK();
}


Status AtModem::stop() {
  STMEPIC_RETURN_ON_ERROR(huart->hardware_stop());
  _device_status = Status::Disconnected("not started");
  return Status::OK();
}

Status AtModem::device_set_settings(const DeviceSettings &settings) {
  auto mayby_settings = dynamic_cast<const AtModemSettings *>(&settings);
  if(!mayby_settings) {
    return Status::ExecutionError("Settings are not of type AtModemSettings");
  }
  this->settings = std::make_unique<AtModemSettings>(*mayby_settings);
  return Status::OK();
}

Result<at_status_t> AtModem::send_command(const char *command, int expected_size) {
  uint8_t tx[32] = { 0 };
  uint16_t size  = strlen(command);
  memcpy(tx, command, size);
  tx[size]     = '\r'; // Add CR at the end
  tx[size + 1] = '\n'; // Add LF at the end
  size += 2;
  STMEPIC_RETURN_ON_ERROR(huart->write(tx, size, 100));
  if(expected_size == 0) {
    return Result<at_status_t>::OK(at_status_t::AT_OK);
  }
  if(expected_size < 0) {
    expected_size = size + 5; // +2 for CRLF and +3 for "OK\r\n"
  }
  uint8_t response_data[1024] = { 0 };
  STMEPIC_RETURN_ON_ERROR(huart->read(response_data, expected_size, 100));
  for(int i = size; i < expected_size - 1; ++i) {
    if(response_data[i] == 'O' && response_data[i + 1] == 'K') {
      return Result<at_status_t>::OK(at_status_t::AT_OK);
    }
  }
  return Result<at_status_t>::OK(at_status_t::AT_ERROR);
};


Status AtModem::init() {
  at_status_t result;

  STMEPIC_ASSING_TO_OR_RETURN(result, send_command("AT", -1));
  if(result != at_status_t::AT_OK) {
    _device_status =
    Status::ExecutionError("Are you sure that the modem is connected, or support AT commands?");
    return _device_status;
  }

  STMEPIC_ASSING_TO_OR_RETURN(result, send_command("AT+CFUN=1", -1));
  if(result != at_status_t::AT_OK) {
    _device_status = Status::ExecutionError("Failed to set modem to full functionality");
    return _device_status;
  }

  if(settings->enable_gps) {
    STMEPIC_ASSING_TO_OR_RETURN(result, send_command("AT+CGNSPWR=1"));
    if(result != at_status_t::AT_OK) {
      _device_status = Status::ExecutionError("Failed to enable GPS");
      return _device_status;
    }

    // STMEPIC_ASSING_TO_OR_RETURN(result, send_command("AT+CGNSINF", 0));
    // if(result != at_status_t::AT_OK) {
    //   _device_status = Status::ExecutionError("Failed to get GPS info");
    //   return _device_status;
    // }

    STMEPIC_ASSING_TO_OR_RETURN(result, send_command("AT+CGNSTST=1", 0));
    if(result != at_status_t::AT_OK) {
      _device_status = Status::ExecutionError("Failed to enable NMEA sentences");
      nmea_status    = Status::OK();
      return _device_status;
    }
  }

  if(settings->enable_gsm) {
    // TODO: Add GSM initialization
  }
  return Status::OK();
}

Status AtModem::do_device_task_reset() {
  // STMEPIC_RETURN_ON_ERROR(device_stop());
  STMEPIC_RETURN_ON_ERROR(send_command("AT+CFUN=1,1", -1));
  nmea_parser.reset();
  return Status::OK();
  // return device_start();
}

Status AtModem::do_device_task_start() {
  return DeviceThreadedBase::do_default_task_start(task, task_before, this);
}

Status AtModem::do_device_task_stop() {
  return DeviceThreadedBase::do_default_task_stop();
}

bool AtModem::device_ok() {
  return _device_status.ok();
}

Result<bool> AtModem::device_is_connected() {
  return Result<bool>::Propagate(_device_status.ok(), _device_status);
}


Status AtModem::task_before(SimpleTask &handler, void *arg) {
  (void)handler;
  AtModem *bar = static_cast<AtModem *>(arg);
  return bar->init();
}

Status AtModem::task(SimpleTask &handler, void *arg) {
  (void)handler;
  AtModem *modem = static_cast<AtModem *>(arg);
  return modem->handle();
}

Status AtModem::handle() {
  uint8_t data[120] = { 0 };
  auto a            = huart->read(data, sizeof(data), 3000);

  if(a == StatusCode::HalBusy) {
    huart->hardware_stop();
    huart->hardware_start();
  }

  log_info("AT Modem" + a.status().to_string() + " data received:" + std::string(reinterpret_cast<const char *>(data)));
  for(size_t i = 0; i < sizeof(data); ++i) {
    if(data[i] == '\0')
      continue; // Skip null characters
    if(settings->enable_gps) {
      nmea_status = nmea_parser.parse_by_character(static_cast<char>(data[i]));
    }
  }
  auto v = nmea_parser.get_gga_data();
  log_info("Long: " + std::to_string(v.latitude) + " Lat: " + std::to_string(v.longitude));
  return Status::OK();
}

Result<const gps::NmeaParser &> AtModem::get_nmea_data() {
  return Result<const gps::NmeaParser &>::Propagate(nmea_parser, nmea_status);
}
