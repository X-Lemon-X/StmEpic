#pragma once


#include "stmepic.hpp"
#include <string>

namespace stmepic
{

class Status{
public:
  Status(Status &status);

  /// @brief Construct a new Status object with ERROR status
  static Status ERROR(int status);

  /// @brief Construct a new Status object with ERROR status and message
  static Status ERROR(const std::string message);

  /// @brief Construct a new Status object with ERROR status and message
  static Status ERROR(int status,const std::string message);
  
  /// @brief Construct a new Status object with OK status
  static Status OK();

  /// @brief Construct a new Status object with ERROR status
  static Status ERROR();

  /// @brief check if the status is OK
  bool ok();

  /// @brief get the status
  /// @return 0 if OK or some error code
  int status();

  /// @brief get the message of the status
  const std::string to_string();
private:
  Status(int status, std::string message);
  int _status;
  std::string _message;
};

template <typename T> 
struct Result{
public: 
  
  Result(Status status): _status(status){};

  static Result<T> OK(T value){
    return Result<T>(value,Status::OK());
  }

  static Result<T> ERROR(Status status){
    return Result<T>(status);
  }
  
  T& valueOrDie(){return _value;}
  Status& status(){return _status;}
  bool ok(){return _status.ok();}
private:
  Result(T value, Status status): _value(value), _status(status){};  

  T _value;
  Status _status;
};

}