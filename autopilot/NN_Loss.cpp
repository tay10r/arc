#include "NN_Loss.h"

namespace NN {

[[nodiscard]] auto
l1Loss(const uint8_t* predicted, const uint8_t* target, const uint16_t size) -> uint32_t
{
  uint32_t loss{};

  for (uint16_t i = 0; i < size; i++) {
    const auto p = predicted[i];
    const auto t = target[i];
    if (p > t) {
      loss += p - t;
    } else {
      loss += t - p;
    }
  }

  return loss;
}

} // namespace NN
