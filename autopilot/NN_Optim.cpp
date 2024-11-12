#include "NN_Optim.h"

#include "NN_Net.h"

#include <stdlib.h>
#include <string.h>

namespace NN {

LSOptimizer::LSOptimizer(Net* net,
                         const uint32_t batchSize,
                         const float noiseMin,
                         const float noiseMax,
                         const float penalty)
  : net_(net)
  , batchSize_(batchSize)
  , noiseMin_(noiseMin)
  , noiseMax_(noiseMax)
  , penalty_(penalty)
{
}

auto
LSOptimizer::allocMemory() -> bool
{
  bestParameters_ = static_cast<float*>(malloc(net_->numParameters * sizeof(float)));
  if (!bestParameters_) {
    return false;
  }

  indices_ = static_cast<uint32_t*>(malloc(net_->numParameters * sizeof(uint32_t)));
  if (!indices_) {
    free(bestParameters_);
    bestParameters_ = nullptr;
    return false;
  }

  for (uint32_t i = 0; i < net_->numParameters; i++) {
    indices_[i] = i;
  }

  memcpy(bestParameters_, net_->parameters, net_->numParameters * sizeof(float));

  return true;
}

void
LSOptimizer::releaseMemory()
{
  free(bestParameters_);
  free(indices_);
  bestParameters_ = nullptr;
  indices_ = nullptr;
}

auto
LSOptimizer::getBestLoss() const -> float
{
  return bestLoss_;
}

auto
LSOptimizer::step(void* rngData, RngIntFunc rngInt, RngFloatFunc rngFloat, void* lossData, LossFunc loss) -> float
{
  if (batchIndex_ == 0) {
    shuffleIndices(rngData, rngInt);
  }

  const auto paramOffset = batchIndex_ * batchSize_;
  const auto paramEnd = paramOffset + batchSize_;
  const auto safeParamEnd = (paramEnd > net_->numParameters) ? net_->numParameters : paramEnd;

  for (auto i = paramOffset; i < safeParamEnd; i++) {
    const auto noise = rngFloat(rngData, noiseMin_, noiseMax_);
    const auto idx = indices_[i];
    net_->parameters[idx] += noise;
  }

  const auto l = loss(lossData, *net_);
  if (l < bestLoss_) {
    bestLoss_ = l;
    memcpy(bestParameters_, net_->parameters, net_->numParameters * sizeof(float));
  } else {
    // restore best model
    // TODO : find a faster way to do this
    memcpy(net_->parameters, bestParameters_, net_->numParameters * sizeof(float));
  }

  bestLoss_ += penalty_;

  batchIndex_++;

  if ((batchIndex_ * batchSize_) >= net_->numParameters) {
    batchIndex_ = 0;
  }

  return l;
}

void
LSOptimizer::shuffleIndices(void* rngData, RngIntFunc func)
{
  for (uint32_t i = 0; i < (net_->numParameters - 1); i++) {
    const auto j = func(rngData, i, net_->numParameters);
    const auto tmp = indices_[i];
    indices_[i] = indices_[j];
    indices_[j] = tmp;
  }
}

} // namespace NN
