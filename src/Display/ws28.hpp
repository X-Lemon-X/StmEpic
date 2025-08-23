#include "main.h"
#include "stmepic.hpp"
#include "device.hpp"
#include <cmath>

using namespace stmepic;

namespace stmepic {

struct Color {
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
  uint8_t white = 0;
};

struct WS28Settings : DeviceSettings {
  uint16_t pixelCount;       // total number of pixels. 200 pixels max
  std::vector<Color> pixels; // vector of 3-element vectors. Each vector in this vector contains three values for red, green and blue in range 0-255 each
};

class WS28Base : public DeviceThreadedBase {
public:
  /// @brief Constructor for the WS2812B class
  /// @param htim Timer handle for the PWM signal generation
  /// @param timer_channel Timer channel for the PWM signal
  /// @note The timer channel should be the same as the one used for the PWM signal
  /// @note Timer should have auto-reload preload enabled
  /// @note DMA request should be added to selected timer and set to direction Memory To Peripheral on Circular Mode
  WS28Base(TIM_HandleTypeDef &htim, unsigned int timer_channel);

  /// @brief Update the current state of WS2812B tape
  /// @param pixels Vector of colors of every pixel
  Status update_pixels(const std::vector<Color> &pixels);

  Status device_set_settings(const DeviceSettings &settings) override;
  Status do_device_task_start() override;
  Status do_device_task_stop() override;
  Status device_start() override;
  Status device_stop() override;
  Status device_reset() override;
  Result<bool> device_is_connected() override;
  bool device_ok() override;
  Status device_get_status() override;

protected:
  uint16_t t0h_ns; // bit 0 HIGH time in nanoseconds
  uint16_t t1h_ns; // bit 1 HIGH time in nanoseconds
  uint16_t t0l_ns; // bit 0 LOW time in nanoseconds
  uint16_t t1l_ns; // bit 1 LOW time in nanoseconds
  uint16_t reset_time_ns; // reset time in nanoseconds
  bool is_RGBW; // does this specific ws28 have a RGBW system (false by deafult)

private:
  WS28Settings settings;
  uint16_t ledCount;

  unsigned int timer_channel;
  TIM_HandleTypeDef &htim;

  uint16_t pwm_bit_0; // duty cycle of bit 0
  uint16_t pwm_bit_1; // duty cycle of bit 1
  uint16_t reset_cycles_number; // number of cycles for the reset time

  std::vector<uint8_t> ledColors; // contains led colors in format {R1, G1, B1, R2, G2, ... , GN, BN} where N is piexelCount

  // zmienić na uniqe pointer
  std::unique_ptr<uint16_t[]> pwm_buffor;

  void setup_timer(); // sets prescaler, period, pwm_bit_0, pwm_bit_1 and inits PWM
  void fill_ledColors(const std::vector<Color> &pixels); // fills ledColors vector
  void pwm_buffor_fill();                                // fills buffor based on ledColors vector
};

}
