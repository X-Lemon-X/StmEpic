#include "stmepic.hpp"
#include "uart.hpp"

using namespace stmepic;

// @brief I2C callback for DMA and IRQ
extern "C" {
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *hi2c) {
  UART::run_tx_callbacks_from_isr(hi2c, false);
}

void HAL_UART_TxHalCpltCallback(UART_HandleTypeDef *hi2c) {
  UART::run_tx_callbacks_from_isr(hi2c, true);
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *hi2c) {
  UART::run_rx_callbacks_from_isr(hi2c, false);
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *hi2c) {
  UART::run_rx_callbacks_from_isr(hi2c, true);
}
}


std::vector<std::shared_ptr<UART>> UART::uart_instances;

UART::UART(UART_HandleTypeDef &huart, const HardwareType type)
: _huart(&huart), _hardwType(type), _mutex(xSemaphoreCreateMutex()){};

UART::~UART() {
  vSemaphoreDelete(_mutex);
  vPortEnterCritical();
  for(auto it = uart_instances.begin(); it != uart_instances.end(); ++it) {
    if((*it)->_huart->Instance == _huart->Instance) {
      uart_instances.erase(it);
      break;
    }
  }
  vPortExitCritical();
};


Result<std::shared_ptr<UART>> UART::Make(UART_HandleTypeDef &huart, const HardwareType type) {
  vPortEnterCritical();
  for(const auto &instance : uart_instances) {
    if(instance->_huart->Instance == huart.Instance)
      return Status::AlreadyExists("UART already exists");
  }
  std::shared_ptr<UART> uart(new UART(huart, type));
  uart_instances.push_back(uart);
  vPortExitCritical();
  return Result<decltype(uart)>::OK(uart);
}

void UART::run_rx_callbacks_from_isr(UART_HandleTypeDef *huart, bool half) {
  for(auto &uart : uart_instances) {
    if(uart->_huart->Instance == huart->Instance) {
      uart->rx_callback(huart, half);
      break;
    }
  }
}


void UART::tx_callback(UART_HandleTypeDef *huart, bool half) {
  if(huart == nullptr || huart->Instance != huart->Instance)
    return;

  if(task_handle == nullptr) {
    dma_lock = false;
    return;
  }

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(task_handle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void UART::rx_callback(UART_HandleTypeDef *huart, bool half) {
  if(huart == nullptr || huart->Instance != _huart->Instance)
    return;

  if(task_handle == nullptr) {
    dma_lock = false;
    return;
  }
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(task_handle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


Status UART::hardware_reset() {
  STMEPIC_RETURN_ON_ERROR(hardware_stop());
  return hardware_start();
}

Status UART::hardware_start() {
  auto staus = HAL_UART_Init(_huart);
  if(_hardwType == HardwareType::DMA) {
  }
  return staus;
}

Status UART::hardware_stop() {
  auto status = HAL_UART_DeInit(_huart);
  return status;
}

Status UART::_read(uint8_t *data, uint16_t size, uint16_t timeout_ms) {
  task_handle = xTaskGetCurrentTaskHandle();
  if(task_handle != nullptr)
    vPortEnterCritical();
  Status result = Status::ExecutionError();
  switch(_hardwType) {
  case HardwareType::DMA:
    dma_lock = true;
    result   = HAL_UART_Receive_DMA(_huart, data, size);
    break;
  case HardwareType::IT:
    dma_lock = true;
    result   = HAL_UART_Receive_IT(_huart, data, size);
    break;
  case HardwareType::BLOCKING: result = HAL_UART_Receive(_huart, data, size, timeout_ms); break;
  }

  if(task_handle != nullptr)
    vPortExitCritical();
  return result;
}


Status UART::read(uint8_t *data, uint16_t size, uint16_t timeout_ms) {
  xSemaphoreTake(_mutex, portMAX_DELAY);
  Status result = Status::ExecutionError();
  task_handle   = xTaskGetCurrentTaskHandle();
  result        = _read(data, size);

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


Status UART::_write(uint8_t *data, uint16_t size, uint16_t timeout_ms) {
  task_handle = xTaskGetCurrentTaskHandle();
  if(task_handle != nullptr)
    vPortEnterCritical();
  Status result = Status::ExecutionError();
  switch(_hardwType) {
  case HardwareType::DMA:
    dma_lock = true;
    result   = HAL_UART_Transmit_DMA(_huart, data, size);
    break;
  case HardwareType::IT:
    dma_lock = true;
    result   = HAL_UART_Transmit_IT(_huart, data, size);
    break;
  case HardwareType::BLOCKING: result = HAL_UART_Transmit(_huart, data, size, timeout_ms); break;
  }
  if(task_handle != nullptr)
    vPortExitCritical();
  return result;
}

Status UART::write(uint8_t *data, uint16_t size, uint16_t timeout_ms) {
  xSemaphoreTake(_mutex, portMAX_DELAY);
  Status result = Status::ExecutionError();
  task_handle   = xTaskGetCurrentTaskHandle();
  result        = _read(data, size);

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