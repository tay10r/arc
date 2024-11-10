#include "NN_Optim.h"

#include "NN_Net.h"

#include <stdlib.h>
#include <string.h>

namespace NN {

LSOptimizer::LSOptimizer(Net* net,
                         const uint32_t batchSize,
                         const int8_t noiseMin,
                         const int8_t noiseMax,
                         const uint8_t penalty)
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
  bestParameters_ = static_cast<uint8_t*>(malloc(net_->numParameters));
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

  memcpy(bestParameters_, net_->parameters, net_->numParameters);

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

namespace {

auto
clamp(int32_t x) -> int32_t
{
  x = (x < 0) ? 0 : x;
  x = (x > 255) ? 255 : x;
  return x;
}

} // namespace

auto
LSOptimizer::step(void* rngData, RngFunc rng, void* lossData, LossFunc loss) -> uint32_t
{
  if (batchIndex_ == 0) {
    shuffleIndices(rngData, rng);
  }

  const auto paramOffset = batchIndex_ * batchSize_;

  const auto paramEnd = paramOffset + batchSize_;

  const auto safeParamEnd = (paramEnd > net_->numParameters) ? net_->numParameters : paramEnd;

  for (auto i = paramOffset; i < safeParamEnd; i++) {
    const auto noise = rng(rngData, static_cast<int32_t>(noiseMin_), static_cast<int32_t>(noiseMax_) + 1);
    const auto idx = indices_[i];
    const auto param = static_cast<int32_t>(net_->parameters[idx]) + noise;
    net_->parameters[idx] = static_cast<uint8_t>(clamp(param));
  }

  const auto l = loss(lossData, *net_);
  if (l < bestLoss_) {
    bestLoss_ = l;
    memcpy(bestParameters_, net_->parameters, net_->numParameters);
  } else {
    // restore best model
    // TODO : find a faster way to do this
    memcpy(net_->parameters, bestParameters_, net_->numParameters);
  }

  bestLoss_ += penalty_;

  batchIndex_++;

  if ((batchIndex_ * batchSize_) >= net_->numParameters) {
    batchIndex_ = 0;
  }

  return l;
}

void
LSOptimizer::shuffleIndices(void* rngData, RngFunc func)
{
  for (uint32_t i = 0; i < (net_->numParameters - 1); i++) {
    const auto j = func(rngData, i, net_->numParameters);
    const auto tmp = indices_[i];
    indices_[i] = indices_[j];
    indices_[j] = tmp;
  }
}

} // namespace NN
