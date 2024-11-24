#include "AP_Program.h"
#include "AP_UbloxGPS.h"

#include <Wire.h>

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

AP::UbloxGPSSensor gpsSensor(&Wire);

} // namespace

void setup()
{
  SerialUSB.begin(115200);
  Wire.begin();
  program.setup(&SerialUSB, &clock, &gpsSensor);
}

void loop()
{
  program.loop();
}
