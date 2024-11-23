#include "SIM_Clock.h"

namespace SIM {

auto
Clock::now() -> uint32_t
{
  return time_;
}

void
Clock::step(uint32_t dt)
{
  time_ += dt;
}

} // namespace SIM
