#pragma once

#include "SIM_GPS.h"

namespace SIM {

class Vehicle
{
public:
  virtual ~Vehicle() = default;

  auto getGPS() -> GPSSensor&;

private:
  GPSSensor gpsSensor_;
};

} // namespace SIM
