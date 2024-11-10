#pragma once

#include <stdint.h>

namespace NN {

[[nodiscard]] auto
l1Loss(const uint8_t* predicted, const uint8_t* target, const uint16_t size) -> uint32_t;

} // namespace NN
