#include "stmepic.hpp"
#include "device.hpp"
#include <memory>

using namespace stmepic;

DeviceThrededSettingsBase::DeviceThrededSettingsBase()
: uxStackDepth(256), uxPriority(tskIDLE_PRIORITY + 2), period(0) {
}

DeviceThrededSettingsBase *DeviceThrededSettingsBase::clone() const {
  return new DeviceThrededSettingsBase(*this);
}


DeviceThreadedBase::DeviceThreadedBase()
: task_handle(nullptr), settings(nullptr), task_running(false), task_args(nullptr) {
  // (void)device_task_stop();
}

DeviceThreadedBase::~DeviceThreadedBase() {
  (void)device_task_stop();
}


Status DeviceThreadedBase::device_task_run() {
  if(task_running)
    return Status::Cancelled("Task is already running");
  auto ret     = do_device_task_start();
  task_running = ret.ok();
  return ret;
}

Status DeviceThreadedBase::device_task_stop() {
  if(!task_running)
    return Status::Cancelled("Task is not running");
  auto ret     = do_device_task_stop();
  task_running = !ret.ok();
  return ret;
}

Status DeviceThreadedBase::device_task_set_settings(const DeviceThrededSettingsBase &settings) {
  if(task_running)
    return Status::Cancelled("Task is running");
  this->settings = std::unique_ptr<DeviceThrededSettingsBase>(settings.clone());
  if(this->settings == nullptr)
    return Status::OutOfMemory("Error allocating memory for settings");
  return Status::OK();
}

bool DeviceThreadedBase::device_task_is_running() const {
  return task_running;
}

Status DeviceThreadedBase::do_default_task_start(TaskFunction task, void *task_arg) {
  if(task == nullptr)
    return Status::Invalid("Task function is not provided");
  if(task_handle != nullptr)
    return Status::AlreadyExists("Task is already running");
  if(settings == nullptr)
    return Status::Invalid("Settings are not provided");
  // auto set        = static_cast<DeviceThrededSettingsDefault *>(settings.get());
  task_args       = std::unique_ptr<internall::DeviceTaskDefaultArgs>(new internall::DeviceTaskDefaultArgs());
  task_args->task = task;
  task_args->args = task_arg;
  task_args->period = settings->period;

  auto ret = xTaskCreate(default_task, "DeviceTask", settings->uxStackDepth, (void *)task_args.get(),
                         settings->uxPriority, &task_handle);
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
  auto task_arg_ptr          = static_cast<internall::DeviceTaskDefaultArgs *>(arg);
  TaskFunction *task         = task_arg_ptr->task;   // Task function
  void *args                 = task_arg_ptr->args;   // Task argument
  uint32_t ticks_wait_period = task_arg_ptr->period; // Task period
  // Task timing setup
  TickType_t xFrequency    = pdMS_TO_TICKS(ticks_wait_period);
  TickType_t xLastWakeTime = xTaskGetTickCount();

  // Task loop
  while(true) {
    task(args);
    vTaskDelay(xFrequency);
  }
}