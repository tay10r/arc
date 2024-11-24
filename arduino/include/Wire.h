#pragma once

#include "Stream.h"

class TwoWire : public Stream
{
public:
  /**
   * @brief Begins buffering a new transmission.
   *
   * @param address The device to send the transmission to.
   * */
  void beginTransmission(uint8_t address);

  /**
   * @brief This will flush the send buffer to the target device.
   *
   * @note This function may block.
   * */
  void endTransmission();

  /**
   * @brief Requests to read data from a specific device.
   *
   * @param address The address of the device to read from.
   *
   * @param len The number of bytes to read.
   *
   * @return The number of bytes that were read from the device.
   *
   * @note This function may block.
   * */
  [[nodiscard]] auto requestFrom(uint8_t address, uint8_t len) -> uint8_t;
};

