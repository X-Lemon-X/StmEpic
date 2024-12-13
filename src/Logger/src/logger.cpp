#include "logger.hpp"
#include <string>
// #include "usbd_cdc_if.h"
#include "stmepic.hpp"


using namespace stmepic;

Logger::Logger(LOG_LEVEL level, bool _print_info,transmit_data_func _transmi_function,  std::string _version): 
  log_level(level),
  transmit_function(_transmi_function),
  print_info(_print_info),
  version(_version) {

}

void Logger::error(std::string msg){
  if (log_level > LOG_LEVEL::LOG_LEVEL_ERROR) return;
  transmit(msg,"ERROR");
}

void Logger::warning(std::string msg){
  if (log_level > LOG_LEVEL::LOG_LEVEL_WARNING ) return;
  transmit(msg,"WARNING");
}

void Logger::info(std::string msg){
  if (log_level > LOG_LEVEL::LOG_LEVEL_INFO) return;
  transmit(msg,"INFO");
}

void Logger::debug(std::string msg){
  if (log_level > LOG_LEVEL::LOG_LEVEL_DEBUG) return;
  transmit(msg,"DEBUG");
}


void Logger::transmit(std::string msg,std::string prefix){
  if(print_info){
    msg = 
        "{\"time\":\""+std::to_string(HAL_GetTick())+ 
        "\",\"level\":\""+prefix+
        "\",\"ver\":\"" + version+
        "\",\"msg\":{" + msg + "}}\n";
  }else{
    msg += "\n";
  }
  if(transmit_function) transmit_function((uint8_t*)msg.c_str(), msg.length());
}

std::string Logger::parse_to_json_format(std::string key, std::string value,bool add_coma,bool as_list){
  if(as_list) return "\""+key+"\": {"+value+"}"+(add_coma?",":"");
  else return "\""+key+"\":\""+value+"\""+(add_coma?",":"");
}

std::string Logger::key_value_to_json(std::string key, std::string value){
  return "\""+key+"\":\""+value+"\"";
}