#pragma once

#include <vector>
#include <memory>

#include <cstdint>

class SerialPort
{
public:
  static auto createFromHw(const char* name) -> std::unique_ptr<SerialPort>;

  virtual ~SerialPort() = default;

  virtual void hostWrite(uint8_t value) = 0;

  virtual auto hostRead() -> std::vector<uint8_t> = 0;
};
