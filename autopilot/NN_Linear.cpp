#include "NN_Linear.h"

#include <assert.h>

namespace NN {

Linear::Linear(uint16_t inFeatures, uint16_t outFeatures)
  : inFeatures_(inFeatures)
  , outFeatures_(outFeatures)
{
}

auto
Linear::numWeights() const -> uint32_t
{
  return inFeatures_ * outFeatures_;
}

void
Linear::forward(const uint8_t* input, const uint8_t* weights, uint8_t* output)
{
  for (uint32_t i = 0; i < static_cast<uint32_t>(outFeatures_); i++) {

    uint32_t out{};

    for (uint32_t j = 0; j < static_cast<uint32_t>(inFeatures_); j++) {
      const auto v =
        static_cast<uint16_t>(input[j]) * static_cast<uint16_t>(weights[i * static_cast<uint32_t>(inFeatures_) + j]);
      out += static_cast<uint32_t>(v);
    }

    out <<= 8 + (inFeatures_);

    assert(out < 256);

    output[i] = static_cast<uint8_t>(out);
  }
}

} // namespace NN
