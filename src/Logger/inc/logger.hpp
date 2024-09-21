
#ifndef LOGER_H
#define LOGER_H

#include <string>

#define BOOL_TO_STRING(b) (b ? "1" : "0")

namespace LOGGER {

enum class LOG_LEVEL {
  LOG_LEVEL_DEBUG=0,
  LOG_LEVEL_INFO=1,
  LOG_LEVEL_WARNING=2,
  LOG_LEVEL_ERROR=3
};



class Logger {
private:
  LOG_LEVEL log_level;
  bool print_info;
  std::string version;
  void transmit(std::string msg, std::string prefix);
public:
  /// @brief  initiate the logger
  /// @param level - log level
  /// @param print_info if set to false the loger will simply print log messages with END_OF_LINE sign at the end. 
  /// If set to true then the message will be encapsulated as if it were json format with the message as it fields,
  /// the logger will add aditional info like time stamp, software version, id of the board log_lvl.
  /// and putt user msg as a separet json field in the main json with name "msg" : { user_msg }
  Logger(LOG_LEVEL level, bool print_info, std::string _version="");

  void error(std::string msg);
  void warning(std::string msg);
  void info(std::string msg);
  void debug(std::string msg);

  /// @brief  parse the key value pair to json format
  /// @param key - key of the json field
  /// @param value - value of the json field
  /// @param add_coma - if set to true the function will add coma at the end of the json field
  /// @param as_list - if set to true the function will add the value as a json field with name "key" : { value }
  /// @return std::string - json field
  std::string parse_to_json_format(std::string key, std::string value,bool add_coma=true, bool as_list=false);
};

}
#endif // LOGER_H