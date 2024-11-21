#pragma once

#include "AP_Magnetometer.h"

namespace AP {

class MMC5983MA final : public Magnetometer
{
public:
  [[nodiscard]] auto setup() -> bool override;

  [[nodiscard]] auto read(Sample* sample) -> bool override;
};

} // namespace AP
