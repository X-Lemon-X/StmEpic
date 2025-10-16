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
    Ticker::get_instance().delay_nop(1000);
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

  if(_hardwType != HardwareType::BLOCKING && result.ok()) {
    if(task_handle != nullptr) {
      result = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(timeout_ms)) == 0 ?
               Status::TimeOut("I2C timeout did't receive response") :
               result;
    } else {
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

  if(_hardwType != HardwareType::BLOCKING && result.ok()) {
    if(task_handle != nullptr) {
      result = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(timeout_ms)) == 0 ? Status::OK() : result;
    } else {
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


Result<std::vector<uint16_t>> I2C::scan_for_devices() {
  if(!i2c_initialized)
    return Status::ExecutionError("I2C is not initialized");

  std::vector<uint16_t> devices;
  uint16_t max_address = _hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_7BIT ? 127 : 1023; // 7bit or 10bit

  for(uint16_t address = 1; address < max_address; address++) {
    auto status = is_device_ready(address, 1, 500);
    if(status.ok()) {
      devices.push_back(address);
    }
  }
  return Result<std::vector<uint16_t>>::OK(devices);
}


Result<std::shared_ptr<I2cMultiplexerGpioID>> I2cMultiplexerGpioID::Make(std::shared_ptr<I2C> i2c,
                                                                         uint8_t channels,
                                                                         GpioPin address_pin_1,
                                                                         std::optional<GpioPin> address_pin_2,
                                                                         std::optional<GpioPin> address_pin_3,
                                                                         std::optional<GpioPin> address_pin_4,
                                                                         uint8_t switch_delay_us) {
  if(i2c == nullptr)
    return Status::Invalid("I2C interface is null");
  if(channels < 1 || channels > 16)
    return Status::Invalid("Channels must be between 1 and 16");
  if(channels > 1 && !address_pin_2.has_value())
    return Status::Invalid("Address pin 2 must be provided for more than 1 channel");
  if(channels > 3 && !address_pin_3.has_value())
    return Status::Invalid("Address pin 3 must be provided for more than 3 channels");
  if(channels > 7 && !address_pin_4.has_value())
    return Status::Invalid("Address pin 4 must be provided for more than 7 channels");

  std::shared_ptr<I2cMultiplexerGpioID> mux(
  new I2cMultiplexerGpioID(i2c, address_pin_1, address_pin_2, address_pin_3, address_pin_4, switch_delay_us));
  return Result<decltype(mux)>::OK(mux);
}

I2cMultiplexerGpioID::I2cMultiplexerGpioID(std::shared_ptr<I2C> i2c,
                                           GpioPin address_pin_1,
                                           std::optional<GpioPin> address_pin_2,
                                           std::optional<GpioPin> address_pin_3,
                                           std::optional<GpioPin> address_pin_4,
                                           uint8_t switch_delay_us)
: _i2c(i2c), _address_pin_1(address_pin_1), _address_pin_2(address_pin_2), _address_pin_3(address_pin_3),
  _address_pin_4(address_pin_4), _channels(0), _selected_channel(1), _switch_delay_us(switch_delay_us) {
  _channels = 0;
  if(address_pin_2.has_value())
    _channels += 2;
  if(address_pin_3.has_value())
    _channels += 4;
  if(address_pin_4.has_value())
    _channels += 8;
  _channels += 1; // address pin 1 is mandatory

  _i2c_channels.resize(_channels + 1);
  for(uint8_t channel = 0; channel < _channels; channel++) {
    _i2c_channels[channel] = std::make_shared<I2cMultiplexerGpioIdNode>(i2c, channel, *this);
  }
  select_channel(0);
}

Status I2cMultiplexerGpioID::select_channel(uint8_t channel) {
  if(channel > _channels)
    return Status::Invalid("Channel out of range");
  if(channel == _selected_channel)
    return Status::OK();
  _address_pin_1.write((channel & 0x01) ? 1 : 0);
  if(_address_pin_2.has_value())
    _address_pin_2->write((channel & 0x02) ? 1 : 0);
  if(_address_pin_3.has_value())
    _address_pin_3->write((channel & 0x04) ? 1 : 0);
  if(_address_pin_4.has_value())
    _address_pin_4->write((channel & 0x08) ? 1 : 0);
  _selected_channel = channel;
  Ticker::get_instance().delay_nop(_switch_delay_us);
  return Status::OK();
}

uint8_t I2cMultiplexerGpioID::get_selected_channel() const {
  return _selected_channel;
}

Result<std::shared_ptr<I2cBase>> I2cMultiplexerGpioID::get_i2c_interface_for_channel(uint8_t channel) {
  if(channel >= _channels)
    return Status::Invalid("Channel out of range");
  return Result<std::shared_ptr<I2cBase>>::OK(_i2c_channels[channel]);
}

Status I2cMultiplexerGpioID::I2cMultiplexerGpioIdNode::hardware_start() {
  return _i2c->hardware_start();
}

Status I2cMultiplexerGpioID::I2cMultiplexerGpioIdNode::hardware_stop() {
  return _i2c->hardware_stop();
}

Status I2cMultiplexerGpioID::I2cMultiplexerGpioIdNode::hardware_reset() {
  return _i2c->hardware_reset();
}

Status I2cMultiplexerGpioID::I2cMultiplexerGpioIdNode::read(uint16_t address,
                                                            uint16_t mem_address,
                                                            uint8_t *data,
                                                            uint16_t size,
                                                            uint16_t mem_size,
                                                            uint16_t timeout_ms) {
  (void)_multiplexer.select_channel(channel);
  return _i2c->read(address, mem_address, data, size, mem_size, timeout_ms);
}

Status I2cMultiplexerGpioID::I2cMultiplexerGpioIdNode::write(uint16_t address,
                                                             uint16_t mem_address,
                                                             uint8_t *data,
                                                             uint16_t size,
                                                             uint16_t mem_size,
                                                             uint16_t timeout_ms) {
  (void)_multiplexer.select_channel(channel);
  return _i2c->write(address, mem_address, data, size, mem_size, timeout_ms);
}

Status I2cMultiplexerGpioID::I2cMultiplexerGpioIdNode::is_device_ready(uint16_t address, uint32_t trials, uint32_t timeout) {
  (void)_multiplexer.select_channel(channel);
  return _i2c->is_device_ready(address, trials, timeout);
}

Result<std::vector<uint16_t>> I2cMultiplexerGpioID::I2cMultiplexerGpioIdNode::scan_for_devices() {
  (void)_multiplexer.select_channel(channel);
  return _i2c->scan_for_devices();
}
