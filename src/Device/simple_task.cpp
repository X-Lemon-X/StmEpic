#include "simple_task.hpp"

using namespace stmepic;

SimpleTask::SimpleTask()
: is_initiated(false), is_running(false), task_started(false), task_handle(nullptr), args(nullptr), task(nullptr),
  period_ms(0), stack_size(0), priority(0), name(nullptr), status(Status::Cancelled("Task not started")) {
}

SimpleTask::~SimpleTask() {
  if(is_running) {
    task_stop();
  }
}

Status SimpleTask::task_init(simple_task_function_pointer task,
                             void *task_arg,
                             uint32_t period_ms,
                             simple_task_function_pointer before_task_task,
                             uint32_t stack_size,
                             UBaseType_t priority,
                             const char *name) {
  if(is_initiated)
    return Status::AlreadyExists("Task is already initiated");

  if(task == nullptr)
    return Status::Invalid("Task function pointer is null");

  if(stack_size == 0) {
    return Status::Invalid("Task stack size is 0");
  }

  if(name == nullptr) {
    return Status::Invalid("Task name is null");
  }

  this->args             = task_arg;
  this->task             = task;
  this->before_task_task = before_task_task;
  this->period_ms        = period_ms;
  this->stack_size       = stack_size;
  this->priority         = priority;
  this->name             = name;
  task_started           = false;
  is_initiated           = true;
  return Status::OK();
}

Status SimpleTask::task_run() {
  if(!is_initiated) {
    status = Status::Invalid("Task is not initiated");
    return status;
  }

  if(is_running)
    return Status::AlreadyExists("Task is already running");

  if(xTaskCreate(task_function, name, stack_size, this, priority, &task_handle) != pdPASS) {
    status = Status::ExecutionError("Task creation failed");
    return status;
  }
  is_running = true;
  return Status::OK();
}

Status SimpleTask::task_stop() {
  if(!is_initiated)
    return Status::Invalid("Task is not initiated");

  if(!is_running)
    return Status::AlreadyExists("Task is not running");

  vTaskDelete(task_handle);
  is_running   = false;
  task_started = false;
  return Status::OK();
}

void SimpleTask::task_set_period(uint32_t period_ms) {
  vPortEnterCritical();
  this->period_ms = period_ms;
  vPortExitCritical();
}

Status SimpleTask::task_get_status() const {
  return status;
}

void SimpleTask::task_function(void *arg) {
  SimpleTask *task = static_cast<SimpleTask *>(arg);
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  if(task->before_task_task != nullptr) {
    auto stat = task->before_task_task(*task, task->args);
    if(!stat.ok()) {
      task->status = Status::ExecutionError("Task failed to start due to \"before_task_task\" failure!");
      task->task_started = true; // Mark as started to avoid blocking forever
      task->task_stop();
      return;
    }
  }
  task->status       = Status::OK("Task started successfully!");
  task->task_started = true;
  for(;;) {
    task->task(*task, task->args);
    TickType_t xFrequency = pdMS_TO_TICKS(task->period_ms);
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}


Status SimpleTask::task_wait_for_task_to_start(uint32_t timeout_ms) {
  if(!is_initiated) {
    return Status::Invalid("Task is not initiated");
  }

  TickType_t start_time = xTaskGetTickCount();
  if(timeout_ms == 0) {
    timeout_ms = portMAX_DELAY; // If timeout is 0, wait indefinitely
  } else {
    timeout_ms = pdMS_TO_TICKS(timeout_ms); // Convert milliseconds to ticks
  }

  while(!task_started) {
    if(timeout_ms > 0 && (xTaskGetTickCount() - start_time) >= timeout_ms) {
      return Status::TimeOut("Task did not start in time");
    }
    vTaskDelay(pdMS_TO_TICKS(10)); // Wait for 10 ms
  }
  return status;
}