#pragma once
#include "etl/unordered_map.h"
#include "etl/vector.h"
#include "stmepic.hpp"
#include "stmepic_status.hpp"
#include <cstddef>
#include <cstdint>

/**
 * @file device.hpp
 * @brief This file contains the MotorBase class definition.
 */


/**
 * @defgroup Devices
 * @brief Functions related to device control. From sensors to actuators. As well as task menagment for devices.
 * @{
 */

namespace stmepic {
/**
 * @def DEVICE_MAX_DEVICE_COUNT
 * @brief Maximum number of devices that can be managed.
 */
#define DEVICE_MAX_DEVICE_COUNT (uint32_t)64


/**
 * @class DeviceBase
 * @brief Abstract base class for all devices.
 *
 * This class provides the interface for device operations such as checking connection
 * status, getting device status, resetting, starting, and stopping the device.
 */

class DeviceBase {
public:
  DeviceBase()          = default;
  virtual ~DeviceBase() = default;


  /**
   * @brief Check if the device is connected.
   *
   * @return Result<bool> True if the device is connected, false otherwise.
   */
  [[nodiscard]] virtual Result<bool> device_is_connected() = 0;


  /**
   * @brief Check if the device is operating normally.
   *
   * @return bool True if the device is operating normally, false otherwise.
   */
  [[nodiscard]] virtual bool device_ok() = 0;


  /**
   * @brief Get the status of the device.
   * for example, if the device is connected, powered on, or if there is an error.
   * @return Status Device status.
   */
  [[nodiscard]] virtual Status device_get_status() = 0;

  /**
   * @brief Reset the device. This will usually require the hardware suport to work
   * normally. For example is some IC have reset pin, this will be used to reset the
   * device or if the IC power can be turned off and on. sometimes baybe it might be even
   * some somamnd send over some communication interface to reset the device.
   * If the device don't have start/stop functionality, this function should return OK.
   * @return Status Status of the operation.
   */
  [[nodiscard]] virtual Status device_reset() = 0;

  /**
   * @brief Enables the device. This will usually require the hardware suport to work
   * normally. For example is some IC have reset pin, this will be used to enable the
   * device. Similar to reset but this is to start the device.
   * If the device don't have start/stop functionality, this function should return OK.
   * @return Status Status of the operation.
   */
  [[nodiscard]] virtual Status device_enable() = 0;

  /**
   * @brief Disables the device. This will usually require the hardware suport to work
   * normally. For example is some IC have reset pin, this will be used to disable the
   * device. Similar to reset but this is to stop the device.
   * If the device don't have start/stop functionality, this function should return OK.
   * @return Status Status of the operation.
   */
  [[nodiscard]] virtual Status device_disable() = 0;
};

/**
 * @class DeviceThrededSettingsBase
 * @brief Abstract base struct to hold all setings for a device task to be run on the device.
 * usua;;y will be cast to the specific settings struct for specific device.
 */
struct DeviceThrededSettingsBase {
  StackType_t uxStackDepth;
  UBaseType_t uxPriority;

  DeviceThrededSettingsBase() : uxStackDepth(256), uxPriority(tskIDLE_PRIORITY + 2) {
  }
};

/**
 * @class DeviceThrededSettingsDefault
 * @brief Default settings for a device task to be run on the device.
 * This struct will be used to set the default settings for the task that will run on the device.
 */
struct DeviceThrededSettingsDefault : public DeviceThrededSettingsBase {
  uint32_t period; // in ms that will determine task run frequency
};


class DeviceThreadedBase : public DeviceBase {
public:
  typedef void(TaskFunction)(void *);

  DeviceThreadedBase();
  virtual ~DeviceThreadedBase();

  /**
   * @brief Run a task that runs on the device.
   * This function is used to start a task that runs on the device to do some work.
   * For example, if the device is a sensor, this function can be used to start reading
   * data from the sensor. Example for encoder this function would start a task that would
   * start reading the angles from the encoder in continous loop.
   * @param settings Settings for the task that will run on the device. should be cast to the specific settings struct for the specific device.
   * @return Status Status of the operation.
   */
  [[nodiscard]] Status device_task_run(const DeviceThrededSettingsBase &settings);

  /**
   * @brief Stop the task that runs on the device.
   * This function is used to stop the task that runs on the device to do some work.
   * similat to device_task_run, but this function stops the task.
   * @return Status Status of the operation.
   */
  [[nodiscard]] virtual Status device_task_stop();

private:
  void *task_arg_ptr[3];

  /**
   * @brief Pure virtual function to start the task that runs on the device.
   * This funciton should be overriden by the specific device to start the task that will run on the device.
   * to do some work. For example, if the device is a sensor, this function can be used to start reading sensor data.
   * However if you don't wont to add this fucntionity your self then simply make this function run the do_default_task_start
   * @param settings Settings for the task that will run on the device. should be cast to the specific settings struct for the specific device.
   * @return Status Status of the operation.
   */
  [[nodiscard]] virtual Status do_device_task_start(const DeviceThrededSettingsBase &settings) = 0;

  /**
   * @brief Pure virtual function to stop the task that runs on the device.
   * This funciton should be overriden by the specific device to stop the task that will run on the device.
   * However if you don't wont to add this fucntionity your self then simply make this function run the do_default_task_stop
   * @return Status Status of the operation.
   */
  [[nodiscard]] virtual Status do_device_task_stop() = 0;

  /**
   * @brief default task taht runs in a  infinit loop with a specified frequency.
   * @param arg 3 arguments that will be passed to the task function.
   * 1 - task function that will be run in a  loop
   * 2 - task argument that will be passed to the task function
   * 3 - period in ms that will determine task run frequency
   */
  static void default_task(void *arg);

protected:
  /// @brief FreeRtos Task handle for the specific device
  TaskHandle_t task_handle;

  /**
   * @brief Runs and Start task privided by the user. With specified frequency.
   *
   * @param settings Settings for the task that will run on the device. Can be customized.
   * @param task Task function that will be run. This function should be static if it is a member function.
   * @param task_arg Argument that will be passed to the task function.
   * Class instance for example that will be used in the task to do some work on.
   * @return Status if the task was started successfully.
   */
  [[nodiscard]] Status do_default_task_start(const DeviceThrededSettingsDefault &settings, TaskFunction task, void *task_arg);

  /**
   * @brief Stops the task that runs default task on the device.
   *
   * @return Status if the task was stopped successfully.
   */
  [[nodiscard]] Status do_default_task_stop();
};


} // namespace stmepic
