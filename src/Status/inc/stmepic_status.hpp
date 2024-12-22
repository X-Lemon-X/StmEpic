#pragma once


#include "stmepic.hpp"
#include <string>

namespace stmepic
{
class Status;

#define STMEPIC_RETURN_ON_ERROR(x) \
  do{                              \
    Status _x = x.status();        \
    if(!_x.ok()) return _x;        \
  } while(false)

#define STMEPIC_ASSING_OR_RETURN(assign,result) \
    STMEPIC_RETURN_ON_ERROR(result);            \
    auto assign = result.valueOrDie();         

enum class StatusCode: char{
  OK = 0,
  OutOfMemory = 1,
  KeyError = 2,
  TypeError = 3,
  Invalid = 4,
  IOError = 5,
  CapacityError = 6,
  IndexError = 7,
  Cancelled = 8,
  UnknownError = 9,
  NotImplemented = 10,
  SerializationError = 11,
  RError = 13,
  // Gandiva range of errors
  CodeGenError = 40,
  ExpressionValidationError = 41,
  ExecutionError = 42,
  // Continue generic codes.
  AlreadyExists = 45,
  TimeOut = 46
};

class Status{
public:
  Status(Status &status) = default;

  [[nodiscard]] static Status OK() {return  Status(StatusCode::OK,nullptr);};
  
  [[nodiscard]] static Status OutOfMemory(const char *msg=nullptr) {return  Status(StatusCode::OutOfMemory,msg);};
  
  [[nodiscard]] static Status KeyError(const char *msg=nullptr) {return  Status(StatusCode::KeyError,msg);};
  
  [[nodiscard]] static Status TypeError(const char *msg=nullptr) {return  Status(StatusCode::TypeError,msg);};
  
  [[nodiscard]] static Status Invalid(const char *msg=nullptr) {return  Status(StatusCode::Invalid,msg);};
  
  [[nodiscard]] static Status IOError(const char *msg=nullptr) {return  Status(StatusCode::IOError,msg);};
  
  [[nodiscard]] static Status CapacityError(const char *msg=nullptr) {return  Status(StatusCode::CapacityError,msg);};
  
  [[nodiscard]] static Status IndexError(const char *msg=nullptr) {return  Status(StatusCode::IndexError,msg);};
  
  [[nodiscard]] static Status Cancelled(const char *msg=nullptr) {return  Status(StatusCode::Cancelled,msg);};
  
  [[nodiscard]] static Status UnknownError(const char *msg=nullptr) {return  Status(StatusCode::UnknownError,msg);};
  
  [[nodiscard]] static Status NotImplemented(const char *msg=nullptr) {return  Status(StatusCode::NotImplemented,msg);};
  
  [[nodiscard]] static Status SerializationError(const char *msg=nullptr) {return  Status(StatusCode::SerializationError,msg);};
  
  [[nodiscard]] static Status RError(const char *msg=nullptr) {return  Status(StatusCode::RError,msg);};
  
  [[nodiscard]] static Status CodeGenError(const char *msg=nullptr) {return  Status(StatusCode::CodeGenError,msg);};
  
  [[nodiscard]] static Status ExpressionValidationError(const char *msg=nullptr) {return  Status(StatusCode::ExpressionValidationError,msg);};
  
  [[nodiscard]] static Status ExecutionError(const char *msg=nullptr) {return  Status(StatusCode::ExecutionError,msg);};
  
  [[nodiscard]] static Status AlreadyExists(const char *msg=nullptr) {return  Status(StatusCode::AlreadyExists,msg);};

  [[nodiscard]] static Status TimeOut(const char *msg=nullptr) {return  Status(StatusCode::TimeOut,msg);};


  /// @brief get the status
  /// @return 0 if OK or some error code
  [[nodiscard]] StatusCode status_code()  {return _status;};

  /// @brief check if the status is OK
  bool ok() {return _status == StatusCode::OK;};

  /// @brief get status from status
  [[nodiscard]] Status& status() {return *this;};

  /// @brief get the message of the status
  [[nodiscard]] const std::string to_string() {
    if(_message != nullptr) return std::string(_message);
    else return "";
  };

private:
  Status(StatusCode status, const char *message) : _status(status), _message(message){};
  StatusCode _status;
  const char *_message;
};

template <typename T> 
struct Result{
public:   
  Result(Status status): _status(status){};
  static auto OK(T value) -> Result<T>{ return Result<T>(value,Status::OK());}
  
  [[nodiscard]] auto valueOrDie() -> T& {return _value;}
  [[nodiscard]] Status& status() {return _status;}
  [[nodiscard]] bool ok() {return _status.ok();}
private:
  Result(T value, Status status): _value(value), _status(status){};  
  T _value;
  Status _status;
};

}