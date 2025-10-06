#pragma once
/*
    SHA256 implementation, header file.

    This implementation was written by Kent "ethereal" Williams-King and is
    hereby released into the public domain. Do what you wish with it.

    No guarantees as to the correctness of the implementation are provided.
*/
#include "stmepic.hpp"
#include <limits>


/**
 * @file sha256.hpp
 * @brief SHA256 algoritm implementation.
 */

/**
 * @defgroup Hash
 * @brief Hashing algorithms like SHA256 etc.
 *
 * @{
 */

namespace stmepic::algorithm {

/// @brief Generate sha256 from provided data
class RandomNumberGenerator {
public:
  RandomNumberGenerator();

  static RandomNumberGenerator &get_instance();

#ifdef HAL_RNG_MODULE_ENABLED
  Status init(RNG_HandleTypeDef &hrng);
#endif

  Status init(uint32_t seed = Ticker::get_instance().get_micros());

  /// @brief Generate a random number
  /// @return a random number between 0 and 2^32-1
  uint32_t random();

  /// @brief Generate a random number between min and max
  /// @tparam T the type of the random number
  /// @param min the minimum value of the random number
  /// @param max the maximum value of the random number
  /// @return a random number between min and max
  template <typename T> T random(T min, T max) {
    float scale = (float)random() / (float)std::numeric_limits<uint32_t>::max();
    return (T)(min + scale * (max - min));
  }

private:
#ifdef HAL_RNG_MODULE_ENABLED
  RNG_HandleTypeDef *hrng;
#endif
};

} // namespace stmepic::algorithm
