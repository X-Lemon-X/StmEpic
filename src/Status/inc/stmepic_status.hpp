#pragma once


#include "stmepic.hpp"
#include <string>

namespace stmepic
{

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
  AlreadyExists = 45
};

class Status{
public:
  Status(Status &status) : _status(status._status), _message(status._message){};

  [[nodiscard]] static auto OK() -> Status {return  Status(StatusCode::OK,nullptr);};
  [[nodiscard]] static auto OutOfMemory(const char *msg=nullptr) -> Status {return  Status(StatusCode::OutOfMemory,msg);};
  [[nodiscard]] static auto KeyError(const char *msg=nullptr) -> Status {return  Status(StatusCode::KeyError,msg);};
  [[nodiscard]] static auto TypeError(const char *msg=nullptr) -> Status {return  Status(StatusCode::TypeError,msg);};
  [[nodiscard]] static auto Invalid(const char *msg=nullptr) -> Status {return  Status(StatusCode::Invalid,msg);};
  [[nodiscard]] static auto IOError(const char *msg=nullptr) -> Status {return  Status(StatusCode::IOError,msg);};
  [[nodiscard]] static auto CapacityError(const char *msg=nullptr) -> Status {return  Status(StatusCode::CapacityError,msg);};
  [[nodiscard]] static auto IndexError(const char *msg=nullptr) -> Status {return  Status(StatusCode::IndexError,msg);};
  [[nodiscard]] static auto Cancelled(const char *msg=nullptr) -> Status {return  Status(StatusCode::Cancelled,msg);};
  [[nodiscard]] static auto UnknownError(const char *msg=nullptr) -> Status {return  Status(StatusCode::UnknownError,msg);};
  [[nodiscard]] static auto NotImplemented(const char *msg=nullptr) -> Status {return  Status(StatusCode::NotImplemented,msg);};
  [[nodiscard]] static auto SerializationError(const char *msg=nullptr) -> Status {return  Status(StatusCode::SerializationError,msg);};
  [[nodiscard]] static auto RError(const char *msg=nullptr) -> Status {return  Status(StatusCode::RError,msg);};
  [[nodiscard]] static auto CodeGenError(const char *msg=nullptr) -> Status {return  Status(StatusCode::CodeGenError,msg);};
  [[nodiscard]] static auto ExpressionValidationError(const char *msg=nullptr) -> Status {return  Status(StatusCode::ExpressionValidationError,msg);};
  [[nodiscard]] static auto ExecutionError(const char *msg=nullptr) -> Status {return  Status(StatusCode::ExecutionError,msg);};
  [[nodiscard]] static auto AlreadyExists(const char *msg=nullptr) -> Status {return  Status(StatusCode::AlreadyExists,msg);};



  /// @brief check if the status is OK
  bool ok() {return _status == StatusCode::OK;};

  /// @brief get the status
  /// @return 0 if OK or some error code
  [[nodiscard]] auto status() -> StatusCode {return _status;};

  /// @brief get the message of the status
  [[nodiscard]] const std::string to_string() {
    if(_message != nullptr)
      return std::string(_message);
    else
      return std::string("");
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
  // static auto ERROR(Status &status) -> Result<T> { return Result<T>(status);}
  
  auto valueOrDie() -> T& {return _value;}
  auto status() -> Status& {return _status;}
  auto ok() -> bool {return _status.ok();}

private:
  Result(T value, Status status): _value(value), _status(status){};  
  T _value;
  Status _status;
};

}