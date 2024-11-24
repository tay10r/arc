#include <Wire.h>

void
TwoWire::beginTransmission(uint8_t)
{
}

void
TwoWire::endTransmission()
{
}

auto
TwoWire::requestFrom(uint8_t, uint8_t) -> uint8_t
{
  return 0;
}
