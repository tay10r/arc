#include "NN_ReLU.h"

namespace NN {

ReLU::ReLU(const uint32_t features, const uint8_t cutoff)
  : numFeatures_(features)
  , cutoff_(cutoff)
{
}

auto
ReLU::numWeights() const -> uint32_t
{
  return 0;
}

void
ReLU::forward(const uint8_t* input, const uint8_t* weights, uint8_t* output)
{
  for (uint32_t i = 0; i < numFeatures_; i++) {
    const auto in = input[i];
    output[i] = (in > cutoff_) ? in : cutoff_;
  }
}

} // namespace NN
