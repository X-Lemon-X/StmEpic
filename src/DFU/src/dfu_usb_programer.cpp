
#include "dfu_usb_programer.hpp"
#include "pin.hpp"
#include "usbd_cdc_if.h"
#include <string>
#include "logger.hpp"

using namespace stmepic;

uint8_t usb_programer_buffer[USB_PROGRAMER_BUFFER_SIZE];
uint32_t usb_programer_buffer_len = 0;
uint8_t usb_programer_data_recived = 0;

UsbProgramer::UsbProgramer(const GpioPin &_boot_device,Logger &_logger): boot_device(_boot_device),logger(_logger){
  WRITE_GPIO(this->boot_device,GPIO_PIN_RESET);
}

void UsbProgramer::reset_device(){
  HAL_NVIC_SystemReset();
}

void UsbProgramer::enter_dfu_mode(){

  WRITE_GPIO(this->boot_device, GPIO_PIN_SET);
  HAL_Delay(50);
  reset_device();
  WRITE_GPIO(this->boot_device,GPIO_PIN_RESET);
  while (true){}
}

void UsbProgramer::set_info(std::string info){
  usb_programer_info = info;
}

void UsbProgramer::handler(){
  // add thsi static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len);
  // to a file USB_DEVICE/App/usbd_cdc_if.h
  // CDC_Receive_FS(buffer, &size);
  if(usb_programer_data_recived == 0) return;
  usb_programer_data_recived = 0;
  uint32_t size = usb_programer_buffer_len;
  usb_programer_buffer_len = 0;
  if(strcmp((char*)usb_programer_buffer, USB_PROGRAMER_REBOOT)==0){
    logger.info("UsbProgramer:Rebooting device");
    reset_device();
  }
  else if(strcmp((char*)usb_programer_buffer, USB_PROGRAMER_PROGRAM)==0){
    logger.info("UsbProgramer: Entering USB-DFU mode");
    enter_dfu_mode();
  }else if(strcmp((char*)usb_programer_buffer, USB_PROGRAMER_INFO)==0){
    // HAL_Delay(2000);
    logger.info("UsbProgramer: Sending info");
    logger.info(std::string(usb_programer_info));
  }
}
