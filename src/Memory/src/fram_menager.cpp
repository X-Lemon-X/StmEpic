#include "fram_menager.hpp"

using namespace FRAMM;

FRAMfile::FRAMfile(){
  data = nullptr;
  data_size = 0;
  name[0] = '\0';
  fram_address = 0;
  fram_size = 0;
}

FRAMfile::~FRAMfile(){
  if(data != nullptr){
    free(data);
  }
}

FRAMMenager::FRAMMenager(){
  init_flag = false;
  i2c = nullptr;
  address = 0;
  fram_size = 0;
}

FRAMMenager::~FRAMMenager(){
}

void FRAMMenager::init(I2C_HandleTypeDef &i2c, uint16_t address, uint32_t fram_size){
  this->i2c = &i2c;
  this->address = address;
  this->fram_size = fram_size;
  if(!is_fram_ready()){
    init_fram();
  }
  init_flag = true;
}

int FRAMMenager::init_fram(){
  return 0;
}