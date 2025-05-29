
#include "stmepic.hpp"
#include "device.hpp"
#include "gpio.hpp"
#include "atmodem.hpp"
#include <string>
#include <string.h>

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
}

Status AtModem::device_get_status() {
}


Status AtModem::device_stop() {
  STMEPIC_RETURN_ON_ERROR(huart->hardware_stop());
  _device_status = Status::Disconnected("not started");
  return Status::OK();
}

Status AtModem::device_set_settings(const DeviceSettings &settings) {
  auto mayby_settings = static_cast<const AtModemSettings *>(&settings);
  if(mayby_settings == nullptr) {
    return Status::ExecutionError("Settings are not of type AtModemSettings");
  }
}

Result<at_status_t> AtModem::send_command(std::string command, std::string &response) {
  if(command.empty()) {
    return Status::Invalid("Command is empty");
  }
  command += "\r\n"; // AT commands end with CRLF
  STMEPIC_RETURN_ON_ERROR(huart->write((uint8_t *)command.c_str(), command.size(), 1000));
  uint8_t data[1024] = { 0 };
  STMEPIC_RETURN_ON_ERROR(huart->read(data, sizeof(data), 1000));
  response = std::string(reinterpret_cast<char *>(data));
  if(response.find("OK") == std::string::npos) {
    return Result<at_status_t>::OK(at_status_t::AT_ERROR);
  }
  return Result<at_status_t>::OK(at_status_t::AT_OK);
};


Status AtModem::device_start() {
  huart->hardware_start();
  std::string response;
  at_status_t result;

  STMEPIC_ASSING_TO_OR_RETURN(result, send_command("AT", response));
  if(result != at_status_t::AT_OK) {
    _device_status =
    Status::ExecutionError("Are you sure that the modem is connected, or support AT commands?");
    return _device_status;
  }

  STMEPIC_ASSING_TO_OR_RETURN(result, send_command("AT+CFUN=1", response));
  if(result != at_status_t::AT_OK) {
    _device_status = Status::ExecutionError("Failed to set modem to full functionality");
    return _device_status;
  }

  if(settings->enable_gps) {
    STMEPIC_ASSING_TO_OR_RETURN(result, send_command("AT+CGNSPWR=1", response));
    if(result != at_status_t::AT_OK) {
      _device_status = Status::ExecutionError("Failed to enable GPS");
      return _device_status;
    }

    STMEPIC_ASSING_TO_OR_RETURN(result, send_command("AT+CGNSINF", response));
    if(result != at_status_t::AT_OK) {
      _device_status = Status::ExecutionError("Failed to get GPS info");
      return _device_status;
    }

    STMEPIC_ASSING_TO_OR_RETURN(result, send_command("AT+CGNSTST=1", response));
    if(result != at_status_t::AT_OK) {
      _device_status = Status::ExecutionError("Failed to enable NMEA sentences");
      return _device_status;
    }
  }

  if(settings->enable_gsm) {
    // TODO: Add GSM initialization commands if needed
  }
  return Status::OK();
}

Status AtModem::device_reset() {
  STMEPIC_RETURN_ON_ERROR(device_stop());
  STMEPIC_RETURN_ON_ERROR(huart->hardware_reset());
  _device_status = Status::Disconnected("not started");
  nmea_parser.reset();
  return device_start();
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


void AtModem::task_before(SimpleTask &handler, void *arg) {
  (void)handler;
  AtModem *bar = static_cast<AtModem *>(arg);
  (void)bar->device_start();
}

void AtModem::task(SimpleTask &handler, void *arg) {
  (void)handler;
  AtModem *modem = static_cast<AtModem *>(arg);
  modem->handle();
}

void AtModem::handle() {
  uint8_t data[1024];
  huart->read(data, sizeof(data), 1000);


  for(size_t i = 0; i < sizeof(data); ++i) {
    if(data[i] == '\0')
      continue; // Skip null characters
    if(settings->enable_gps) {
      nmea_status = nmea_parser.parse_by_character(static_cast<char>(data[i]));
    }
  }
}

Result<const gps::NmeaParser &> AtModem::get_nmea_data() {
  return Result<const gps::NmeaParser &>::Propagate(nmea_parser, nmea_status);
}
