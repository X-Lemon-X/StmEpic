#include "logger.hpp"
#include <string>
// #include "usbd_cdc_if.h"
#include "stmepic.hpp"


using namespace stmepic;

Logger::Logger() {
  log_level         = LOG_LEVEL::LOG_LEVEL_WARNING;
  transmit_function = nullptr;
  print_info        = false;
  version           = "";
}

Status Logger::init(LOG_LEVEL level, bool _print_info, transmit_data_func _transmi_function, std::string _version) {
  if(_transmi_function == nullptr)
    return Status::ExecutionError("Transmit function is nullptr");
  log_level         = level;
  transmit_function = _transmi_function;
  print_info        = _print_info;
  version           = _version;
  return Status::OK();
}

void Logger::error(std::string msg) {
  if(log_level > LOG_LEVEL::LOG_LEVEL_ERROR)
    return;
  transmit(msg, "ERROR");
}

void Logger::warning(std::string msg) {
  if(log_level > LOG_LEVEL::LOG_LEVEL_WARNING)
    return;
  transmit(msg, "WARNING");
}

void Logger::info(std::string msg) {
  if(log_level > LOG_LEVEL::LOG_LEVEL_INFO)
    return;
  transmit(msg, "INFO");
}

void Logger::debug(std::string msg) {
  if(log_level > LOG_LEVEL::LOG_LEVEL_DEBUG)
    return;
  transmit(msg, "DEBUG");
}

void Logger::transmit(std::string msg, std::string prefix) {
  if(print_info) {
    msg = "{\"time\":\"" + std::to_string(HAL_GetTick()) + "\",\"level\":\"" + prefix +
          "\",\"ver\":\"" + version + "\",\"msg\":{" + msg + "}}\n";
  } else {
    msg += "\n";
  }
  if(transmit_function)
    transmit_function((uint8_t*)msg.c_str(), msg.length());
}

std::string Logger::key_value_to_json(std::string key, std::string value) {
  return "\"" + key + "\":\"" + value + "\"";
}

Logger& Logger::get_instance() {
  static Logger* logger_instance = nullptr;
  if(logger_instance == nullptr)
    logger_instance = new Logger();
  return *logger_instance;
}