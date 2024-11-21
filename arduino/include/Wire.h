#pragma once

#include "Stream.h"

class TwoWire : public Stream
{
public:
  void beginTransmission(uint8_t address);

  void endTransmission();
};

