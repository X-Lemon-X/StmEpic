#pragma once

#include "status.hpp"
#include <stmepic.hpp>
#include <string>

#define BOOL_TO_STRING(b) (b ? "1" : "0")

/**
 * @file logger.hpp
 * @brief Interface to log messages.
 *
 */

/**
 * @defgroup Logger
 * @brief Logger module for logging messages.
 * Logger module for logging messages with log levels like DEBUG, INFO, WARNING, ERROR.
 * Logger is used fruaut the libar using global logger instance that usually have to be
 * configured. To use somem inetrface to log data
 * @{
 */


namespace stmepic {

// @brief log_debug macro for logging debug messages with file and function name for debug purposes.
#define log_debug(...) stmepic::Logger::get_instance().debug(__VA_ARGS__, __FILE__, __func__)

// @brief log_info macro for logging info messages with file and function name for debug purposes.
#define log_info(...) stmepic::Logger::get_instance().info(__VA_ARGS__, __FILE__, __func__)

// @brief log_warn macro for logging warning messages with file and function name for debug purposes.
#define log_warn(...) stmepic::Logger::get_instance().warning(__VA_ARGS__, __FILE__, __func__)

// @brief log_error macro for logging error messages with file and function name for debug purposes.
#define log_error(...) stmepic::Logger::get_instance().error(__VA_ARGS__, __FILE__, __func__)

/**
 * @enum LOG_LEVEL
 * @brief Enumeration representing various log levels.
 *
 * @var LOG_LEVEL::LOG_LEVEL_DEBUG
 * Debug log level.
 *
 * @var LOG_LEVEL::LOG_LEVEL_INFO
 * Info log level.
 *
 * @var LOG_LEVEL::LOG_LEVEL_WARNING
 * Warning log level.
 *
 * @var LOG_LEVEL::LOG_LEVEL_ERROR
 * Error log level.
 */
enum class LOG_LEVEL { LOG_LEVEL_DEBUG = 0, LOG_LEVEL_INFO = 1, LOG_LEVEL_WARNING = 2, LOG_LEVEL_ERROR = 3 };


/**
 * @class Logger
 * @brief Class representing the logger.
 *
 * This class provides functionalities for logging messages with log levels like DEBUG, INFO, WARNING, ERROR.
 * It provides global instance of the logger that can be used to log messages.
 * however multiple instances can be created if needed.
 * The logger print usefull data like time stamp, log level, software version
 * However the user can configure the logger to print only the message provided by the user.
 */

// __FILE_NAME__
// __func__
class Logger {
public:
  using transmit_data_func = uint8_t (*)(uint8_t *, uint16_t);

  /// @brief Constructor for the Logger class, However it won't do much until the init function is called.
  Logger();

  /// @brief  Initiate the logger with transmit daat interface and log level.
  /// @param level - log level like DEBUG, INFO, WARNING, ERROR
  /// @param print_info if set to false the loger will simply print log messages with END_OF_LINE sign at the end.
  ///
  /// If set to true then the message will be printed in json format with fields like: the message,
  /// time stamp, function name, file name, software version, log level.
  /// @param transmi_function - function pointer to the function that will be used to transmit the data for example if we wont to use UART or USB to print data
  /// @param use_semihosting - if set to true the logger will use Semihosting to print data do the debugger console , No extra hadware is needed only the debugger.
  /// if used with OpenOCD you have to add argument to enable it: "init; arm semihosting enable"
  /// You can't use Semihosting on when using st-link-gdb server only OpenOCD.
  Status
  init(LOG_LEVEL level, bool print_info, transmit_data_func transmi_function, bool use_semihosting = false, std::string _version = "");

  /// @brief  log the ERROR message
  void error(std::string msg, const char *file = nullptr, const char *function_name = nullptr);

  /// @brief  log the WARNING message
  void warning(std::string msg, const char *file = nullptr, const char *function_name = nullptr);

  /// @brief  log the INFO message
  void info(std::string msg, const char *file = nullptr, const char *function_name = nullptr);

  /// @brief  log the DEBUG message
  void debug(std::string msg, const char *file = nullptr, const char *function_name = nullptr);

  /// @brief  parse the key value pair to json format
  /// @param key - key of the json field
  /// @param value - value of the json field
  /// @param add_coma - if set to true the function will add coma at the end of the json field
  /// @param as_list - if set to true the function will add the value as a json field with name "key" : { value }
  /// @return std::string - json field
  template <typename T>
  static std::string parse_to_json_format(std::string key, T value, bool add_coma = true, bool as_list = false) {
    std::string val;
    if constexpr(std::is_same<T, const char *>::value)
      val = std::string(value);
    else if constexpr(std::is_same<T, std::string>::value)
      val = value;
    else
      val = std::to_string(value);

    if(as_list)
      return "\"" + key + "\": {" + val + "}" + (add_coma ? "," : "");
    else
      return "\"" + key + "\":\"" + val + "\"" + (add_coma ? "," : "");
  }

  // static std::string parse_to_json_format(std::string key, std::string value,bool add_coma=true, bool as_list=false);


  /// @brief  Get global logger instance
  /// If not initiated the logger won't log anything
  /// @return Logger& - logger instance
  static Logger &get_instance();

private:
  static Logger *logger_instance;
  LOG_LEVEL log_level;
  bool print_info;
  std::string version;
  void transmit(std::string msg, std::string prefix, const char *file = nullptr, const char *function_name = nullptr);
  static std::string key_value_to_json(std::string key, std::string value);
  transmit_data_func transmit_function;
  bool use_semihosting;
};

} // namespace stmepic