#include "AP_Magnetometer.h"

namespace AP {

void
Magnetometer::toSample(const int16_t* values, const float rangeMin, const float rangeMax, Sample* sample)
{
  for (int i = 0; i < 3; i++) {
    const float rangeScale = rangeMax - rangeMin;
    const float normalizedValue = static_cast<float>(values[i]) / 32768.0F;
    sample->xyz[i] = normalizedValue * rangeScale + rangeMin;
  }
}

} // namespace AP
