#include "AP_Program.h"

namespace {

class ClockImpl final : public AP::Clock
{
public:
  auto now() -> uint32_t override
  {
    return micros();
  }
};

AP::Program program;

ClockImpl clock;

} // namespace

void setup()
{
  program.setup(&Serial, &clock);
}

void loop()
{
  program.loop();
}
