
#include "stmepic.hpp"
#include "random_number_generator.hpp"

using namespace stmepic::algorithm;
using namespace stmepic;


RandomNumberGenerator::RandomNumberGenerator() {
#ifdef HAL_RNG_MODULE_ENABLED
  hrng = nullptr;
#endif
}

RandomNumberGenerator &RandomNumberGenerator::get_instance() {
  static RandomNumberGenerator instance;
  return instance;
}

#ifdef HAL_RNG_MODULE_ENABLED
Status RandomNumberGenerator::init(RNG_HandleTypeDef &hrng) {
  this->hrng = &hrng;
  if(HAL_RNG_Init(this->hrng) != HAL_OK) {
    this->hrng = nullptr;
    return Status::HalError("Failed to initialize RNG peripheral");
  }
  return Status::OK();
}
#endif

Status RandomNumberGenerator::init(uint32_t seed) {
  std::srand(seed);
  return Status::OK();
}

uint32_t RandomNumberGenerator::random() {
#ifdef HAL_RNG_MODULE_ENABLED
  if(hrng != nullptr) {
    uint32_t random_number;
    if(HAL_RNG_GenerateRandomNumber(hrng, &random_number) != HAL_OK) {
      return 0;
    }
    return random_number;
  } else {
    // RNG not initialized, fallback to software RNG
    return (uint32_t)std::rand();
  }
#else
  return (uint32_t)std::rand();
#endif
}