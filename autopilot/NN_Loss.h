#pragma once

#include <stdint.h>

namespace NN {

[[nodiscard]] auto
l1Loss(const float* predicted, const float* target, const uint16_t size) -> float;

[[nodiscard]] auto
mseLoss(const float* predicted, const float* target, const uint16_t size) -> float;

} // namespace NN
