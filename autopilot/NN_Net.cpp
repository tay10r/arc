#include "NN_Net.h"

#include <assert.h>
#include <stdlib.h>

namespace NN {

void
Net::releaseMemory()
{
  free(parameters);
}

void
Net::randomize(void* rngData, auto(*rngFunc)(void*)->uint8_t)
{
  assert(parameters != nullptr);

  for (uint32_t i = 0; i < numParameters; i++) {
    parameters[i] = rngFunc(rngData);
  }
}

} // namespace NN
