#ifndef __CIRCUALR_BUFFOR_HPP__
#define __CIRCUALR_BUFFOR_HPP__

#include "stmepic_status.hpp"
#include <cstdint>


namespace stmepic {

/// @brief a CIrcular buffor class that stores copies of data in a static circualar array
/// @tparam circualr_buffor_data_type
/// @tparam buffor_size
template <typename circualr_buffor_data_type, uint32_t buffor_size>
class static_circular_buffor {
  private:
  circualr_buffor_data_type buffor[buffor_size];
  uint32_t head;
  uint32_t tail;
  uint32_t size;

  public:
  static_assert(buffor_size > 0, "Buffor size must be greater than 0");

  static_circular_buffor() : head(0), tail(0), size(0){};
  ~static_circular_buffor(){};

  /// @brief push_back the data to the last element of the buffor
  /// @param data data to be pushed back
  /// @return uint8_t CIRCULAR_BUFFOR_OK if success
  Status push_back(circualr_buffor_data_type data) {
    if(head == tail && size == buffor_size) {
      return Status::CapacityError("Buffer is full");
    }
    size++;
    buffor[tail] = data;
    tail++;
    if(tail == buffor_size)
      tail = 0;
    return Status::OK();
  };

  /// @brief get the copy of the front element of the buffor
  /// @return CIRCULAR_BUFFOR_OK if success, CIRCULAR_BUFFOR_EMPTY if buffor is empty
  Result<circualr_buffor_data_type> get_front() {
    if(head == tail && size == 0)
      return Status::CapacityError("Buffer is empty");
    // if(data == nullptr)
    //   return Status::ERROR((int)Status_circular_buffor::CIRCULAR_BUFFOR_ERROR);
    // *data = buffor[head];
    return Result<circualr_buffor_data_type>::OK(buffor[head]);
  };

  Status pop_front() {
    if(head == tail && size == 0)
      return Status::CapacityError("Buffer is empty");
    size--;
    head++;
    if(head == buffor_size)
      head = 0;
    return Status::OK();
  };

  /// @brief get the size of the buffor
  /// @return uint32_t size of the buffor
  uint32_t get_size() const {
    return size;
  };
};

} // namespace stmepic

#endif // __CIRCUALR_BUFFOR_HPP__