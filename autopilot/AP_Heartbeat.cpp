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
    payload.base_mode = MAV_MODE_GUIDED_ARMED;
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

#if 0
    {
      mavlink_global_position_int_t payload{};
      payload.lat = static_cast<int32_t>(41.4633 * 1.0e7);
      payload.lon = static_cast<int32_t>(-71.9571 * 1.0e7);
      mavlink_message_t msg{};
      mavlink_msg_global_position_int_encode(/*system_id=*/1, MAV_COMP_ID_AUTOPILOT1, &msg, &payload);
      bus.send(msg);
    }
#endif
  }
}

} // namespace AP
