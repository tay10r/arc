#include "AP_WGS84.h"

#include <math.h>

#define PI 3.141592653589793238F

namespace AP {

WGS84Ref::WGS84Ref(const float lat, const float lon)
  : refLat_(lat)
  , refLon_(lon)
{
  const auto radLat = (lat / 180.0F) * PI;
  metersPerDegree_[0] = 111132.92F - 559.82 * cosf(2 * radLat) + 1.175 * cosf(4 * radLat) - 0.0023F * cosf(6 * radLat);
  metersPerDegree_[1] = 111412.84F * cosf(radLat) - 93.5F * cosf(3 * radLat) + 0.118F * cosf(5 * radLat);
}

auto
WGS84Ref::toLatLon(const Vec2f& in) const -> Vec2f
{
  const auto deltaLat = in[0] / metersPerDegree_[0];
  const auto deltaLon = in[1] / metersPerDegree_[1];
  return { refLat_ + deltaLat, refLon_ + deltaLon };
}

} // namespace AP
