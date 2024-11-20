#pragma once

#include <stdint.h>

namespace AP {

class Magnetometer
{
public:
  struct Sample final
  {
    float xyz[3];
  };

  virtual ~Magnetometer() = default;

  [[nodiscard]] virtual auto setup() -> bool = 0;

  [[nodiscard]] virtual auto read(Sample* sample) -> bool = 0;

protected:
  /**
   * @brief Converts the sensor-specific range to a reading in Gauss.
   * */
  static void toSample(const int16_t* value, float rangeMin, float rangeMax, Sample* sample);
};

} // namespace AP
