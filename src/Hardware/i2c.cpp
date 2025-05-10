#include "stmepic.hpp"
#include "i2c.hpp"

using namespace stmepic;

// @brief I2C callback for DMA and IRQ
extern "C" {
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) {
  I2C::run_tx_callbacks_from_isr(hi2c);
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
  I2C::run_rx_callbacks_from_isr(hi2c);
}
}

std::vector<std::shared_ptr<I2C>> I2C::i2c_instances;

I2C::I2C(I2C_HandleTypeDef &hi2c, GpioPin &sda, GpioPin &scl, const HardwareType type)
: _hi2c(&hi2c), _gpio_sda(sda), _gpio_scl(scl), _hardwType(type), dma_lock(false), i2c_initialized(false) {
  _mutex = xSemaphoreCreateMutex();
};

I2C::~I2C() {
  vSemaphoreDelete(_mutex);
  vPortEnterCritical();
  for(auto it = i2c_instances.begin(); it != i2c_instances.end(); ++it) {
    if((*it)->_hi2c->Instance == _hi2c->Instance) {
      i2c_instances.erase(it);
      break;
    }
  }
  vPortExitCritical();
};


Result<std::shared_ptr<I2C>> I2C::Make(I2C_HandleTypeDef &hi2c, GpioPin &sda, GpioPin &scl, const HardwareType type) {
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

void I2C::run_tx_callbacks_from_isr(I2C_HandleTypeDef *hi2c) {
  for(auto &i2c : i2c_instances) {
    if(i2c->_hi2c->Instance == hi2c->Instance) {
      i2c->tx_callback(hi2c);
      break;
    }
  }
}

void I2C::run_rx_callbacks_from_isr(I2C_HandleTypeDef *hi2c) {
  for(auto &i2c : i2c_instances) {
    if(i2c->_hi2c->Instance == hi2c->Instance) {
      i2c->rx_callback(hi2c);
      break;
    }
  }
}


void I2C::tx_callback(I2C_HandleTypeDef *hi2c) {
  if(hi2c == nullptr || hi2c->Instance != _hi2c->Instance)
    return;

  if(task_handle == nullptr) {
    dma_lock = false;
    return;
  }

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(task_handle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void I2C::rx_callback(I2C_HandleTypeDef *hi2c) {
  if(hi2c == nullptr || hi2c->Instance != _hi2c->Instance)
    return;

  if(task_handle == nullptr) {
    dma_lock = false;
    return;
  }
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(task_handle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


Status I2C::hardware_reset() {
  STMEPIC_RETURN_ON_ERROR(hardware_stop());
  // restart
  // vPortEnterCritical();
  GPIO_InitTypeDef gpioinit = { 0 };
  gpioinit.Pin              = _gpio_sda.pin;
  gpioinit.Mode             = GPIO_MODE_OUTPUT_PP;
  gpioinit.Pull             = GPIO_NOPULL;
  gpioinit.Speed            = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(&_gpio_sda.port, &gpioinit);
  gpioinit.Pin = _gpio_scl.pin;
  HAL_GPIO_Init(&_gpio_scl.port, &gpioinit);

  _gpio_sda.write(1);
  _gpio_scl.write(1);

  for(size_t i = 0; i < 20; i++) {
    _gpio_scl.toggle();
    Ticker::get_instance().delay_nop(1);
  }
  // vPortExitCritical();
  return hardware_start();
}

Status I2C::hardware_start() {
  auto staus = HAL_I2C_Init(_hi2c);
  if(staus != HAL_OK)
    return Status(staus);
  i2c_initialized = true;
  return staus;
}

Status I2C::hardware_stop() {
  auto status     = HAL_I2C_DeInit(_hi2c);
  i2c_initialized = false;
  return status;
}

Status I2C::_read(uint16_t address, uint16_t mem_address, uint8_t *data, uint16_t size, uint16_t mem_size, uint16_t timeout_ms) {
  task_handle = xTaskGetCurrentTaskHandle();

  address = address << 1;
  if(task_handle != nullptr)
    vPortEnterCritical();
  Status result = Status::ExecutionError();
  switch(_hardwType) {
  case HardwareType::DMA:
    dma_lock = true;
    result   = HAL_I2C_Mem_Read_DMA(_hi2c, address, mem_address, mem_size, data, size);
    break;
  case HardwareType::IT:
    dma_lock = true;
    result   = HAL_I2C_Mem_Read_IT(_hi2c, address, mem_address, mem_size, data, size);
    break;
  case HardwareType::BLOCKING:
    result = HAL_I2C_Mem_Read(_hi2c, address, mem_address, mem_size, data, size, timeout_ms);
    break;
  }
  if(task_handle != nullptr)
    vPortExitCritical();
  return result;
}


Status I2C::read(uint16_t address, uint16_t mem_address, uint8_t *data, uint16_t size, uint16_t mem_size, uint16_t timeout_ms) {
  if(!i2c_initialized)
    return Status::ExecutionError("I2C is not initialized");

  xSemaphoreTake(_mutex, portMAX_DELAY);
  Status result = Status::ExecutionError();
  task_handle   = xTaskGetCurrentTaskHandle();
  result        = _read(address, mem_address, data, size, mem_size, timeout_ms);

  if(_hardwType != HardwareType::BLOCKING) {
    if(result.ok() && task_handle != nullptr) {
      ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(timeout_ms));
    } else if(result.ok() && task_handle == nullptr) {
      while(dma_lock)
        __NOP();
    }
  }
  xSemaphoreGive(_mutex);
  return result;
}


Status I2C::_write(uint16_t address, uint16_t mem_address, uint8_t *data, uint16_t size, uint16_t mem_size, uint16_t timeout_ms) {
  task_handle = xTaskGetCurrentTaskHandle();
  if(task_handle != nullptr)
    vPortEnterCritical();
  address       = address << 1;
  Status result = Status::ExecutionError();
  switch(_hardwType) {
  case HardwareType::DMA:
    dma_lock = true;
    result   = HAL_I2C_Mem_Write_DMA(_hi2c, address, mem_address, mem_size, data, size);
    break;
  case HardwareType::IT:
    dma_lock = true;
    result   = HAL_I2C_Mem_Write_IT(_hi2c, address, mem_address, mem_size, data, size);
    break;
  case HardwareType::BLOCKING:
    result = HAL_I2C_Mem_Write(_hi2c, address, mem_address, mem_size, data, size, timeout_ms);
    break;
  }
  if(task_handle != nullptr)
    vPortExitCritical();
  return result;
}

Status I2C::write(uint16_t address, uint16_t mem_address, uint8_t *data, uint16_t size, uint16_t mem_size, uint16_t timeout_ms) {
  if(!i2c_initialized)
    return Status::ExecutionError("I2C is not initialized");

  if(size != 0 && data == nullptr)
    return Status::Invalid("Data pointer is null");

  xSemaphoreTake(_mutex, portMAX_DELAY);
  Status result = Status::ExecutionError();
  task_handle   = xTaskGetCurrentTaskHandle();
  result        = _write(address, mem_address, data, size, mem_size, timeout_ms);

  if(_hardwType != HardwareType::BLOCKING) {
    if(result.ok() && task_handle != nullptr) {
      ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(timeout_ms));
    } else if(result.ok() && task_handle == nullptr) {
      while(dma_lock)
        __NOP();
    }
  }
  xSemaphoreGive(_mutex);
  return result;
}

Status I2C::is_device_ready(uint16_t address, uint32_t trials, uint32_t timeout) {
  if(!i2c_initialized)
    return Status::ExecutionError("I2C is not initialized");

  xSemaphoreTake(_mutex, portMAX_DELAY);
  address       = address << 1;
  Status status = HAL_I2C_IsDeviceReady(_hi2c, address, trials, timeout);
  xSemaphoreGive(_mutex);
  return status;
}


Result<std::vector<uint8_t>> I2C::scan_for_devices() {
  if(!i2c_initialized)
    return Status::ExecutionError("I2C is not initialized");

  std::vector<uint8_t> devices;
  uint16_t max_address = _hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_7BIT ? 127 : 1023; // 7bit or 10bit

  for(uint8_t address = 1; address < max_address; address++) {
    auto status = is_device_ready(address, 1, 500);
    if(status.ok()) {
      devices.push_back(address);
    }
  }
  return Result<std::vector<uint8_t>>::OK(devices);
}