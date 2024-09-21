#ifndef __CIRCUALR_BUFFOR_HPP__
#define __CIRCUALR_BUFFOR_HPP__

#include <cstdint>


namespace CIRCULAR_BUFFOR{

enum class circular_buffor_status: uint8_t{
  CIRCULAR_BUFFOR_OK = 0,
  CIRCULAR_BUFFOR_FULL = 1,
  CIRCULAR_BUFFOR_EMPTY = 2,
  CIRCULAR_BUFFOR_ERROR = 3
};

/// @brief a CIrcular buffor class that stores copies of data in a static circualar array
/// @tparam circualr_buffor_data_type 
/// @tparam buffor_size 
template <typename circualr_buffor_data_type, uint32_t buffor_size>
class static_circular_buffor{
private:
  circualr_buffor_data_type buffor[buffor_size];
  uint32_t head;
  uint32_t tail;
  uint32_t size;
public:
  static_assert(buffor_size>0, "Buffor size must be greater than 0");

  static_circular_buffor(): head(0), tail(0), size(0){};
  ~static_circular_buffor(){};

  /// @brief push_back the data to the last element of the buffor
  /// @param data data to be pushed back
  /// @return uint8_t CIRCULAR_BUFFOR_OK if success
  uint8_t push_back(circualr_buffor_data_type data){
    if (head == tail && size == buffor_size){
      return (uint8_t)circular_buffor_status::CIRCULAR_BUFFOR_FULL;
    }          
    size++;
    buffor[tail] = data; 
    tail++;
    if(tail == buffor_size)
      tail = 0;
    return (uint8_t)circular_buffor_status::CIRCULAR_BUFFOR_OK;
  };

  /// @brief get the copy of the front element of the buffor 
  /// @return CIRCULAR_BUFFOR_OK if success, CIRCULAR_BUFFOR_EMPTY if buffor is empty
  uint8_t get_front(circualr_buffor_data_type* data){
    if(head == tail && size == 0)
      return (uint8_t)circular_buffor_status::CIRCULAR_BUFFOR_EMPTY;
    if(data == nullptr)
      return (uint8_t)circular_buffor_status::CIRCULAR_BUFFOR_ERROR;
    *data = buffor[head];
    return (uint8_t)circular_buffor_status::CIRCULAR_BUFFOR_OK;
  };

  uint8_t pop_front(){
    if(head == tail && size == 0)
      return (uint8_t)circular_buffor_status::CIRCULAR_BUFFOR_EMPTY;
    size--;
    head++;
    if(head == buffor_size)
      head = 0;
    return (uint8_t)circular_buffor_status::CIRCULAR_BUFFOR_OK;
  };

  /// @brief get the size of the buffor
  /// @return uint32_t size of the buffor
  uint32_t get_size() const{
    return size;
  };

};
  
}

#endif // __CIRCUALR_BUFFOR_HPP__