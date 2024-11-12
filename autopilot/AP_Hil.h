#pragma once

#include "AP_Mavlink.h"

namespace AP {

class HilActuatorControls final : public MAVLinkComponent
{
public:
  void loop(MAVLinkBus& bus, uint32_t timeDelta) override;

  void recv(const mavlink_message_t& msg) override;
};

} // namespace AP
