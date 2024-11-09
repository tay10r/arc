#pragma once

#include "NN_Module.h"

namespace NN {

class Linear final : public Module
{
public:
  Linear(uint16_t inFeatures, uint16_t outFeatures);

  [[nodiscard]] auto numWeights() const -> uint32_t override;

  void forward(const uint8_t* in, const uint8_t* weights, uint8_t* out) override;

private:
  uint16_t inFeatures_{};

  uint16_t outFeatures_{};
};

} // namespace NN
