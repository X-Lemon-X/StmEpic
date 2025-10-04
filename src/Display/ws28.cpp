#include "ws28.hpp"

WS28Base::WS28Base(TIM_HandleTypeDef &htim, unsigned int timer_channel)
: htim(htim), timer_channel(timer_channel) {
  WS28Settings deafult_settings;
  deafult_settings.pixelCount = 1;

  Color white;
  white.red   = 255;
  white.green = 255;
  white.blue  = 255;
  is_RGBW     = false;

  deafult_settings.pixels.resize(deafult_settings.pixelCount, white);

  device_set_settings(deafult_settings);

  t0h_ns              = 1;
  t1h_ns              = 1;
  t0l_ns              = 1;
  t1l_ns              = 1;
  reset_time_ns       = 0;
  pwm_bit_0           = 0;
  pwm_bit_1           = 0;
  reset_cycles_number = 0;
}

void WS28Base::setup_timer() {
  const uint32_t pwmFreq = 1 / ((t0h_ns + t0l_ns) * 1e-9f);
  const uint32_t period  = HAL_RCC_GetSysClockFreq() / (pwmFreq)-1;

  __HAL_TIM_SET_PRESCALER(&htim, 0);
  __HAL_TIM_SET_AUTORELOAD(&htim, period);
  HAL_TIM_PWM_Init(&htim);

  pwm_bit_0 = (uint16_t)std::round((float)t0h_ns / ((float)t0h_ns + (float)t0l_ns) * (float)(period + 1));
  pwm_bit_1 = (uint16_t)std::round((float)t1h_ns / ((float)t0h_ns + (float)t0l_ns) * (float)(period + 1));
  reset_cycles_number = (float)reset_time_ns / (float)(t0h_ns + t0l_ns) + 1;
}

void WS28Base::fill_ledColors(const std::vector<Color> &pixels) {
  if(!is_RGBW) {
    for(uint16_t ledIndex = 0; ledIndex < ledCount; ledIndex += 3) {
      ledColors[ledIndex]     = pixels[ledIndex / 3].green;
      ledColors[ledIndex + 1] = pixels[ledIndex / 3].red;
      ledColors[ledIndex + 2] = pixels[ledIndex / 3].blue;
    }
  } else {
    for(uint16_t ledIndex = 0; ledIndex < ledCount; ledIndex += 4) {
      ledColors[ledIndex]     = pixels[ledIndex / 4].green;
      ledColors[ledIndex + 1] = pixels[ledIndex / 4].red;
      ledColors[ledIndex + 2] = pixels[ledIndex / 4].blue;
      ledColors[ledIndex + 3] = pixels[ledIndex / 4].white;
    }
  }
}

void WS28Base::pwm_buffor_fill() {
  if(!is_RGBW) {
    pwm_buffor = std::make_unique<uint16_t[]>(24 * settings.pixelCount + reset_cycles_number);
    std::fill_n(pwm_buffor.get(), 24 * settings.pixelCount + reset_cycles_number, 0);
  } else {
    pwm_buffor = std::make_unique<uint16_t[]>(32 * settings.pixelCount + reset_cycles_number);
    std::fill_n(pwm_buffor.get(), 32 * settings.pixelCount + reset_cycles_number, 0);
  }

  uint32_t b = 0;
  for(uint16_t ledIndex = 0; ledIndex < ledCount; ledIndex++) {
    for(int8_t bit = 7; bit >= 0; bit--) {
      if((ledColors[ledIndex] >> bit) & 1) {
        pwm_buffor[b] = pwm_bit_1;
      } else {
        pwm_buffor[b] = pwm_bit_0;
      }
      b++;
    }
  }
}

Status WS28Base::update_pixels(const std::vector<Color> &pixels) {
  fill_ledColors(pixels);
  pwm_buffor_fill();

  if(HAL_TIM_PWM_Start_DMA(&htim, timer_channel, (uint32_t *)pwm_buffor.get(),
                           24 * settings.pixelCount + reset_cycles_number) != HAL_OK) {
    return Status::HalError("Failed to start PWM");
  } else {
    return Status::OK();
  }

  return Status::OK();
}

Status WS28Base::device_set_settings(const DeviceSettings &_settings) {
  const auto *ws2812b_settings = static_cast<const WS28Settings *>(&_settings);
  settings                     = *ws2812b_settings;

  ledCount = settings.pixelCount * 3;
  ledColors.resize(ledCount, 0);

  Color black{};
  settings.pixels.resize(settings.pixelCount, black);

  fill_ledColors(settings.pixels);

  return Status::OK();
}

Status WS28Base::do_device_task_start() {
  setup_timer();
  pwm_buffor_fill();
  if(HAL_TIM_PWM_Start_DMA(&htim, timer_channel, (uint32_t *)pwm_buffor.get(),
                           24 * settings.pixelCount + reset_cycles_number) != HAL_OK) {
    return Status::HalError("Failed to start PWM");
  } else {
    return Status::OK();
  }
}

Status WS28Base::do_device_task_reset() {
  return Status::OK();
}

Status WS28Base::do_device_task_stop() {
  for(int pixelIndex = 0; pixelIndex < settings.pixelCount; pixelIndex++) {
    settings.pixels[pixelIndex].red   = 0;
    settings.pixels[pixelIndex].green = 0;
    settings.pixels[pixelIndex].blue  = 0;
    settings.pixels[pixelIndex].white = 0;
  }
  update_pixels(settings.pixels);

  HAL_TIM_PWM_Stop_DMA(&htim, timer_channel);
  return Status::OK();
}

Result<bool> WS28Base::device_is_connected() {
  return Status::OK();
}

bool WS28Base::device_ok() {
  return true;
}

Status WS28Base::device_get_status() {
  return Status::OK();
}
