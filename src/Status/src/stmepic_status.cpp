#include "stmepic.hpp"

using namespace stmepic;

Status::Status(int status, std::string message): _status(status), _message(message){}

Status::Status(Status &status){
  this->_status = status._status;
  this->_message = status._message;
}

Status Status::OK(){
  return Status(0,"");
}

Status Status::ERROR(){
  return Status(1,"");
}

Status Status::ERROR(int status){
  return Status(status,"");
}

Status Status::ERROR(std::string message){
  return Status(1,message);
}

Status Status::ERROR(int status,std::string message){
  return Status(status,message);
}

bool Status::ok(){
  return _status == 0;
}

const std::string Status::to_string(){
  return _message;
}
