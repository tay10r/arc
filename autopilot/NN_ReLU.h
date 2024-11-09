#pragma once

#include "NN_Module.h"

namespace NN {

class ReLU final : public Module
{
public:
  explicit ReLU(uint32_t features, uint8_t cutoff = 128);

  [[nodiscard]] auto numWeights() const -> uint32_t override;

  void forward(const uint8_t* input, const uint8_t* weights, uint8_t* output) override;

private:
  uint16_t numFeatures_{};

  uint8_t cutoff_{};
};

} // namespace NN
