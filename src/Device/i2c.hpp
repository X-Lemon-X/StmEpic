#pragma once

#include "stmepic.hpp"
#include "interfaces.hpp"

namespace stmepic {

class I2C : public HardwareInterface {
  public:
  I2C(I2C_HandleTypeDef& hi2c, const gpio::GpioPin& sda, const gpio::GpioPin& scl, const HardwareTy type)
  : _hi2c(&hi2c), _gpio_sda(sda), _gpio_sdc(scl), _hardwType(type) {
    _mutex = xSemaphoreCreateMutex();
  };

  ~I2C() {
    vSemaphoreDelete(_mutex);
  };

  void dma_callback(I2C_HandleTypeDef& hi2c) {
    if(hi2c.Instance != _hi2c->Instance)
      return;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(task_handle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }

  void it_callback(I2C_HandleTypeDef& hi2c) {
    if(hi2c.Instance != _hi2c->Instance)
      return;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(task_handle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }

  Status reset() {
    return Status::NotImplemented();
  }

  Status start() {
    return Status::NotImplemented();
  }

  Status stop() {
    return Status::NotImplemented();
  }

  Result<uint8_t*> read(uint16_t address, uint16_t mem_address, uint8_t* data, uint16_t size) {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    Result<uint8_t*> result = Status::ExecutionError();
    switch(_hardwType) {
    case HardwareTy::DMA: result = read_dma(address, mem_address, data, size); break;
    case HardwareTy::IRQ: result = read_irq(address, mem_address, data, size); break;
    case HardwareTy::BLOCKING: result = read_bl(address, mem_address, data, size); break;
    }
    xSemaphoreGive(_mutex);
    return result;
  }

  Status write(uint16_t address, uint16_t mem_address, uint16_t mem_size, uint8_t* data, uint16_t size) {
    switch(_hardwType) {
    case HardwareTy::DMA: return write_dma(address, mem_address, mem_size, data, size);
    case HardwareTy::IRQ: return write_irq(address, mem_address, mem_size, data, size);
    case HardwareTy::BLOCKING:
      return write_bl(address, mem_address, mem_size, data, size);
    }
  }

  private:
  SemaphoreHandle_t _mutex;
  const HardwareTy _hardwType;
  const gpio::GpioPin& _gpio_sda;
  const gpio::GpioPin& _gpio_sdc;
  I2C_HandleTypeDef* _hi2c;
  TaskHandle_t task_handle;

  Result<uint8_t*> read_dma(uint16_t address, uint16_t mem_address, uint8_t* data, uint16_t size) {
    return Status::NotImplemented();
  }

  Result<uint8_t*> read_irq(uint16_t address, uint16_t mem_address, uint8_t* data, uint16_t size) {
    return Status::NotImplemented();
  }

  Result<uint8_t*> read_bl(uint16_t address, uint16_t mem_address, uint8_t* data, uint16_t size) {
    return Status::NotImplemented();
  }

  Status write_dma(uint16_t address, uint16_t mem_address, uint16_t mem_size, uint8_t* data, uint16_t size) {
    vPortEnterCritical();
    task_handle = xTaskGetCurrentTaskHandle();
    auto stat = HAL_I2C_Mem_Write_DMA(_hi2c, address, mem_address, mem_size, data, size);
    vPortExitCritical();
    if(stat == HAL_OK) {
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
    return DeviceTranslateStatus::translate_hal_status_to_status(stat);
  }

  Status write_irq(uint16_t address, uint16_t mem_address, uint16_t mem_size, uint8_t* data, uint16_t size) {
    vPortEnterCritical();
    task_handle = xTaskGetCurrentTaskHandle();
    auto stat   = HAL_I2C_Mem_Write_IT(_hi2c, address, mem_address, mem_size, data, size);
    vPortExitCritical();
    if(stat == HAL_OK) {
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
    return DeviceTranslateStatus::translate_hal_status_to_status(stat);
  }

  Status write_bl(uint16_t address, uint16_t mem_address, uint16_t mem_size, uint8_t* data, uint16_t size) {
    vPortEnterCritical();
    auto stat = HAL_I2C_Mem_Write(_hi2c, address, mem_address, mem_size, data, size, HAL_MAX_DELAY);
    vPortExitCritical();
    return DeviceTranslateStatus::translate_hal_status_to_status(stat);
  }
};

} // namespace stmepic