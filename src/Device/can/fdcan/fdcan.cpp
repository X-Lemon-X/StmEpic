#include "stmepic.hpp"
#include "hardware.hpp"
#include "can.hpp"
#include <cstring>
#include "fdcan.hpp"

#define CAN_SEND_RETRY_COUNT 20

static const uint32_t CAN_ALL_TX_BUFFERS =
FDCAN_TX_BUFFER0 | FDCAN_TX_BUFFER1 | FDCAN_TX_BUFFER2 | FDCAN_TX_BUFFER3 | FDCAN_TX_BUFFER4 | FDCAN_TX_BUFFER5 |
FDCAN_TX_BUFFER6 | FDCAN_TX_BUFFER7 | FDCAN_TX_BUFFER8 | FDCAN_TX_BUFFER9 | FDCAN_TX_BUFFER10 | FDCAN_TX_BUFFER11 |
FDCAN_TX_BUFFER12 | FDCAN_TX_BUFFER13 | FDCAN_TX_BUFFER14 | FDCAN_TX_BUFFER15 | FDCAN_TX_BUFFER16 |
FDCAN_TX_BUFFER17 | FDCAN_TX_BUFFER18 | FDCAN_TX_BUFFER19 | FDCAN_TX_BUFFER20 | FDCAN_TX_BUFFER21 |
FDCAN_TX_BUFFER22 | FDCAN_TX_BUFFER23 | FDCAN_TX_BUFFER24 | FDCAN_TX_BUFFER25 | FDCAN_TX_BUFFER26 |
FDCAN_TX_BUFFER27 | FDCAN_TX_BUFFER28 | FDCAN_TX_BUFFER29 | FDCAN_TX_BUFFER30 | FDCAN_TX_BUFFER31;

using namespace stmepic;

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hcan, uint32_t RxFifo0ITs) {
  FDCAN::run_rx_callbacks_from_irq(hcan, RxFifo0ITs);
}

void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hcan, uint32_t RxFifo1ITs) {
  FDCAN::run_rx_callbacks_from_irq(hcan, RxFifo1ITs);
}

void HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef *hcan, uint32_t BufferIndexes) {
  FDCAN::run_tx_callbacks_from_irq(hcan, BufferIndexes);
}

CanDataFrame::CanDataFrame()
: frame_id(0), remote_request(false), extended_id(false), data_size(0), fdcan_frame(true) {
  std::memset(data, 0, sizeof(data));
}

std::vector<std::shared_ptr<FDCAN>> FDCAN::can_instances;

Result<std::shared_ptr<FDCAN>>
FDCAN::Make(FDCAN_HandleTypeDef &hcan, const FDcanFilterConfig &filter, GpioPin *tx_led, GpioPin *rx_led) {
  vPortEnterCritical();
  for(const auto &instance : can_instances) {
    if(instance->_hcan->Instance == hcan.Instance)
      return Status::AlreadyExists();
  }

  if(filter.filters.size() == 0) {
    return Status::Invalid("Filter configuration is empty");
  }

  std::shared_ptr<FDCAN> can(new FDCAN(hcan, filter, tx_led, rx_led));
  can_instances.push_back(can);
  vPortExitCritical();
  return Result<decltype(can)>::OK(can);
}

void FDCAN::run_tx_callbacks_from_irq(FDCAN_HandleTypeDef *hcan, uint32_t BufferIndexes) {
  for(auto &can : can_instances) {
    if(can->_hcan->Instance == hcan->Instance) {
      can->tx_callback(hcan, BufferIndexes);
      break;
    }
  }
}

void FDCAN::run_rx_callbacks_from_irq(FDCAN_HandleTypeDef *hcan, uint32_t RxFifo0ITs) {
  for(auto &can : can_instances) {
    if(can->_hcan->Instance == hcan->Instance) {
      can->rx_callback(hcan, RxFifo0ITs);
      break;
    }
  }
}

FDCAN::FDCAN(FDCAN_HandleTypeDef &hcan, const FDcanFilterConfig &_filter, GpioPin *tx_led, GpioPin *rx_led)
: is_initiated(false), _hcan(&hcan), last_tx_mailbox(0), filter(_filter), _gpio_tx_led(tx_led), _gpio_rx_led(rx_led),
  task_handle_tx(nullptr), task_handle_rx(nullptr), tx_queue_handle(nullptr), rx_queue_handle(nullptr) {
  tx_queue_handle                 = xQueueCreate(CAN_QUEUE_SIZE, sizeof(CanDataFrame));
  rx_queue_handle                 = xQueueCreate(CAN_QUEUE_SIZE, sizeof(CanDataFrame));
  fdcan_in_fd_mode                = hcan.Init.FrameFormat == FDCAN_FRAME_CLASSIC ? false : true;
  fdcan_in_bitrate_switching_mode = false;

  switch(filter.fifo_number) {
  case FDCAN_FIFO::FDCAN_FIFO0: can_fifo = FDCAN_RX_FIFO0; break;
  case FDCAN_FIFO::FDCAN_FIFO1: can_fifo = FDCAN_RX_FIFO1; break;
  default: can_fifo = FDCAN_RX_FIFO0; break;
  }

  add_callback(0, default_callback_function, nullptr);
};

FDCAN::~FDCAN() {
  (void)hardware_stop();
  vQueueDelete(tx_queue_handle);
  vQueueDelete(rx_queue_handle);
}

Status FDCAN::hardware_reset() {
  STMEPIC_RETURN_ON_ERROR(hardware_stop());
  return hardware_start();
}

Status FDCAN::hardware_stop() {
  if(is_initiated == false) {
    return Status::OK();
  }

  if(task_handle_rx != nullptr) {
    vTaskDelete(task_handle_rx);
    task_handle_rx = nullptr;
  }
  if(task_handle_tx != nullptr) {
    vTaskDelete(task_handle_tx);
    task_handle_tx = nullptr;
  }
  while(eTaskGetState(task_handle_rx) != eDeleted || eTaskGetState(task_handle_tx) != eDeleted) {
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  if(_gpio_tx_led)
    _gpio_tx_led->write(0);
  if(_gpio_rx_led)
    _gpio_rx_led->write(0);


  task_handle_rx = nullptr;
  task_handle_tx = nullptr;
  xQueueReset(tx_queue_handle);
  xQueueReset(rx_queue_handle);
  STMEPIC_RETURN_ON_ERROR(Status(HAL_FDCAN_DeactivateNotification(_hcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE))); //| FDCAN_IT_RX_FIFO1_NEW_MESSAGE
  STMEPIC_RETURN_ON_ERROR(Status(HAL_FDCAN_Stop(_hcan)));
  STMEPIC_RETURN_ON_ERROR(Status(HAL_FDCAN_DeInit(_hcan)));
  is_initiated = false;
  return Status::OK();
}

Status FDCAN::hardware_start() {
  if(is_initiated == true) {
    return Status::OK();
  }
  STMEPIC_RETURN_ON_ERROR(Status(HAL_FDCAN_Init(_hcan)));
  for(auto &&fil : filter.filters) {
    STMEPIC_RETURN_ON_ERROR(Status(HAL_FDCAN_ConfigFilter(_hcan, &fil)));
  }

  STMEPIC_RETURN_ON_ERROR(Status(
  HAL_FDCAN_ConfigGlobalFilter(_hcan, filter.globalFilter_NonMatchingStd, filter.globalFilter_NonMatchingExt,
                               filter.globalFilter_RejectRemoteExt, filter.globalFilter_RejectRemoteStd)));

  // FDCAN_IT_RX_FIFO1_NEW_MESSAGE
  STMEPIC_RETURN_ON_ERROR(Status(HAL_FDCAN_Start(_hcan)));

  uint32_t active_it;
  if(filter.fifo_number == FDCAN_FIFO::FDCAN_FIFO0) {
    active_it = FDCAN_IT_RX_FIFO0_NEW_MESSAGE;
  } else {
    active_it = FDCAN_IT_RX_FIFO1_NEW_MESSAGE;
  }

  STMEPIC_RETURN_ON_ERROR(Status(HAL_FDCAN_ActivateNotification(_hcan, active_it, 0)));
  if(task_handle_rx == nullptr)
    xTaskCreate(FDCAN::task_rx, "FDCAN_RX", 1024, this, 1, &task_handle_rx);
  if(task_handle_tx == nullptr)
    xTaskCreate(FDCAN::task_tx, "FDCAN_TX", 1024, this, 1, &task_handle_tx);
  is_initiated = true;
  return Status::OK();
}

Status FDCAN::add_callback(uint32_t frame_id, internall::hardware_can_function_pointer callback, void *args) {
  if(callback == nullptr)
    return Status::Invalid("Callback function is null");

  internall::CanCallbackTask calldata;
  calldata.callback = callback;
  calldata.args     = args;

  if(frame_id == 0) {
    default_callback_task_data = calldata;
    return Status::OK();
  }

  vPortEnterCritical();
  if(callbacks.find(frame_id) != callbacks.end())
    return Status::AlreadyExists("Callback for can mgs already exists");
  callbacks[frame_id] = calldata;
  vPortExitCritical();
  return Status::OK();
}

Status FDCAN::remove_callback(uint32_t frame_id) {
  if(frame_id == 0) {
    default_callback_task_data = { nullptr, default_callback_function };
    return Status::OK();
  }

  vPortEnterCritical();
  if(callbacks.find(frame_id) == callbacks.end())
    return Status::KeyError("Callback for can mgs does not exists");
  callbacks.erase(frame_id);
  vPortExitCritical();
  return Status::OK();
}

void FDCAN::task_rx(void *arg) {
  auto can = static_cast<FDCAN *>(arg);
  CanDataFrame msg;
  stmepic::internall::CanCallbackTask *task = nullptr;
  while(true) {
    if(xQueueReceive(can->rx_queue_handle, &msg, 100) != pdTRUE)
      continue;

    vPortEnterCritical();
    auto mayby_task = can->callbacks.find(msg.frame_id);
    if(mayby_task != can->callbacks.end()) {
      task = &mayby_task->second;
    } else {
      task = &can->default_callback_task_data;
    }
    vPortExitCritical();
    if(can->_gpio_rx_led)
      can->_gpio_rx_led->write(0);
    // call the callback on a message
    task->callback(*can, msg, task->args);
  }
}

void FDCAN::task_tx(void *arg) {
  auto can         = static_cast<FDCAN *>(arg);
  CanDataFrame msg = {};
  while(true) {
    if(xQueueReceive(can->tx_queue_handle, &msg, 100) != pdTRUE) {
      if(can->_gpio_tx_led)
        can->_gpio_tx_led->write(0);
      continue;
    }
    uint8_t retrys_count = 0;
    while(HAL_FDCAN_GetTxFifoFreeLevel(can->_hcan) == 0 && retrys_count <= CAN_SEND_RETRY_COUNT) {
      vTaskDelay(5);
    }
    // if we are not able to send the message after somany retries then it won't be sent until kingdom come.
    if(CAN_SEND_RETRY_COUNT == retrys_count) {
      HAL_FDCAN_AbortTxRequest(can->_hcan, CAN_ALL_TX_BUFFERS);
    }

    FDCAN_TxHeaderTypeDef header;
    header.Identifier          = msg.frame_id;
    header.IdType              = msg.extended_id ? FDCAN_EXTENDED_ID : FDCAN_STANDARD_ID;
    header.TxFrameType         = msg.remote_request ? FDCAN_REMOTE_FRAME : FDCAN_DATA_FRAME;
    header.DataLength          = msg.data_size;
    header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    header.BitRateSwitch       = can->fdcan_in_bitrate_switching_mode ? FDCAN_BRS_ON : FDCAN_BRS_OFF;
    header.FDFormat            = can->fdcan_in_fd_mode ? FDCAN_FD_CAN : FDCAN_CLASSIC_CAN;
    header.TxEventFifoControl  = FDCAN_NO_TX_EVENTS;
    header.MessageMarker       = 0;
    HAL_FDCAN_AddMessageToTxFifoQ(can->_hcan, &header, msg.data);
    if(can->_gpio_tx_led)
      can->_gpio_tx_led->write(1);
  }
}

Status FDCAN::write(const CanDataFrame &msg) {
  if(xQueueSend(tx_queue_handle, &msg, pdMS_TO_TICKS(10)) != pdTRUE) {
    return Status::CapacityError("Queue is full, can't send message");
  }
  return Status::OK();
}

void FDCAN::tx_callback(FDCAN_HandleTypeDef *hcan, uint32_t BufferIndexes) {
  if(hcan->Instance != _hcan->Instance || !is_initiated)
    return;
  // if(_gpio_tx_led)
  //   _gpio_tx_led->write(0);
}

void FDCAN::rx_callback(FDCAN_HandleTypeDef *hcan, uint32_t RxFifo0ITs) {
  if(hcan->Instance != _hcan->Instance || !is_initiated)
    return;
  CanDataFrame msg = {};
  FDCAN_RxHeaderTypeDef header;
  if(HAL_FDCAN_GetRxMessage(hcan, can_fifo, &header, msg.data) != HAL_OK)
    return;
  if(_gpio_rx_led)
    _gpio_rx_led->write(1);

  msg.frame_id       = header.Identifier;
  msg.extended_id    = header.IdType == FDCAN_EXTENDED_ID ? true : false;
  msg.remote_request = header.RxFrameType == FDCAN_REMOTE_FRAME ? true : false;
  msg.data_size      = header.DataLength;
  // we ignore:
  // header.ErrorStateIndicator since we don't use it
  // header.BitRateSwitch since we don't use it
  msg.fdcan_frame = header.FDFormat == FDCAN_FD_CAN ? true : false;
  // also we ignore the
  // header.RxTimestamp
  // header.FilterIndex
  // header.IsFilterMatchingFrame
  BaseType_t hptw = pdFALSE;
  xQueueSendFromISR(rx_queue_handle, &msg, &hptw);
  portYIELD_FROM_ISR(hptw);
}

void FDCAN::default_callback_function(CanBase &can, CanDataFrame &msg, void *args) {
  (void)can;
  (void)args;
  (void)msg;
}