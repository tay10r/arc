#pragma once

#include <stdint.h>

namespace NN {

struct Net;

/**
 * @brief The "Local Search" optimizer, based on "Derivative-Free Optimization of Neural Networks using Local Search".
 * */
class LSOptimizer final
{
public:
  using RngIntFunc = auto (*)(void*, int32_t minValue, int32_t maxValue) -> int32_t;

  using RngFloatFunc = auto (*)(void*, float minValue, float maxValue) -> float;

  using LossFunc = auto (*)(void*, const Net& net) -> float;

  /**
   * @brief Constructs a new optimizer object.
   *
   * @param net The neural network to optimize.
   *
   * @param batchSize The number of weights to update at a time.
   *                  Note that this has nothing to do with the input shape to the network.
   *
   * @param noiseMin The minimum amount of noise to add to each set of weights.
   *
   * @param noiseMax The maximum amount of noise to add to each set of weights.
   *
   * @param penalty The weight decay for each step.
   * */
  LSOptimizer(Net* net, uint32_t batchSize = 4, float noiseMin = -0.1F, float noiseMax = 0.1F, float penalty = 1.0e-3F);

  [[nodiscard]] auto allocMemory() -> bool;

  void releaseMemory();

  auto step(void* rngData, RngIntFunc rngInt, RngFloatFunc rngFloat, void* lossData, LossFunc loss) -> float;

  [[nodiscard]] auto getBestLoss() const -> float;

protected:
  void shuffleIndices(void* rngData, RngIntFunc rng);

private:
  Net* net_{};

  uint32_t batchSize_{};

  uint32_t batchIndex_{};

  float noiseMin_{};

  float noiseMax_{};

  float penalty_{};

  float bestLoss_{ static_cast<float>(1.0e6F) };

  float* bestParameters_{};

  uint32_t* indices_{};
};

} // namespace NN
