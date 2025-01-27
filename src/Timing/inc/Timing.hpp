#pragma once


#include "stmepic.hpp"
#include "stmepic_status.hpp"
#include <memory>
#include <vector>

/**
 * @file Timing.hpp
 * @brief  Ticker, Timing, and TimeScheduler classes, which provide
 * functionalities for handling time-based operations.
 *
 */

/**
 * @defgroup Timing
 * @brief Functions to control time-based operations.
 * Schedulers, timers, and tickers.
 * @{
 */

namespace stmepic {

/// @brief Convert frequency to period in microseconds
/// what for becouse i can
/// @param frequency frequency in Hz
uint32_t frequency_to_period_us(float frequency);

/**
 * @brief Tick class for time tracking.
 *
 * Used as a base Clock for all time-based operations, with 1us resolution.
 * The Ticer is used globally by multiple classes.
 * There fore it's important to initalise the static instance of the Ticker class.
 */
class Ticker {
public:
  /// @brief Construct a new Ticker object
  Ticker();

  /// @brief Init the timer object
  /// @param timer pointer to the timer object which will be used to count the time
  /// it't recommended to use 16 or 32 bit timer with 1us resolution
  /// the interrupt of the timer should be set to run every 1ms
  /// and timer COUNT register should represent single microsecond
  /// COUNT have to be set to 1000 to represent 1ms

  void init(TIM_HandleTypeDef *timer);

  /// @brief this function should be executed once in a timer interrupt for each passing 1ms, therefore the frequency of the imer interrupt shoul dbe set to exactly 1ms
  void irq_update_ticker();

  // void update_ticker_loop();

  /// @brief Get current time in microseconds
  /// @return  current time in microseconds [us]
  uint32_t get_micros();

  /// @brief get time in milliseconds
  /// @return current time in milliseconds [ms]
  uint32_t get_millis() const;

  /// @brief  get time in seconds with microsecond resolution
  /// @return  current time in seconds [s]
  float get_seconds();

  /// @brief Get the global instance of the Ticker object
  /// This shoule be initated otherwise bunch of other relying classes will not work
  static Ticker &get_instance();

private:
  uint32_t tick_millis;
  uint32_t tick_micros;
  TIM_HandleTypeDef *timer;
  static Ticker *ticker;
};


/**
 * @class Timing
 * @brief Class for handling time-based operations.
 *
 * This class provides functionalities for handling time-based operations.
 * It can be used to create timers that can be trigered after specified period of time.
 * Most of the time it will be used with TimeScheduler class.
 * To creat tasksk that run  with a specific frequency.
 * For example, to create a task that reads a sensor every 100ms, or blink an LED every 500ms.
 */

class Timing {
private:
  Ticker &ticker;
  uint32_t period;
  bool repeat, triggered_flag;
  bool timer_enabled;
  void (*function)(Timing &);

public:
  using callback_funciton = void (*)(Timing &);
  uint32_t last_time;
  uint32_t difference_d;
  uint32_t current_time_d;

  /// @brief Construct a new Timing object
  /// @param ticker reference to the ticker object with us resolution
  Timing(Ticker &ticker);

  /// @brief Make a new Timing object and assign function to be called when the timer triggers
  /// @return Technicaly it always returns OK so no need to check the status for now.
  static Result<std::shared_ptr<Timing>>
  Make(uint32_t period, bool repeat = true, callback_funciton function = nullptr, Ticker &ticker = Ticker::get_instance());

  /// @brief Set the behaviour of the timer
  /// @param period period of the timer in microseconds [us]
  /// @param repeat if the timer should repeat
  void set_behaviour(uint32_t period, bool repeat = true);

  /// @brief Check if the timer has triggered
  /// @return true if the timer has triggered
  bool triggered();

  /// @brief Reset the timer, it current time and repeat status
  void reset();

  /// @brief Run the function assigned to the timer if the timer is triggered
  void run_function();

  /// @brief allows to disbale and enabel timer freely
  void enable(bool timer_enabled);
};

/**
 * @class TimeScheduler
 * @brief Class for handling multiple timers.
 *
 * This class provides functionalities for handling multiple timers.
 * It can be used to add multiple then the class will handle them in the blocking or non-blocking mode.
 */
class TimeScheduler {
public:
  /// @brief Construct a new Time Scheduler object
  /// @param ticker reference to the ticker object with us resolution
  TimeScheduler(Ticker &ticker);

  /// @brief Add a timer to the scheduler
  /// @param timer shared pointer to the timer object
  [[nodiscard]] Status add_timer(std::shared_ptr<Timing> timer);

  /// @brief Handle all timers
  /// @note this function will never leave the loop and only run the timers
  void schedules_handle_blocking();

  /// @brief Handle all timers
  /// @note this function will run once over all timers and return, there forse should be called in a loop
  void schedules_handle_non_blocking();

private:
  Ticker &ticker;
  std::vector<std::shared_ptr<Timing>> timers;
};


} // namespace stmepic

/** @} */