#pragma once

#include <stdint.h>

#define NN_MAX_REGS 16

namespace NN {

struct Net final
{
  uint32_t numParameters{};

  uint16_t regSizes[NN_MAX_REGS]{};

  float* parameters{};

  float* regs[NN_MAX_REGS]{};

  /**
   * @brief Attempts to allocate the parameters and registers in the network.
   *
   * @return True on success, false on failure.
   * */
  [[nodiscard]] auto allocMemory() -> bool;

  /**
   * @brief Releases memory allocated by the registers and parameters.
   * */
  void releaseMemory();

  /**
   * @brief Randomizes the parameters.
   * */
  void randomize(void* rngData, auto(*rngFunc)(void*)->float);
};

} // namespace NN
