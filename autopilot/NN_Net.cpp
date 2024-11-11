#include "NN_Net.h"

#include <assert.h>
#include <stdlib.h>

namespace NN {

auto
Net::allocMemory() -> bool
{
  size_t allocSize = numParameters * sizeof(float);
  for (uint8_t i = 0; i < NN_MAX_REGS; i++) {
    allocSize += regSizes[i] * sizeof(float);
  }
  auto* memory = static_cast<float*>(malloc(allocSize));
  if (!memory) {
    return false;
  }
  parameters = memory;
  memory += numParameters;
  for (uint8_t i = 0; i < NN_MAX_REGS; i++) {
    regs[i] = memory;
    memory += regSizes[i];
  }
  return true;
}

void
Net::releaseMemory()
{
  free(parameters);
}

void
Net::randomize(void* rngData, auto(*rngFunc)(void*)->float)
{
  assert(parameters != nullptr);

  for (uint32_t i = 0; i < numParameters; i++) {
    parameters[i] = rngFunc(rngData);
  }
}

} // namespace NN
