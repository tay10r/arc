#include "AP_UbloxGPS.h"

namespace AP {

UbloxGPSSensor::UbloxGPSSensor(TwoWire* bus, const uint8_t address)
  : bus_(bus)
  , address_(address)
{
}

auto
UbloxGPSSensor::read() -> bool
{
  auto success{ false };

  const uint8_t maxReads{ 255 };

  for (uint8_t i = 0; i < maxReads; i++) {

    // Set the read address
    bus_->beginTransmission(address_);
    (void)bus_->write(0xff);
    bus_->endTransmission();

    // Read the NMEA data, if available.
    if (bus_->requestFrom(address_, 1) == 0) {
      break;
    }
    const auto value = bus_->read();
    if ((value < 0) || (value == 0xff)) {
      break;
    }
    const char buf{ static_cast<char>(value) };
    success |= parseData(&buf, 1);
    // If we successfully read a byte, try to read another.
  }

  return success;
}

} // namespace AP
