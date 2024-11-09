#pragma once

#include <stdint.h>

namespace AP {

class Clock
{
public:
  virtual ~Clock() = default;

  /**
   * @brief Gets the time since boot, in terms of microseconds.
   */
  virtual auto now() -> uint32_t = 0;
};

class Stopwatch final
{
public:
  void begin(Clock& clk);

  [[nodiscard]] auto getElapsed(Clock& clk) const -> uint32_t;

private:
  uint32_t lastTimestamp_{};
};

class Timer final
{
public:
  explicit Timer(uint32_t interval);

  [[nodiscard]] auto step(uint32_t timeDelta) -> uint32_t;

  /**
   * @brief Starts the timer from zero.
   */
  void reset();

private:
  uint32_t interval_{};

  uint32_t elapsed_{};
};

} // namespace AP
