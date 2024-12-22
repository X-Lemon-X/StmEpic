
#ifndef LOGER_H
#define LOGER_H

#include "stmepic_status.hpp"
#include <string>
#include <stmepic.hpp>

#define BOOL_TO_STRING(b) (b ? "1" : "0")

namespace stmepic {

enum class LOG_LEVEL {
  LOG_LEVEL_DEBUG=0,
  LOG_LEVEL_INFO=1,
  LOG_LEVEL_WARNING=2,
  LOG_LEVEL_ERROR=3
};

class JsonParse {
  public:
  
  // template<typename T>
  // [[nodiscard]] JsonParse& AddKeyValue(std::string key, T value){
  //   if constexpr (std::is_same<T, const char*>::value)
  //     return key_value_to_json(key,std::string(value));
  //   else
  //     return key_value_to_json(key,std::to_string(value));
  // };

  // [[nodiscard]] std::string get_json() const { return _json; }
  // private:
  // std::string _json="{";

  // key_value_to_json
};

class Logger {
public:
  using transmit_data_func = uint8_t (*)(uint8_t*,uint16_t);

  /// @brief  initiate the logger
  /// @param level - log level
  /// @param print_info if set to false the loger will simply print log messages with END_OF_LINE sign at the end. 
  /// If set to true then the message will be encapsulated as if it were json format with the message as it fields,
  /// the logger will add aditional info like time stamp, software version, id of the board log_lvl.
  /// and putt user msg as a separet json field in the main json with name "msg" : { user_msg }
  /// @param _transmi_function - function that will be used to transmit the data on some interface.
  Logger();

  Status init(LOG_LEVEL level, bool print_info,transmit_data_func _transmi_function, std::string _version="");

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
  template<typename T>
  static std::string parse_to_json_format(std::string key, T value,bool add_coma=true, bool as_list=false){
    std::string val;
    if constexpr (std::is_same<T, const char*>::value)
      val = std::string(value);
    else if constexpr (std::is_same<T, std::string>::value)
      val = value;
    else
      val = std::to_string(value);
    
    if(as_list) return "\""+key+"\": {"+val+"}"+(add_coma?",":"");
    else return "\""+key+"\":\""+val+"\""+(add_coma?",":"");
  }

  // static std::string parse_to_json_format(std::string key, std::string value,bool add_coma=true, bool as_list=false);


  /// @brief  global logger instance
  /// @return Logger& - logger instance
  static Logger& get_instance();

private:
  static Logger *logger_instance;
  LOG_LEVEL log_level;
  bool print_info;
  std::string version;
  void transmit(std::string msg, std::string prefix);
  static std::string key_value_to_json(std::string key, std::string value);
  transmit_data_func transmit_function;
};

}
#endif // LOGER_H