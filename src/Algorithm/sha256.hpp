#pragma once
/*
    SHA256 implementation, header file.

    This implementation was written by Kent "ethereal" Williams-King and is
    hereby released into the public domain. Do what you wish with it.

    No guarantees as to the correctness of the implementation are provided.
*/
#include "stmepic.hpp"


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
class SHA256 {
public:
  /// @brief The size of the output of the sha256
  static const uint8_t SHA256_OUTPUT_SIZE = 32;

  /// @brief SHA256 generate sha256 from provided data
  /// @param data the data that will be hashed
  /// @param len the size of the data
  /// @param output the output buffer from uint8_t that will contain the hash with size SHA256_OUTPUT_SIZE
  static void sha256(const void *data, uint64_t len, void *output);

private:
  static void sha256_endian_reverse64(uint64_t input, uint8_t *output);
  static uint32_t sha256_endian_read32(uint8_t *input);
  static void sha256_endian_reverse32(uint32_t input, uint8_t *output);
  static uint32_t sha256_ror(uint32_t input, uint32_t by);


  static const uint32_t sha256_initial_h[8];
  static const uint32_t sha256_round_k[64];
};

} // namespace stmepic::algorithm
