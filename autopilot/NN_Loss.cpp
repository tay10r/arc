#include "NN_Loss.h"

namespace NN {

auto
l1Loss(const float* predicted, const float* target, const uint16_t size) -> float
{
  float loss{};

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

auto
mseLoss(const float* predicted, const float* target, const uint16_t size) -> float
{
  float loss{};

  for (uint16_t i = 0; i < size; i++) {
    const auto delta = target[i] - predicted[i];
    loss += delta * delta;
  }

  return loss / size;
}

} // namespace NN
