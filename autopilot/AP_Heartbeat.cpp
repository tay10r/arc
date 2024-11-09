#include "AP_Heartbeat.h"

namespace AP {

void
HeartbeatComponent::loop(MAVLinkBus& bus, const uint32_t timeDelta)
{
  if (timer_.step(timeDelta) > 0) {
    heartbeatDue_ = true;
  }

  if (heartbeatDue_ && bus.readyToSend()) {
    mavlink_heartbeat_t payload{};
    payload.type = MAV_TYPE_GENERIC;
    payload.autopilot = MAV_AUTOPILOT_GENERIC;
    payload.base_mode = MAV_MODE_AUTO_DISARMED;
    payload.custom_mode = 0;
    payload.system_status = MAV_STATE_ACTIVE;
    mavlink_message_t msg{};
    mavlink_msg_heartbeat_encode(/*system_id=*/1, MAV_COMP_ID_AUTOPILOT1, &msg, &payload);
    // Keep the heartbeat due flag if the message sending fails.
    heartbeatDue_ = !bus.send(msg);
    if (!heartbeatDue_) {
      // If the message sending was successful, restart the timer.
      timer_.reset();
    }
  }
}

} // namespace AP
