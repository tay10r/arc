#pragma once

#include "mavlink/common/mavlink.h"

#include <Stream.h>

#include <stdint.h>

namespace AP {

enum class MAVLinkBufferFlags : uint16_t
{
  kNone = 0,
  /**
   * @brief The buffer has a MAVLink message in it.
   */
  kUsed = 1,
  /**
   * @brief The buffer is currently being written out.
   */
  kActive = 2
};

class MAVLinkParser final
{
public:
  [[nodiscard]] auto read(Stream& stream) -> mavlink_message_t*;

protected:
private:
  mavlink_parse_state_t parser_{};

  mavlink_message_t rxMessage_{};

  mavlink_status_t rxStatus_{};
};

struct MAVLinkBuffer final
{
  uint8_t data[MAVLINK_MAX_PACKET_LEN];

  MAVLinkBufferFlags flags{};

  /**
   * @brief Where the next byte is that should be written.
   */
  uint16_t writeOffset{};

  /**
   * @brief The number of used bytes in the message.
   */
  uint16_t size{};

  [[nodiscard]] auto hasFlag(MAVLinkBufferFlags flags) const -> bool;
};

class MAVLinkBus final
{
public:
  void processOutput(Stream& stream);

  [[nodiscard]] auto send(const mavlink_message_t& msg) -> bool;

  [[nodiscard]] auto readyToSend() const -> bool;

private:
  MAVLinkBuffer buffers_[2]{};
};

class MAVLinkComponent
{
public:
  virtual ~MAVLinkComponent() = default;

  virtual void loop(MAVLinkBus& bus, uint32_t timeDelta) = 0;

  virtual void recv(const mavlink_message_t& msg);
};

} // namespace AP
