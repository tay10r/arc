#pragma once

#include "AP_LinAlg.h"

namespace AP {

class WGS84Ref final
{
public:
  WGS84Ref(float lat, float lon);

  [[nodiscard]] auto toLatLon(const Vec2f& in) const -> Vec2f;

private:
  float refLat_;

  float refLon_;

  float metersPerDegree_[2];
};

} // namespace AP
