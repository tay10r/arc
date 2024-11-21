#include "AP_Mavlink.h"

namespace AP {

auto
MAVLinkParser::read(Stream& stream) -> mavlink_message_t*
{
  while (stream.available() > 0) {
    auto rxValue = stream.read();
    if (rxValue < 0) {
      break;
    }

    if (mavlink_parse_char(MAVLINK_COMM_0, static_cast<uint8_t>(rxValue), &rxMessage_, &rxStatus_) == 1) {
      return &rxMessage_;
    }
  }

  return nullptr;
}

auto
MAVLinkBuffer::hasFlag(const MAVLinkBufferFlags flag) const -> bool
{
  return (static_cast<uint8_t>(flags) & static_cast<uint8_t>(flag)) == static_cast<uint8_t>(flag);
}

void
MAVLinkBus::processOutput(Stream& stream)
{
  MAVLinkBuffer* activeBuffer{};

  const auto numBuffers = sizeof(buffers_) / sizeof(buffers_[0]);

  for (auto i = 0u; i < numBuffers; i++) {
    if (buffers_[i].hasFlag(MAVLinkBufferFlags::kActive)) {
      activeBuffer = &buffers_[i];
      break;
    }
  }

  if (!activeBuffer) {
    // Find a buffer with a message in it and make it active.
    for (auto i = 0u; i < numBuffers; i++) {
      if (buffers_[i].hasFlag(MAVLinkBufferFlags::kUsed)) {
        activeBuffer = &buffers_[i];
        activeBuffer->flags = MAVLinkBufferFlags::kActive;
        break;
      }
    }
  }

  if (activeBuffer) {
    while ((activeBuffer->writeOffset < activeBuffer->size) && (stream.availableForWrite() > 0)) {
      const auto c = activeBuffer->data[activeBuffer->writeOffset];
      const auto writeSize = stream.write(c);
      if (!writeSize) {
        break;
      }
      activeBuffer->writeOffset += writeSize;
    }
    if (activeBuffer->writeOffset >= activeBuffer->size) {
      activeBuffer->writeOffset = 0;
      activeBuffer->size = 0;
      activeBuffer->flags = MAVLinkBufferFlags::kNone;
    }
  }
}

auto
MAVLinkBus::send(const mavlink_message_t& msg) -> bool
{
  const auto numBuffers = sizeof(buffers_) / sizeof(buffers_[0]);
  for (auto i = 0u; i < numBuffers; i++) {
    if (buffers_[i].flags != MAVLinkBufferFlags::kNone) {
      continue;
    }
    buffers_[i].size = mavlink_msg_to_send_buffer(buffers_[i].data, &msg);
    buffers_[i].writeOffset = 0;
    buffers_[i].flags = MAVLinkBufferFlags::kUsed;
    return true;
  }
  return false;
}

auto
MAVLinkBus::readyToSend() const -> bool
{
  const auto numBuffers = sizeof(buffers_) / sizeof(buffers_[0]);
  for (auto i = 0u; i < numBuffers; i++) {
    if (buffers_[i].flags == MAVLinkBufferFlags::kNone) {
      // We found at least one empty buffer, which means we can send a message.
      return true;
    }
  }
  return false;
}

void
MAVLinkComponent::recv(const mavlink_message_t&)
{
}

} // namespace AP
