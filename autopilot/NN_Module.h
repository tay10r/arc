#pragma once

#include <stdint.h>

namespace NN {

class Module
{
public:
  virtual ~Module() = default;

  [[nodiscard]] virtual auto numWeights() const -> uint32_t = 0;

  virtual void forward(const uint8_t* input, const uint8_t* weights, uint8_t* output) = 0;
};

} // namespace NN
