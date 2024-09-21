#ifndef ENCODERS_H
#define ENCODERS_H

#include "stm32f4xx_hal.h"
#include "Timing.hpp"
#include "filter.hpp"

#define VELOCITY_FILTER_SIZE 6
#define ANGLE_MAX_DEFFERENCE 2.0f // 1 radian



namespace ENCODER {

/// @brief translates the register data to angle for AS5600 magnetic encoder 
uint16_t translate_reg_to_angle_AS5600(uint8_t first,uint8_t second);

/// @brief translates the register data to angle for MT6701 magnetic encoder
uint16_t translate_reg_to_angle_MT6701(uint8_t first,uint8_t second);


class Encoder {
private:
  I2C_HandleTypeDef *hi2c;
  TIMING::Ticker *ticker;
  FILTERS::FilterBase *filter_angle;
  FILTERS::FilterBase *filter_velocity;

  bool encoder_connected;

  uint8_t data[2];
  float last_time;
  float prev_angle;
  float current_velocity;

  float prev_angle_velocity;
  float over_drive_angle;
  float absolute_angle;
  float ratio;

  uint16_t resolution;
  float offset;
  float dead_zone_correction_angle;
  bool reverse;
  uint8_t address;
  uint8_t angle_register;
  uint8_t magnes_detection_register;
  bool enable_filter;
  bool enable_velocity;
  bool enable_velocity_filter;
  uint16_t velocity_sample_count;
  uint16_t velocity_samples_amount;

  uint16_t (*translate_reg_to_angle_function)(uint8_t,uint8_t);
  
  /// @brief Calucaltes velcoicty, and passes it thoroung a filter
  /// @param angle current angle
  /// @param current_time  current time
  /// @return returns the filtered velocity
  float calculate_velocity(float angle);


  /// @brief reads the angle from the encoder and converts it to radians
  /// applays the offset and the reverse
  float read_angle_rads();

public:
  
  /// @brief Init fucnion of the encoder
  Encoder();
  
  /// @brief Initiates the encoder
  /// should be called after all the settings are set  especially the dead zone correction angle if used
  void init(I2C_HandleTypeDef &hi2c,TIMING::Ticker &ticker , FILTERS::FilterBase *filter_angle,FILTERS::FilterBase *filter_velocity);

  /// @brief Pings the encoder to check if it is connected
  /// @return true if the encoder is connected
  bool ping_encoder();

  /// @brief Check if last request to the encoder was successful if not the encoder is disconnected,
  /// therefor it requires to be used when handle is called regularly, otherwise the return value will be unreliable.
  /// use ping_encoder() if you want to check if the encoder is connected.
  /// @return true if the encoder is connected.
  bool is_connected() const;

  /// @brief handles the encoder updates data read from the encoder
  void handle();

  /// @brief reads the raw angle from the encoder
  /// @return the raw angle in uint16_t
  uint16_t read_raw_angle();

  /// @brief reads the angle from the encoder
  /// @return the angle in radians
  float read_angle();

  /// @brief reads the last calculated velocity
  /// @return the velocity in radians per second 
  float get_velocity() const;

  /// @brief gets the last calcualted angle with offset and reverse
  /// @return the angle in radians
  float get_angle() const;

  /// @brief gets the latest calculated absolute angle 
  /// absoulte angle includes information how many times the encoder has rotated
  /// for example if the encoder has rotated 3 times the angle will be 6pi + the current angle
  /// @return the absoulte angle in radians
  float get_absoulute_angle() const;

  /// @brief sets the resolution of the encoder
  /// @param resolution the resolution of the encoder 
  Encoder& set_resolution(uint16_t resolution);
  
  /// @brief sets the offset of the encoder
  /// @param offset the offset in radians
  Encoder& set_offset(float offset);
  
  /// @brief sets the reverse of the encoder
  /// @param reverse true if the encoder is reversed
  Encoder& set_reverse(bool reverse);
  
  /// @brief sets the address of the encoder 
  /// @param address the address of the encoder
  Encoder& set_address(uint8_t address);
  
  /// @brief sets the angle register of the encoder
  /// @param angle_register the angle register of the encoder
  Encoder& set_angle_register(uint8_t angle_register);
  
  /// @brief sets the magnes detection register of the encoder (unsuported feature in MT6701)
  /// @param magnes_detection_register the magnes detection register of the encoder 
  Encoder& set_magnes_detection_register(uint8_t magnes_detection_register);
  
  /// @brief sets the enable filter of the encoder 
  /// @param enable_filter true if the filter is enabled
  Encoder& set_enable_position_filter(bool enable_filter);
  
  /// @brief sets the enable velocity of the encoder 
  /// @param enable_velocity true if the you wnat the velocity to be calculated
  Encoder& set_enable_velocity(bool enable_velocity);
  
  /// @brief sets the enable velocity filter of the encoder 
  /// @param enable_velocity_filter true if the velocity filter is enabled
  Encoder& set_enable_velocity_filter(bool enable_velocity_filter);

  /// @brief sets how many smaples will be skipped before calculating the velocity
  /// fixes the problem of low velocity readings
  /// @param velocity_samples_amount the amount of samples to skip before calculating the velocity
  Encoder& set_velocity_sample_amount(uint16_t velocity_samples_amount);\

  /// @brief sets the begin roation dead zone correction angle
  /// The dead zone angle is used to correct initial value of the angle to be eather positive or negative.
  /// Since the same angle can be read as either positive or negative value for exmple -pi/2 or 3pi/2
  /// the dead zone angle is used to correct this by making the angle 
  /// In the counter clockwise direction on the left side of the dead zone angle negative and on the right side positive
  /// @param begin_roation_dead_zone_correction_angle the angle in radians, can be set to 0 if dead zone correction angle shall not be used.
  Encoder& set_dead_zone_correction_angle(float begin_roation_dead_zone_correction_angle);


  /// @brief sets the function that will be used to translate register data to angle.
  /// It have to return the angle in uint16_t and takes the data from two registers first and second.
  /// @param function the function that will be used to read the angle
  /// @return the angle in uint16_t in binary format for example 0-4095 or 0-16383
  Encoder& set_function_to_read_angle(uint16_t (*function)(uint8_t,uint8_t));

  /// @brief set the ratio that will be multiplayed by value of the angle and velocity
  /// @param ratio the ratio that will be multiplayed by value of the angle and velocity
  Encoder& set_ratio(float ratio);

};



}


#endif // ENCODERS_H