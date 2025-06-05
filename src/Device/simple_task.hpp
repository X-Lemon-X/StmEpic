#pragma once
#include "stmepic.hpp"


#define FREQUENCY_TO_PERIOD_MS(frequency) (uint32_t)(1000.0f / (float)frequency)

namespace stmepic {


/**
 * @brief Class for creating simple tasks that run in a loop with a variable period.
 */
class SimpleTask {
private:
  /* data */
public:
  using simple_task_function_pointer = Status (*)(SimpleTask &, void *);

  SimpleTask();
  ~SimpleTask();

  /**
   * @brief Initiate the task configuration.
   * @param task function that will be run in the task loop (is shouldn't block or be infinite loop)
   * @param task_arg argument that will be passed to the task function
   * @param period_ms period in milliseconds of the delay between task runs
   * @param stack_size size of the stack for the task
   * @param priority priority of the task
   * @param name name of the task
   * @return Status
   */
  Status task_init(simple_task_function_pointer task,
                   void *task_arg,
                   uint32_t period_ms,
                   simple_task_function_pointer before_task_task = nullptr,
                   uint32_t stack_size                           = 244,
                   UBaseType_t priority                          = tskIDLE_PRIORITY + 2,
                   const char *name                              = "SimpleTask");

  /**
   * @brief Start the task
   *
   * @return Status
   */
  Status task_run();

  /**
   * @brief Stop the task
   *
   * @return Status
   */
  Status task_stop();

  /**
   * @brief Set the period of the task, can be changed while the task is running
   * @param period_ms period in milliseconds
   */
  void task_set_period(uint32_t period_ms);


  /**
   * @brief Get status of the task
   * @return Status of the task, can be used to check if the task is running or not
   */
  Status task_get_status() const;

  /**
   * @brief Wait for the task to start
   * This function will block until the task is started successfully as until before_task_task is executed correctly.
   * @param timeout_ms timeout in milliseconds, if 0 then it will wait indefinitely
   * @return Status of the task after waiting for it to start
   */
  [[nodiscard]]
  Status task_wait_for_task_to_start(uint32_t timeout_ms = 0);

private:
  SimpleTask(const SimpleTask &other)            = delete;
  SimpleTask &operator=(const SimpleTask &other) = delete;

  bool is_initiated;
  bool is_running;
  bool task_started;
  xTaskHandle task_handle;
  void *args;
  simple_task_function_pointer task;
  simple_task_function_pointer before_task_task;
  uint32_t period_ms;
  uint32_t stack_size;
  UBaseType_t priority;
  const char *name;
  Status status;
  static void task_function(void *arg);
};

} // namespace stmepic