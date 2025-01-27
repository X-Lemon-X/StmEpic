#include "stmepic.hpp"
#include "i2c.hpp"

using namespace stmepic;

// Define the static member
std::vector<std::shared_ptr<I2C>> I2C::i2c_interfaces;


// @brief I2C callback for DMA and IRQ
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) {
  I2C::run_tx_callbacks_from_irq(hi2c);
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
  I2C::run_rx_callbacks_from_irq(hi2c);
}

Result<std::shared_ptr<I2C>>
I2C::Make(I2C_HandleTypeDef &hi2c, const gpio::GpioPin &sda, const gpio::GpioPin &scl, const HardwareTy type) {
  static std::vector<std::shared_ptr<I2C>> i2c_instances;

  vPortEnterCritical();
  for(const auto &instance : i2c_instances) {
    if(instance->_hi2c->Instance == hi2c.Instance)
      return Status::AlreadyExists();
  }
  std::shared_ptr<I2C> i2c(new I2C(hi2c, sda, scl, type));
  i2c_instances.push_back(i2c);
  vPortExitCritical();
  return Result<decltype(i2c)>::OK(i2c);
}

void I2C::run_tx_callbacks_from_irq(I2C_HandleTypeDef *hi2c) {
  for(auto &i2c : i2c_interfaces) {
    if(i2c->_hi2c->Instance == hi2c->Instance) {
      i2c->tx_callback(hi2c);
      break;
    }
  }
}

void I2C::run_rx_callbacks_from_irq(I2C_HandleTypeDef *hi2c) {
  for(auto &i2c : i2c_interfaces) {
    if(i2c->_hi2c->Instance == hi2c->Instance) {
      i2c->rx_callback(hi2c);
      break;
    }
  }
}


I2C::I2C(I2C_HandleTypeDef &hi2c, const gpio::GpioPin &sda, const gpio::GpioPin &scl, const HardwareTy type)
: _hi2c(&hi2c), _gpio_sda(sda), _gpio_scl(scl), _hardwType(type) {
  _mutex = xSemaphoreCreateMutex();
};

I2C::~I2C() {
  vSemaphoreDelete(_mutex);
  vPortEnterCritical();
  for(auto it = i2c_interfaces.begin(); it != i2c_interfaces.end(); ++it) {
    if((*it)->_hi2c->Instance == _hi2c->Instance) {
      i2c_interfaces.erase(it);
      break;
    }
  }
  vPortExitCritical();
};

void I2C::tx_callback(I2C_HandleTypeDef *hi2c) {
  if(hi2c == nullptr || hi2c->Instance != _hi2c->Instance)
    return;

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(task_handle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void I2C::rx_callback(I2C_HandleTypeDef *hi2c) {
  if(hi2c == nullptr || hi2c->Instance != _hi2c->Instance)
    return;

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(task_handle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


Status I2C::reset() {
  STMEPIC_RETURN_ON_ERROR(stop());
  // restart
  GPIO_InitTypeDef gpioinit = { 0 };
  gpioinit.Pin              = _gpio_sda.pin;
  gpioinit.Mode             = GPIO_MODE_OUTPUT_PP;
  gpioinit.Pull             = GPIO_NOPULL;
  gpioinit.Speed            = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(_gpio_sda.port, &gpioinit);
  gpioinit.Pin = _gpio_scl.pin;
  HAL_GPIO_Init(_gpio_scl.port, &gpioinit);

  vPortEnterCritical();
  HAL_GPIO_WritePin(_gpio_sda.port, _gpio_sda.pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(_gpio_scl.port, _gpio_scl.pin, GPIO_PIN_SET);

  for(size_t i = 0; i < 20; i++) {
    HAL_GPIO_TogglePin(_gpio_scl.port, _gpio_scl.pin);
    HAL_Delay(1);
  }
  vPortExitCritical();
  return start();
}

Status I2C::start() {
  auto staus = HAL_I2C_Init(_hi2c);
  return staus;
}

Status I2C::stop() {
  auto status = HAL_I2C_DeInit(_hi2c);
  return status;
}

Status I2C::read(uint16_t address, uint16_t mem_address, uint8_t *data, uint16_t size, uint16_t mem_size) {
  xSemaphoreTake(_mutex, portMAX_DELAY);
  Status result = Status::ExecutionError();
  switch(_hardwType) {
  case HardwareTy::DMA: result = read_dma(address, mem_address, mem_size, data, size); break;
  case HardwareTy::IRQ: result = read_irq(address, mem_address, mem_size, data, size); break;
  case HardwareTy::BLOCKING: result = read_bl(address, mem_address, mem_size, data, size); break;
  }
  xSemaphoreGive(_mutex);
  return result;
}

Status I2C::write(uint16_t address, uint16_t mem_address, uint8_t *data, uint16_t size, uint16_t mem_size) {
  switch(_hardwType) {
  case HardwareTy::DMA: return write_dma(address, mem_address, mem_size, data, size);
  case HardwareTy::IRQ: return write_irq(address, mem_address, mem_size, data, size);
  case HardwareTy::BLOCKING: return write_bl(address, mem_address, mem_size, data, size);
  }
}

Status I2C::read_dma(uint16_t address, uint16_t mem_address, uint16_t mem_size, uint8_t *data, uint16_t size) {
  vPortEnterCritical();
  task_handle = xTaskGetCurrentTaskHandle();
  auto stat   = HAL_I2C_Mem_Read_DMA(_hi2c, address, mem_address, mem_size, data, size);
  vPortExitCritical();
  if(stat == HAL_OK) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  }
  return stat;
}

Status I2C::read_irq(uint16_t address, uint16_t mem_address, uint16_t mem_size, uint8_t *data, uint16_t size) {
  return Status::NotImplemented();
}

Status I2C::read_bl(uint16_t address, uint16_t mem_address, uint16_t mem_size, uint8_t *data, uint16_t size) {
  return Status::NotImplemented();
}

Status I2C::write_dma(uint16_t address, uint16_t mem_address, uint16_t mem_size, uint8_t *data, uint16_t size) {
  vPortEnterCritical();
  task_handle = xTaskGetCurrentTaskHandle();
  auto stat   = HAL_I2C_Mem_Write_DMA(_hi2c, address, mem_address, mem_size, data, size);
  vPortExitCritical();
  if(stat == HAL_OK) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  }
  return stat;
}

Status I2C::write_irq(uint16_t address, uint16_t mem_address, uint16_t mem_size, uint8_t *data, uint16_t size) {
  vPortEnterCritical();
  task_handle = xTaskGetCurrentTaskHandle();
  auto stat   = HAL_I2C_Mem_Write_IT(_hi2c, address, mem_address, mem_size, data, size);
  vPortExitCritical();
  if(stat == HAL_OK) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  }
  return stat;
}

Status I2C::write_bl(uint16_t address, uint16_t mem_address, uint16_t mem_size, uint8_t *data, uint16_t size) {
  vPortEnterCritical();
  auto stat = HAL_I2C_Mem_Write(_hi2c, address, mem_address, mem_size, data, size, HAL_MAX_DELAY);
  vPortExitCritical();
  return stat;
}