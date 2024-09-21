#include "board_id.hpp"

using namespace BOARD_ID;

Board_id::Board_id(const GpioPin &_id_0,const GpioPin &_id_1,const GpioPin &_id_2): id_0(_id_0), id_1(_id_1), id_2(_id_2){
  this->id_set = false;
}

uint8_t Board_id::get_id(){

  #ifdef BOARD_ID_OWERWRITE
    return BOARD_ID_OWERWRITE_ID;
  #endif

  #ifndef BOARD_ID_OWERWRITE
  if (id_set) return id;
  id = 0;
  id |= READ_GPIO(id_0);
  id |= READ_GPIO(id_1) << 1;
  id |= READ_GPIO(id_2) << 2;
  id_set = true;
  return id;
  #endif
}