#include "../Serial.h"

#include <Windows.h>

namespace {

class WindowsSerialPort final : public SerialPort
{
public:
  WindowsSerialPort(const char* name)
  {
  }

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

auto SerialPort::createFromHw(const char* name) -> std::unique_ptr<SerialPort>
{
  return std::make_unique<WindowsSerialPort>(name);
}
