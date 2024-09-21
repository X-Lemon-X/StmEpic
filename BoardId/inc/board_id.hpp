#include "pin.hpp"
#include "stm32f4xx_hal.h"

#ifndef BOARD_ID_HPP
#define BOARD_ID_HPP


/// @brief if BOARD_ID_OWERWRITE is defined, then BOARD_ID_OWERWRITE_ID will be used as board id  
// #define BOARD_ID_OWERWRITE
// #define BOARD_ID_OWERWRITE_ID 2




namespace BOARD_ID{




class Board_id{
private:
  const GpioPin &id_0;
  const GpioPin &id_1;
  const GpioPin &id_2;
  uint8_t id;
  bool id_set;
public:
  Board_id(const GpioPin &id_0,const GpioPin &id_1,const GpioPin &id_2);
  uint8_t get_id();
};
  
} // namespace BOARD_ID


#endif