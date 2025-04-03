#include "stmepic.hpp"
#include "device.hpp"
#include <memory>

using namespace stmepic;

DeviceThreadedSettings::DeviceThreadedSettings()
: uxStackDepth(456), uxPriority(tskIDLE_PRIORITY + 2), period(0) {
}

DeviceThreadedBase::DeviceThreadedBase() : task_running(false) {
}

DeviceThreadedBase::~DeviceThreadedBase() {
  (void)device_task_stop();
}


Status DeviceThreadedBase::device_task_start() {
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

Status DeviceThreadedBase::device_task_set_settings(const DeviceThreadedSettings &settings) {
  if(task_running)
    return Status::Cancelled("Task is running");
  this->settings = settings;
  return Status::OK();
}

bool DeviceThreadedBase::device_task_is_running() const {
  return task_running;
}

Status DeviceThreadedBase::do_default_task_start(task_function_pointer task,
                                                 task_function_pointer before_task_funciton,
                                                 void *task_arg) {
  if(task == nullptr)
    return Status::Invalid("Task function is not provided");
  STMEPIC_RETURN_ON_ERROR(task_s.task_init(task, task_arg, settings.period, before_task_funciton,
                                           settings.uxStackDepth, settings.uxPriority, "DeviceTask"));
  return task_s.task_run();
}

Status DeviceThreadedBase::do_default_task_stop() {
  return task_s.task_stop();
}
