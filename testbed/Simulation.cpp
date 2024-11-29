#include "Simulation.h"

#include <AP_Program.h>

#include <array>

#include <cstring>

namespace {

class FakeStream final : public Stream
{
public:
  [[nodiscard]] auto availableForWrite() -> int override { return txBuffer_.size() - txOffset_; }

  [[nodiscard]] auto write(const uint8_t value) -> size_t override
  {
    if (availableForWrite() > 0) {
      txBuffer_[txOffset_] = value;
      txOffset_++;
      return 1;
    }
    return 0;
  }

  [[nodiscard]] auto available() -> int override { return rxOffset_; }

  [[nodiscard]] auto read() -> int override
  {
    if (available() > 0) {
      const auto value = rxBuffer_[rxOffset_];
      rxOffset_--;
      return static_cast<int>(value);
    }
    //
    return -1;
  }

  [[nodiscard]] auto hostWrite(const uint8_t value) -> size_t
  {
    if (rxSize_ < rxBuffer_.size()) {
      rxSize_++;
      return 1;
    }
    return 0;
  }

  [[nodiscard]] auto hostRead() -> std::vector<std::uint8_t>
  {
    std::vector<std::uint8_t> buffer(txOffset_);
    for (auto i = 0u; i < txOffset_; i++) {
      buffer[i] = txBuffer_[i];
    }
    txOffset_ = 0;
    return buffer;
  }

  void step()
  {
    // Clear the data that has already been read and make room for more.
    std::memmove(rxBuffer_.data(), rxBuffer_.data() + rxOffset_, rxSize_ - rxOffset_);
    rxOffset_ = 0;
    rxSize_ -= rxOffset_;
  }

private:
  std::array<std::uint8_t, 4> txBuffer_;

  std::array<std::uint8_t, 4> rxBuffer_;

  std::size_t txOffset_{};

  std::size_t rxOffset_{};

  std::size_t rxSize_{};
};

class FakeClock final : public AP::Clock
{
public:
  [[nodiscard]] auto now() -> uint32_t override { return time_; }

  void step(const float timeDelta)
  {
    const auto microSeconds = static_cast<int>(timeDelta * 1.0e6F);
    time_ += static_cast<uint32_t>(microSeconds);
  }

private:
  uint32_t time_;
};

class SimulationImpl final : public Simulation
{
public:
  explicit SimulationImpl(std::shared_ptr<sim::Agent> agent)
    : agent_(std::move(agent))
  {
  }

  void setup() override { program_.setup(&fakeStream_, &fakeClock_, AP::GPSSensor::null()); }

  void loop(const float timeDelta) override
  {
    fakeClock_.step(timeDelta);

    program_.loop();

    const auto readBuffer = fakeStream_.hostRead();
    for (const auto c : readBuffer) {
      if (mavlink_parse_char(MAVLINK_COMM_0, c, &message_, &status_) == 1) {
        interpretMessage(message_);
      }
    }

    fakeStream_.step();
  }

protected:
  void interpret(const mavlink_message_t& /*msg*/, const mavlink_heartbeat_t& /*payload*/)
  {
    //
  }

  void interpret(const mavlink_message_t& /*msg*/, const mavlink_actuator_output_status_t& payload)
  {
    //
  }

  void interpretMessage(const mavlink_message_t& msg)
  {
    switch (msg.msgid) {
      case MAVLINK_MSG_ID_HEARTBEAT: {
        mavlink_heartbeat_t payload{};
        mavlink_msg_heartbeat_decode(&msg, &payload);
        interpret(msg, payload);
      } break;
      case MAVLINK_MSG_ID_ACTUATOR_OUTPUT_STATUS: {
        mavlink_actuator_output_status_t payload{};
        mavlink_msg_actuator_output_status_decode(&msg, &payload);
        interpret(msg, payload);
      } break;
    }
    //
  }

private:
  std::shared_ptr<sim::Agent> agent_;

  FakeStream fakeStream_;

  FakeClock fakeClock_;

  AP::Program program_;

  mavlink_parse_state_t parser_;

  mavlink_message_t message_;

  mavlink_status_t status_;
};

} // namespace

auto
Simulation::create(std::shared_ptr<sim::Agent> agent) -> std::unique_ptr<Simulation>
{
  return std::make_unique<SimulationImpl>(std::move(agent));
}
