#include "stmepic.hpp"
#include "device.hpp"


using namespace stmepic;


DeviceThreadedBase::DeviceThreadedBase() : task_handle(nullptr) {
  (void)device_task_stop();
}

DeviceThreadedBase::~DeviceThreadedBase() {
  (void)device_task_stop();
}


Status DeviceThreadedBase::device_task_run(const DeviceThrededSettingsBase &settings) {
  return do_device_task_start(settings);
}

Status DeviceThreadedBase::device_task_stop() {
  return do_device_task_stop();
}


Status DeviceThreadedBase::do_default_task_start(const DeviceThrededSettingsDefault &settings, TaskFunction task, void *task_arg) {
  if(task == nullptr)
    return Status::Invalid("Task function is not provided");
  if(task_handle != nullptr)
    return Status::AlreadyExists("Task is already running");
  task_arg_ptr[0] = (void *)&task;
  task_arg_ptr[1] = task_arg;
  task_arg_ptr[2] = (void *)&settings.period;

  auto ret = xTaskCreate(default_task, "DeviceTask", settings.uxStackDepth, task_arg_ptr, settings.uxPriority, &task_handle);
  return ret == pdPASS ? Status::OK() : Status::ExecutionError("Error creating task");
}

Status DeviceThreadedBase::do_default_task_stop() {
  if(task_handle == nullptr)
    return Status::ExecutionError("Task is not running");
  vTaskDelete(task_handle);
  task_handle = nullptr;
  return Status::OK();
}

void DeviceThreadedBase::default_task(void *arg) {
  TaskFunction *task         = (TaskFunction *)(arg);    // Function pointer
  void *task_arg             = (arg + 1);                // Task argument
  uint32_t ticks_wait_period = *((uint32_t *)(arg + 2)); // Task period

  if(ticks_wait_period == 0)
    ticks_wait_period = 1;

  // Task timing setup
  TickType_t xFrequency    = pdMS_TO_TICKS(ticks_wait_period);
  TickType_t xLastWakeTime = xTaskGetTickCount();

  // Task loop
  while(true) {
    task(task_arg);
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}