#pragma once

#include <stdint.h>

#define NN_MAX_REGS 16

namespace NN {

struct Net final
{
  uint32_t numParameters{};

  uint16_t regSizes[NN_MAX_REGS]{};

  uint8_t* parameters{};

  uint8_t* regs[NN_MAX_REGS]{};

  /**
   * @brief Releases memory allocated by the registers and parameters.
   * */
  void releaseMemory();

  /**
   * @brief Randomizes the parameters.
   * */
  void randomize(void* rngData, auto(*rngFunc)(void*)->uint8_t);
};

} // namespace NN
