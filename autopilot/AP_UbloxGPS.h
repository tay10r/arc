#pragma once

#include "AP_GPS.h"

#include <Wire.h>

namespace AP {

class UbloxGPSSensor final : public GPSSensor
{
public:
  /**
   * @brief Constructs an interface to the sensor.
   *
   * @param bus The I2C bus that the GPS sensor is connected to.
   *
   * @param address The I2C address that the GPS sensor uses.
   * */
  UbloxGPSSensor(TwoWire* bus, uint8_t address = 0x42);

  /**
   * @brief Attempts to read from the device.
   *
   * @return True if at least one NMEA message was parsed, false otherwise.
   * */
  [[nodiscard]] auto read() -> bool override;

private:
  /**
   * @brief The bus used to interface with the device.
   * */
  TwoWire* bus_{};

  /**
   * @brief The address that the GPS sensor is associated with.
   * */
  uint8_t address_{};
};

} // namespace AP
