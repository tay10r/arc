#pragma once

#include "Stream.h"

class HardwareSerial final : public Stream
{
public:
  ~HardwareSerial() override = default;

  void begin(unsigned long baud);
};

extern HardwareSerial Serial;
