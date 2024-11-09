#include "AP_Time.h"

namespace AP {

void
Stopwatch::begin(Clock& clk)
{
  lastTimestamp_ = clk.now();
}

auto
Stopwatch::getElapsed(Clock& clk) const -> uint32_t
{
  const auto t = clk.now();
  if (t < lastTimestamp_) {
    // Probably overflow
    uint32_t v{};
    return ((~v) - lastTimestamp_) + t;
  } else {
    return t - lastTimestamp_;
  }
}

Timer::Timer(const uint32_t interval)
  : interval_(interval)
{
}

auto
Timer::step(const uint32_t timeDelta) -> uint32_t
{
  elapsed_ += timeDelta;
  const auto ticks = elapsed_ / interval_;
  elapsed_ -= ticks * interval_;
  return ticks;
}

void
Timer::reset()
{
  elapsed_ = 0;
}

} // namespace AP
