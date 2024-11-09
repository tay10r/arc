#pragma once

#include "AP_Mavlink.h"

#include "AP_Time.h"

namespace AP {

class HeartbeatComponent final : public MAVLinkComponent
{
public:
  void loop(MAVLinkBus& bus, uint32_t timeDelta) override;

private:
  Timer timer_{ 1000000ul };

  bool heartbeatDue_{ false };
};

} // namespace AP
