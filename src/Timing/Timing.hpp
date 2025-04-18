#pragma once


#include "stmepic.hpp"
#include "status.hpp"
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

  void init(TIM_HandleTypeDef *timer, TIM_HandleTypeDef *timer2 = nullptr);

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

  /// @brief Delay for a specified amount of time
  /// @param delay time to delay in miliseconds [ms]
  void delay(uint32_t miliseconds);

  /// @brief Delay for a specified amount of time in more less miliseconds
  /// @param delay time to delay in miliseconds [ms]
  void delay_nop(uint32_t miliseconds);


private:
  uint32_t tick_millis;
  uint32_t tick_micros;
  TIM_HandleTypeDef *timer;
  TIM_HandleTypeDef *timer2;
  static Ticker *ticker;
};


/**
 * @class Timer
 * @brief Class for handling time-based operations.
 *
 * This class provides functionalities for handling time-based operations.
 * It can be used to create timers that can be trigered after specified period of time.
 * usefull for operation that check if something was done in a specific time period.
 * Or for creatign simple task for whitch runing separate thread would be an overkill.
 * In task scenario Timing should be used with TimeScheduler.
 */

class Timer {
private:
  Ticker &ticker;
  uint32_t period;
  bool repeat, triggered_flag;
  bool timer_enabled;
  void (*function)(Timer &);

public:
  using callback_funciton = void (*)(Timer &);
  uint32_t last_time;
  uint32_t difference_d;
  uint32_t current_time_d;

  /// @brief Construct a new Timing object
  /// @param ticker reference to the ticker object with us resolution
  Timer(Ticker &ticker);

  /// @brief Make a new Timing object and assign function to be called when the timer triggers
  /// @return Technicaly it always returns OK so no need to check the status for now.
  static Result<std::shared_ptr<Timer>>
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

} // namespace stmepic

/** @} */