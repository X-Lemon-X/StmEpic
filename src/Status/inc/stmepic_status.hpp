#pragma once


#include <string>


/**
 * @file stmepic_status.hpp
 * @brief  Status class definition.
 */

/**
 * @defgroup Status
 * @brief Return types and function result handling.
 * @{
 */

namespace stmepic {


class Status;

/**
 * @brief Macro for returning on error in a single line.
 *
 * This macro is used to return from a function if the result is not OK.
 * usefull to avoid having to check if the status is OK and return the status if not.
 */
#define STMEPIC_RETURN_ON_ERROR(x) \
  do {                             \
    Status _x = x.status ();       \
    if (!_x.ok ())                 \
      return _x;                   \
  } while (false)

/**
 * @brief  Macro for assigning a value from a result and returning on error in a single line.
 *
 * This macro is used to assign a value from a result and return from a function if the result is not OK.
 */
#define STMEPIC_ASSING_OR_RETURN(assign, result) \
  STMEPIC_RETURN_ON_ERROR (result);              \
  auto assign = result.valueOrDie ();

/**
 * @enum StatusCode
 * @brief Enumeration representing various statuses. Inspired by ApacheArrow.
 *
 * @var StatusCode::OK
 * Operation was successful.
 *
 * @var StatusCode::OutOfMemory
 * Operation failed due to lack of memory.
 *
 * @var StatusCode::KeyError
 * Operation failed due to a key error.
 *
 * @var StatusCode::TypeError
 * Operation failed due to a type error.
 *
 * @var StatusCode::Invalid
 * Operation failed due to an invalid operation.
 *
 * @var StatusCode::IOError
 * Operation failed due to an I/O error.
 *
 * @var StatusCode::CapacityError
 * Operation failed due to a capacity error.
 *
 * @var StatusCode::IndexError
 * Operation failed due to an index error.
 *
 * @var StatusCode::Cancelled
 * Operation was cancelled.
 *
 * @var StatusCode::UnknownError
 * Operation failed due to an unknown error.
 *
 * @var StatusCode::NotImplemented
 * Operation is not implemented.
 *
 * @var StatusCode::SerializationError
 * Operation failed due to a serialization error.
 *
 * @var StatusCode::RError
 * Operation failed due to an R error.
 *
 * @var StatusCode::CodeGenError
 * Operation failed due to a code generation error.
 *
 * @var StatusCode::ExpressionValidationError
 * Operation failed due to an expression validation error.
 *
 * @var StatusCode::ExecutionError
 * Operation failed due to an execution error.
 *
 * @var StatusCode::AlreadyExists
 * Operation failed because the object already exists.
 *
 * @var StatusCode::TimeOut
 * Operation failed due to a timeout.
 */
enum class StatusCode : char {
  OK                 = 0,
  OutOfMemory        = 1,
  KeyError           = 2,
  TypeError          = 3,
  Invalid            = 4,
  IOError            = 5,
  CapacityError      = 6,
  IndexError         = 7,
  Cancelled          = 8,
  UnknownError       = 9,
  NotImplemented     = 10,
  SerializationError = 11,
  RError             = 13,
  // Gandiva range of errors
  CodeGenError              = 40,
  ExpressionValidationError = 41,
  ExecutionError            = 42,
  // Continue generic codes.
  AlreadyExists = 45,
  TimeOut       = 46
};

/**
 * @brief Status class used as return type.
 *
 * This class is used as a return type for functions that can fail. It contains a status code and a message.
 * For easy use, there are static methods for creating common statuses with some messages.
 * The Status can be converted to a Result object that contains the status and the value.
 */
class Status {
  public:
  Status (Status& status) = default;

  [[nodiscard]] static Status OK () {
    return Status (StatusCode::OK, nullptr);
  };

  [[nodiscard]] static Status OutOfMemory (const char* msg = nullptr) {
    return Status (StatusCode::OutOfMemory, msg);
  };

  [[nodiscard]] static Status KeyError (const char* msg = nullptr) {
    return Status (StatusCode::KeyError, msg);
  };

  [[nodiscard]] static Status TypeError (const char* msg = nullptr) {
    return Status (StatusCode::TypeError, msg);
  };

  [[nodiscard]] static Status Invalid (const char* msg = nullptr) {
    return Status (StatusCode::Invalid, msg);
  };

  [[nodiscard]] static Status IOError (const char* msg = nullptr) {
    return Status (StatusCode::IOError, msg);
  };

  [[nodiscard]] static Status CapacityError (const char* msg = nullptr) {
    return Status (StatusCode::CapacityError, msg);
  };

  [[nodiscard]] static Status IndexError (const char* msg = nullptr) {
    return Status (StatusCode::IndexError, msg);
  };

  [[nodiscard]] static Status Cancelled (const char* msg = nullptr) {
    return Status (StatusCode::Cancelled, msg);
  };

  [[nodiscard]] static Status UnknownError (const char* msg = nullptr) {
    return Status (StatusCode::UnknownError, msg);
  };

  [[nodiscard]] static Status NotImplemented (const char* msg = nullptr) {
    return Status (StatusCode::NotImplemented, msg);
  };

  [[nodiscard]] static Status SerializationError (const char* msg = nullptr) {
    return Status (StatusCode::SerializationError, msg);
  };

  [[nodiscard]] static Status RError (const char* msg = nullptr) {
    return Status (StatusCode::RError, msg);
  };

  [[nodiscard]] static Status CodeGenError (const char* msg = nullptr) {
    return Status (StatusCode::CodeGenError, msg);
  };

  [[nodiscard]] static Status ExpressionValidationError (const char* msg = nullptr) {
    return Status (StatusCode::ExpressionValidationError, msg);
  };

  [[nodiscard]] static Status ExecutionError (const char* msg = nullptr) {
    return Status (StatusCode::ExecutionError, msg);
  };

  [[nodiscard]] static Status AlreadyExists (const char* msg = nullptr) {
    return Status (StatusCode::AlreadyExists, msg);
  };

  [[nodiscard]] static Status TimeOut (const char* msg = nullptr) {
    return Status (StatusCode::TimeOut, msg);
  };


  /// @brief get the status
  /// @return 0 if OK or some error code
  [[nodiscard]] StatusCode status_code () {
    return _status;
  };

  /// @brief check if the status is OK
  bool ok () {
    return _status == StatusCode::OK;
  };

  /// @brief get status from status
  [[nodiscard]] Status& status () {
    return *this;
  };

  /// @brief get the message of the status
  [[nodiscard]] const std::string to_string () {
    if (_message != nullptr)
      return std::string (_message);
    else
      return "";
  };

  private:
  Status (StatusCode status, const char* message)
  : _status (status), _message (message){};
  StatusCode _status;
  const char* _message;
};

/**
 * @brief Result class used as return type.
 *
 * This class is used as a return type for functions that can fail. It contains a status and a value.
 * The Result can be converted to a Status object that contains the status and the message.
 *
 */
template <typename T> struct Result {
  public:
  /// @brief Create a new Result from status for clean return from functions when error occurs.
  Result (Status status) : _status (status){};

  /// @brief Return a new Result with value and status OK.
  static auto OK (T value) -> Result<T> {
    return Result<T> (value, Status::OK ());
  }

  /// @brief Get the value of the result or weard error if the status is not OK.
  /// You should check if the status is ok before calling this function.
  [[nodiscard]] auto valueOrDie () -> T& {
    return _value;
  }

  /// @brief Get the status of the result.
  [[nodiscard]] Status& status () {
    return _status;
  }

  /// @brief Check if the status is OK.
  [[nodiscard]] bool ok () {
    return _status.ok ();
  }

  private:
  Result (T value, Status status) : _value (value), _status (status){};
  T _value;
  Status _status;
};

} // namespace stmepic