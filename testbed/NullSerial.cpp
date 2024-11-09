#include "Serial.h"

namespace {

class NullSerialPort final : public SerialPort
{
public:
  void hostWrite(uint8_t value) override
  {
  }

  auto hostRead() -> std::vector<uint8_t> override
  {
    std::vector<uint8_t> buffer;
    return buffer;
  }
};

} // namespace

auto SerialPort::createFromHw(const char*) -> std::unique_ptr<SerialPort>
{
  return std::make_unique<NullSerialPort>();
}
