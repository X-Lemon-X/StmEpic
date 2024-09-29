#include "stm32f4xx_hal.h"
#include <vector>
#include <memory>

#ifndef TIMING_HPP
#define TIMING_HPP

namespace TIMING
{

/// @brief Convert frequency to period in microseconds
/// what for becouse i can
/// @param frequency frequency in Hz
uint32_t frequency_to_period(float frequency);

class Ticker{
private:
  uint32_t tick_millis;
  uint32_t tick_micros;
public:

  /// @brief Construct a new Ticker object
  Ticker();

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
};
  
class Timing
{
private:
  Ticker &ticker;
  uint32_t period;
  bool repeat, triggered_flag;
  bool timer_enabled;
  void (*function)(Timing&);

public:
  uint32_t last_time;
  uint32_t difference_d;
  uint32_t current_time_d;

  /// @brief Construct a new Timing object
  /// @param ticker reference to the ticker object with us resolution
  Timing(Ticker &ticker);

  /// @brief Make a new Timing object and assign function to be called when the timer triggers
  static std::shared_ptr<Timing> Make(Ticker &ticker, uint32_t period, bool repeat=true,void (*function)(Timing&) = nullptr);

  /// @brief Set the behaviour of the timer
  /// @param period period of the timer in microseconds [us]
  /// @param repeat if the timer should repeat
  void set_behaviour(uint32_t period, bool repeat=true);

  /// @brief Check if the timer has triggered
  /// @return true if the timer has triggered
  bool triggered();

  /// @brief Reset the timer, it current time and repeat status
  void reset();

  /// @brief Run the function assigned to the timer
  void run_function();

  /// @brief allows to disbale and enabel timer freely 
  void enable(bool timer_enabled);
};

class TimeScheduler
{
private:
  Ticker &ticker;
  std::vector<std::shared_ptr<Timing>> timers;

public:
  /// @brief Construct a new Time Scheduler object
  /// @param ticker reference to the ticker object with us resolution
  TimeScheduler(Ticker &ticker);

  /// @brief Add a timer to the scheduler
  void add_timer(std::shared_ptr<Timing> timer);

  /// @brief Handle all timers
  /// @note this function will never leave the loop and only run the timers
  void schedules_handle_blocking();

  /// @brief Handle all timers
  /// @note this function will run once over all timers and return, there forse should be called in a loop
  void schedules_handle_non_blocking();
};

} // namespace TIMING


#endif // TIMING_HPP