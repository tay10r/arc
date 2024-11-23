#pragma once

#include <stdint.h>

namespace AP {

class Random final
{
public:
  Random(uint32_t seed = 0);

  auto operator()() -> uint32_t;

  auto randint(uint32_t max) -> uint32_t;

private:
  uint32_t x_{};
};

} // namespace AP
