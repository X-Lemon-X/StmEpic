#include "stmepic.hpp"
#include "logger.hpp"
#include <string>
// #include "usbd_cdc_if.h"


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

void Logger::error(std::string msg, const char* file, const char* function_name) {
  if(log_level > LOG_LEVEL::LOG_LEVEL_ERROR)
    return;
  transmit(msg, "ERROR", file, function_name);
}

void Logger::warning(std::string msg, const char* file, const char* function_name) {
  if(log_level > LOG_LEVEL::LOG_LEVEL_WARNING)
    return;
  transmit(msg, "WARNING", file, function_name);
}

void Logger::info(std::string msg, const char* file, const char* function_name) {
  if(log_level > LOG_LEVEL::LOG_LEVEL_INFO)
    return;
  transmit(msg, "INFO", file, function_name);
}

void Logger::debug(std::string msg, const char* file, const char* function_name) {
  if(log_level > LOG_LEVEL::LOG_LEVEL_DEBUG)
    return;
  transmit(msg, "DEBUG", file, function_name);
}

void Logger::transmit(std::string msg, std::string prefix, const char* file, const char* function_name) {
  if(print_info) {
    std::string debug_info = "";
    if(file != nullptr && function_name != nullptr)
      debug_info = "," + key_value_to_json("file", file) + "," +
                   key_value_to_json("function", function_name);
    msg = "{\"time\":\"" + std::to_string(HAL_GetTick()) + "\",\"level\":\"" + prefix +
          "\",\"ver\":\"" + version + "\"" + debug_info + ",\"msg\":{" + msg + "}}\n";
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