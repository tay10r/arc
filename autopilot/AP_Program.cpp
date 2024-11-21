#include "AP_Program.h"

namespace AP {

void
Program::setup(Stream* stream, Clock* clock)
{
  mavlinkStream_ = stream;
  clock_ = clock;
  stopwatch_.begin(*clock_);
}

void
Program::loop()
{
  const auto timeDelta = stopwatch_.getElapsed(*clock_);

  stopwatch_.begin(*clock_);

  mavlinkBus_.processOutput(*mavlinkStream_);

  while (true) {

    auto* msg = mavlinkParser_.read(*mavlinkStream_);
    if (!msg) {
      break;
    }

    forwardToComponents(*msg);
  }

  heartbeat_.loop(mavlinkBus_, timeDelta);
}

void
Program::forwardToComponents(const mavlink_message_t& msg)
{
  heartbeat_.recv(msg);
}

} // namespace AP
