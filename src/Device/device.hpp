#pragma once
#include "etl/unordered_map.h"
#include "etl/vector.h"
#include "stmepic.hpp"
#include "status.hpp"
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

namespace stmepic::internall {
struct DeviceTaskDefaultArgs;
}

namespace stmepic {
/**
 * @def DEVICE_MAX_DEVICE_COUNT
 * @brief Maximum number of devices that can be managed.
 */
#define DEVICE_MAX_DEVICE_COUNT (uint32_t)64
typedef void(TaskFunction)(void *);


/**
 * @class DeviceBase
 * @brief Abstract base class for all devices.
 *
 * This class provides the interface for device operations such as checking connection
 * status, getting device status, resetting, starting, and stopping the device.
 *
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
  /// @brief  Stack size for the task that will run on the device.
  StackType_t uxStackDepth;

  /// @brief Priority for the task that will run on the device.
  UBaseType_t uxPriority;

  /// @brief Period in ms for the task that will run on the device.
  uint32_t period;

  DeviceThrededSettingsBase();
  virtual DeviceThrededSettingsBase *clone() const;
};

/**
 * @class DeviceThreadedBase
 * @brief Abstract base class for all devices that run a task on the device.
 * This class provides the interface for device operations such as starting and stopping
 * a task that runs on the device. For example, if the device is a sensor, that requires some reading done in a loop.
 */
class DeviceThreadedBase : public DeviceBase {
public:
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
  [[nodiscard]] Status device_task_start();

  /**
   * @brief Stop the task that runs on the device.
   * This function is used to stop the task that runs on the device to do some work.
   * similat to device_task_start, but this function stops the task.
   * @return Status Status of the operation.
   */
  [[nodiscard]] Status device_task_stop();

  /**
   * @brief Set the settings for the task that will run on the device.
   * This function is used to set the settings for the task that will run on the device to do some work.
   * @param settings Settings for the task that will run on the device. should be cast to the specific settings struct for the specific device.
   */
  Status device_task_set_settings(const DeviceThrededSettingsBase &settings);

  /**
   * @brief Check if the task is running.
   * @return bool True if the task is running, false otherwise.
   */
  bool device_task_is_running() const;

protected:
  /**
   * @brief Pure virtual function to start the task that runs on the device.
   * This funciton should be overriden by the specific device to start the task that will run on the device.
   * to do some work. For example, if the device is a sensor, this function can be used to start reading sensor data.
   * However if you don't wont to add this fucntionity your self then simply make this function run the do_default_task_start
   * @param settings Settings for the task that will run on the device. should be cast to the specific settings struct for the specific device.
   * @return Status Status of the operation.
   */
  [[nodiscard]] virtual Status do_device_task_start() = 0;

  /**
   * @brief Pure virtual function to stop the task that runs on the device.
   * This funciton should be overriden by the specific device to stop the task that will run on the device.
   * However if you don't wont to add this fucntionity your self then simply make this function run the do_default_task_stop
   * @return Status Status of the operation.
   */
  [[nodiscard]] virtual Status do_device_task_stop() = 0;

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
  [[nodiscard]] Status do_default_task_start(TaskFunction task, void *task_arg);

  /**
   * @brief Stops the task that runs default task on the device.
   *
   * @return Status if the task was stopped successfully.
   */
  [[nodiscard]] Status do_default_task_stop();


private:
  std::unique_ptr<DeviceThrededSettingsBase> settings;
  std::unique_ptr<internall::DeviceTaskDefaultArgs> task_args;
  bool task_running;

  /**
   * @brief default task taht runs in a  infinit loop with a specified frequency.
   * @param arg DeviceTaskDefaultArgs struct that holds the task function, argument and period.
   *
   */
  static void default_task(void *arg);
};


} // namespace stmepic


namespace stmepic::internall {

/**
 * @struct DeviceTaskDefaultArgs
 * @brief Struct to hold the default arguments for the default task that will run on the device.
 */
struct DeviceTaskDefaultArgs {
  void *args;
  TaskFunction *task;
  uint32_t period;
};
} // namespace stmepic::internall