#pragma once

#include "stmepic.hpp"


namespace stmepic {


class MultiplexerBase {
public:
  MultiplexerBase() : _mutex(xSemaphoreCreateMutex()) {
  }
  virtual ~MultiplexerBase() = default;

  /// @brief Select the channel of the multiplexer
  /// @param channel the channel to select
  /// @return
  virtual Status select_channel(uint8_t channel) = 0;

  /// @brief Get the currently selected channel of the multiplexer
  /// @return the currently selected channel of the multiplexer
  virtual uint8_t get_selected_channel() const = 0;

  /// @brief Get the total number of channels of the multiplexer
  /// @return the total number of channels of the multiplexer
  virtual uint8_t get_total_channels() const = 0;

  /// @brief Lock the multiplexer for exclusive access
  void lock() {
    xSemaphoreTake(_mutex, portMAX_DELAY);
  }

  /// @brief Unlock the multiplexer for exclusive access
  void unlock() {
    xSemaphoreGive(_mutex);
  }

private:
  SemaphoreHandle_t _mutex;
};


} // namespace stmepic