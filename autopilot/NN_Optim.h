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
  using RngFunc = auto (*)(void*, int32_t minValue, int32_t maxValue) -> int32_t;

  using LossFunc = auto (*)(void*, const Net& net) -> uint32_t;

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
  LSOptimizer(Net* net, uint32_t batchSize = 2, int8_t noiseMin = -16, int8_t noiseMax = 16, uint8_t penalty = 1);

  [[nodiscard]] auto allocMemory() -> bool;

  void releaseMemory();

  auto step(void* rngData, RngFunc rng, void* lossData, LossFunc loss) -> uint32_t;

protected:
  void shuffleIndices(void* rngData, RngFunc rng);

  void addNoise();

private:
  Net* net_{};

  uint32_t batchSize_{};

  uint32_t batchIndex_{};

  int8_t noiseMin_{};

  int8_t noiseMax_{};

  uint8_t penalty_{};

  uint32_t bestLoss_{ static_cast<uint32_t>(~0ul) };

  uint8_t* bestParameters_{};

  uint32_t* indices_{};
};

} // namespace NN
