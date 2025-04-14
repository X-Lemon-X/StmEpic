#include "stmepic.hpp"
#include "hardware.hpp"
#include "can.hpp"
#include <cstring>
#include "can2.0.hpp"

#define CAN_SEND_RETRY_COUNT 20

using namespace stmepic;

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  CAN::run_rx_callbacks_from_irq(hcan);
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  CAN::run_rx_callbacks_from_irq(hcan);
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan) {
  CAN::run_tx_callbacks_from_irq(hcan);
}

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan) {
  CAN::run_tx_callbacks_from_irq(hcan);
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan) {
  CAN::run_tx_callbacks_from_irq(hcan);
}


CanDataFrame::CanDataFrame() : frame_id(0), remote_request(false), extended_id(false), data_size(0) {
  std::memset(data, 0, sizeof(data));
}

std::vector<std::shared_ptr<CAN>> CAN::can_instances;

Result<std::shared_ptr<CAN>>
CAN::Make(CAN_HandleTypeDef &hcan, const CAN_FilterTypeDef &filter, GpioPin *tx_led, GpioPin *rx_led) {
  vPortEnterCritical();
  for(const auto &instance : can_instances) {
    if(instance->_hcan->Instance == hcan.Instance)
      return Status::AlreadyExists();
  }
  std::shared_ptr<CAN> can(new CAN(hcan, filter, tx_led, rx_led));
  can_instances.push_back(can);
  vPortExitCritical();
  return Result<decltype(can)>::OK(can);
}

void CAN::run_tx_callbacks_from_irq(CAN_HandleTypeDef *hcan) {
  for(auto &can : can_instances) {
    if(can->_hcan->Instance == hcan->Instance) {
      can->tx_callback(hcan);
      break;
    }
  }
}

void CAN::run_rx_callbacks_from_irq(CAN_HandleTypeDef *hcan) {
  for(auto &can : can_instances) {
    if(can->_hcan->Instance == hcan->Instance) {
      can->rx_callback(hcan);
      break;
    }
  }
}

CAN::CAN(CAN_HandleTypeDef &hcan, const CAN_FilterTypeDef &_filter, GpioPin *tx_led, GpioPin *rx_led)
: is_initiated(false), _hcan(&hcan), last_tx_mailbox(0), can_fifo(_filter.FilterFIFOAssignment),
  filter(_filter), _gpio_tx_led(tx_led), _gpio_rx_led(rx_led), task_handle_tx(nullptr),
  task_handle_rx(nullptr), tx_queue_handle(nullptr), rx_queue_handle(nullptr) {
  tx_queue_handle = xQueueCreate(CAN_QUEUE_SIZE, sizeof(CanDataFrame));
  rx_queue_handle = xQueueCreate(CAN_QUEUE_SIZE, sizeof(CanDataFrame));
  can_fifo        = filter.FilterFIFOAssignment;
  add_callback(0, default_callback_function, nullptr);
};

CAN::~CAN() {
  (void)hardware_stop();
  vQueueDelete(tx_queue_handle);
  vQueueDelete(rx_queue_handle);
}

Status CAN::hardware_reset() {
  STMEPIC_RETURN_ON_ERROR(hardware_stop());
  return hardware_start();
}

Status CAN::hardware_stop() {
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
  task_handle_rx = nullptr;
  task_handle_tx = nullptr;
  xQueueReset(tx_queue_handle);
  xQueueReset(rx_queue_handle);
  STMEPIC_RETURN_ON_ERROR(
  Status(HAL_CAN_DeactivateNotification(_hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING)));
  STMEPIC_RETURN_ON_ERROR(Status(HAL_CAN_Stop(_hcan)));
  STMEPIC_RETURN_ON_ERROR(Status(HAL_CAN_DeInit(_hcan)));
  is_initiated = false;
  return Status::OK();
}

Status CAN::hardware_start() {
  if(is_initiated == true) {
    return Status::OK();
  }
  STMEPIC_RETURN_ON_ERROR(Status(HAL_CAN_Init(_hcan)));
  STMEPIC_RETURN_ON_ERROR(Status(HAL_CAN_ConfigFilter(_hcan, &filter)));
  STMEPIC_RETURN_ON_ERROR(Status(HAL_CAN_Start(_hcan)));
  STMEPIC_RETURN_ON_ERROR(
  Status(HAL_CAN_ActivateNotification(_hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING)));
  if(task_handle_rx == nullptr)
    xTaskCreate(CAN::task_rx, "CAN_RX", 1024, this, 1, &task_handle_rx);
  if(task_handle_tx == nullptr)
    xTaskCreate(CAN::task_tx, "CAN_TX", 254, this, 1, &task_handle_tx);
  is_initiated = true;
  return Status::OK();
}

Status CAN::add_callback(uint32_t frame_id, internall::hardware_can_function_pointer callback, void *args) {
  if(callback == nullptr)
    return Status::Invalid("Callback function is null");

  if(frame_id == 0) {
    internall::CanCallbackTask dc = { args, callback };
    default_callback_task_data    = dc;
    return Status::OK();
  }

  vPortEnterCritical();
  if(callbacks.find(frame_id) != callbacks.end())
    return Status::AlreadyExists("Callback for can mgs already exists");
  internall::CanCallbackTask calldata;
  calldata.callback   = callback;
  calldata.args       = args;
  callbacks[frame_id] = calldata;
  vPortExitCritical();
  return Status::OK();
}

Status CAN::remove_callback(uint32_t frame_id) {
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

void CAN::task_rx(void *arg) {
  auto can                                  = static_cast<CAN *>(arg);
  CanDataFrame msg                          = {};
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

void CAN::task_tx(void *arg) {
  auto can         = static_cast<CAN *>(arg);
  CanDataFrame msg = {};
  while(true) {
    if(xQueueReceive(can->tx_queue_handle, &msg, 100) != pdTRUE) {
      if(can->_gpio_tx_led)
        can->_gpio_tx_led->write(0);
      continue;
    }
    uint8_t retrys_count = 0;
    while(HAL_CAN_GetTxMailboxesFreeLevel(can->_hcan) == 0 && retrys_count <= CAN_SEND_RETRY_COUNT) {
      vTaskDelay(5);
    }
    // if we are not able to send the message after somany retries then it won't be sent until kingdom come.
    if(CAN_SEND_RETRY_COUNT == retrys_count) {
      HAL_CAN_AbortTxRequest(can->_hcan, CAN_TX_MAILBOX0 | CAN_TX_MAILBOX1 | CAN_TX_MAILBOX2);
    }

    CAN_TxHeaderTypeDef header;
    if(msg.extended_id) {
      header.ExtId = msg.frame_id;
      header.IDE   = CAN_ID_EXT;
    } else {
      header.StdId = msg.frame_id;
      header.IDE   = CAN_ID_STD;
    }
    header.DLC                = msg.data_size;
    header.RTR                = msg.remote_request ? CAN_RTR_REMOTE : CAN_RTR_DATA;
    header.TransmitGlobalTime = DISABLE;
    HAL_CAN_AddTxMessage(can->_hcan, &header, msg.data, &can->last_tx_mailbox);
    if(can->_gpio_tx_led)
      can->_gpio_tx_led->write(1);
  }
}

Status CAN::write(const CanDataFrame &msg) {
  // CanDataFrame *msg_s = (CanDataFrame *)pvPortMalloc(sizeof(CanDataFrame));
  // if(msg_s == nullptr)
  //   return Status::OutOfMemory("Can't allocate memory for CanDataFrame");
  // *msg_s = msg;
  if(xQueueSend(tx_queue_handle, &msg, pdMS_TO_TICKS(10)) != pdTRUE) {
    return Status::CapacityError("Queue is full, can't send message");
  }
  return Status::OK();
}

void CAN::tx_callback(CAN_HandleTypeDef *hcan) {
  if(hcan->Instance != _hcan->Instance || !is_initiated)
    return;
  if(_gpio_tx_led)
    _gpio_tx_led->write(0);
}

void CAN::rx_callback(CAN_HandleTypeDef *hcan) {
  if(hcan->Instance != _hcan->Instance || !is_initiated)
    return;
  CanDataFrame msg = {};
  CAN_RxHeaderTypeDef header;
  if(HAL_CAN_GetRxMessage(hcan, can_fifo, &header, msg.data) != HAL_OK)
    return;
  if(_gpio_rx_led)
    _gpio_rx_led->write(1);

  if(header.IDE == CAN_ID_EXT) {
    msg.frame_id    = header.ExtId;
    msg.extended_id = true;
  } else {
    msg.frame_id    = header.StdId;
    msg.extended_id = false;
  }
  msg.data_size      = header.DLC;
  msg.remote_request = header.RTR == CAN_RTR_REMOTE ? true : false;
  BaseType_t hptw    = pdFALSE;
  xQueueSendFromISR(rx_queue_handle, &msg, &hptw);
  portYIELD_FROM_ISR(hptw);
}

void CAN::default_callback_function(CanBase &can, CanDataFrame &msg, void *args) {
  (void)can;
  (void)args;
  (void)msg;
}