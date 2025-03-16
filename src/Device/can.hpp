#pragma once

#include "stmepic.hpp"
#include "hardware.hpp"
#include "device.hpp"
#include <unordered_map>


/**
 * @file can.hpp
 * @brief CAN interface wrapper that alow to do handle the CAN interface with ease to use callbacks. By adding
 * callbacks for specific frame ids. with nice rx and tx tasks that handle the traffic in the background.
 */

namespace stmepic {

class CanBase;

struct CanDataFrame {
public:
  CanDataFrame();
  /// @brief frame_id of the message
  uint32_t frame_id;

  /// @brief remote request flag
  bool remote_request;

  /// @brief extended id flag
  bool extended_id;

  /// @brief data of the message max 64 bits
  uint8_t data[8];

  /// @brief size of the data
  uint8_t data_size;
};


namespace internall {
/// @brief Callback function for the CAN interface
/// @param CanDataFrame the data of the incoming CAN frame
/// @param void* args provided by the user
using hardware_can_function_pointer = void (*)(CanBase &, CanDataFrame &, void *);

struct CanCallbackTask {
  void *args;
  hardware_can_function_pointer callback;
};
} // namespace internall


/**
 * @brief Class for controlling the CAN interface
 * automatically by allowing to add callbacks for specific frame ids.
 * as well as writing to interface from any task in a non blocking / thread safe fashion.
 */
class CanBase : public HardwareInterface {

public:
  CanBase(){};
  virtual ~CanBase(){};
  CanBase(const CanBase &) = delete;

  /**
   * @brief Write a CAN data frame to quue that will be then send to the CAN interface.
   * @param msg data frame that will be send
   * @return Status::OK if the data frame was added successfully
   */
  virtual Status write(const CanDataFrame &msg) = 0;

  /**
   * @brief Add a callback function to the CAN interface for specific frame id
   * The callback is run in a RX task there fore it don't have to bo super fast but it shouldn't be too slow either.
   * @note The CAN have default callback that runs for all IDs that don't have a registered callback.
   * You CAN change the default callback by adding your custom callback with the frame_id = 0
   * @param frame_id the ID of the CAN data frame on which the callback will be called
   * @param callback the callback function that will be called when the frame_id is received
   * @param args the arguments that will be passed to the callback function
   * @return Status OK if the callback was added successfully
   */
  virtual Status
  add_callback(uint32_t frame_id, internall::hardware_can_function_pointer callback, void *args = nullptr) = 0;

  /**
   * @brief Remove the callback function for the specific frame id
   *
   * @param frame_id the ID of the CAN data frame on which the callback will be removed
   * @return Status OK if the callback was removed successfully
   */
  virtual Status remove_callback(uint32_t frame_id) = 0;
};


} // namespace stmepic