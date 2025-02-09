#include "simple_task.hpp"

using namespace stmepic;

SimpleTask::SimpleTask()
: is_initiated(false), is_running(false), task_handle(nullptr), args(nullptr), task(nullptr), period_ms(0),
  stack_size(0), priority(0), name(nullptr) {
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
  is_initiated           = true;
  return Status::OK();
}

Status SimpleTask::task_run() {
  if(!is_initiated)
    return Status::Invalid("Task is not initiated");

  if(is_running)
    return Status::AlreadyExists("Task is already running");

  if(xTaskCreate(task_function, name, stack_size, this, priority, &task_handle) != pdPASS) {
    return Status::ExecutionError("Task creation failed");
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
  is_running = false;
  return Status::OK();
}

void SimpleTask::task_set_period(uint32_t period_ms) {
  vPortEnterCritical();
  this->period_ms = period_ms;
  vPortExitCritical();
}

void SimpleTask::task_function(void *arg) {
  SimpleTask *task = static_cast<SimpleTask *>(arg);
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  if(task->before_task_task != nullptr)
    task->before_task_task(*task, task->args);
  for(;;) {
    task->task(*task, task->args);
    TickType_t xFrequency = pdMS_TO_TICKS(task->period_ms);
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}