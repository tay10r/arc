#pragma once

#include "AP_Time.h"

namespace SIM {

class Clock final : public AP::Clock
{
public:
  [[nodiscard]] auto now() -> uint32_t override;

  void step(uint32_t dt);

private:
  uint32_t time_{};
};

} // namespace SIM
