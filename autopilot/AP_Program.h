#pragma once

#include <Arduino.h>

#include "AP_GPS.h"
#include "AP_Heartbeat.h"
#include "AP_Mavlink.h"
#include "AP_Time.h"

namespace AP {

/**
 * @brief This is the top level class containing the entire autopilot program.
 */
class Program final
{
public:
  /**
   * @brief Initializes the autopilot program.
   *
   * @param mavlink_stream The stream where MAVLink traffic should be routed through.
   *
   * @param clock For keeping track of time.
   *
   * @param gpsSensor A GPS sensor for indicating the position and velocity of the vehicle.
   */
  void setup(Stream* mavlink_stream, Clock* clock, GPSSensor* gpsSensor);

  void loop();

protected:
  void forwardToComponents(const mavlink_message_t& msg);

private:
  /**
   * @brief The stream to route MAVLink traffic through.
   */
  Stream* mavlinkStream_{};

  /**
   * @brief For reading MAVLink messages from the serial bus.
   */
  MAVLinkParser mavlinkParser_{};

  /**
   * @brief For sending MAVLink messages out over the serial stream.
   */
  MAVLinkBus mavlinkBus_{};

  /**
   * @brief For keeping track of time.
   */
  Clock* clock_{};

  /**
   * @brief For measuring the time between ticks.
   */
  Stopwatch stopwatch_;

  /**
   * @brief The component for sending heartbeats out.
   */
  HeartbeatComponent heartbeat_;

  /**
   * @brief The MAVLink component for GPS.
   * */
  GPSComponent gpsComponent_;
};

} // namespace AP
