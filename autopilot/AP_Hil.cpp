#include "AP_Hil.h"

namespace AP {

void
HilActuatorControls::loop(MAVLinkBus& bus, uint32_t timeDelta)
{
  (void)bus;
  (void)timeDelta;
}

void
HilActuatorControls::recv(const mavlink_message_t& msg)
{
  (void)msg;
}

} // namespace AP
