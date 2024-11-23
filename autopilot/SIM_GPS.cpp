#include "SIM_GPS.h"

namespace SIM {

GPSSensor::GPSSensor(uint32_t seed)
  : random_(seed)
  , ref_(/*lat=*/0, /*lon=*/0)
{
}

void
GPSSensor::step(const uint32_t timeDelta)
{
  sentenceDue_ |= sampleTimer_.step(timeDelta) > 0;
}

void
GPSSensor::setOrigin(const float lat, const float lon)
{
  ref_ = AP::WGS84Ref(lat, lon);
}

void
GPSSensor::setOffset(const float x, const float y)
{
  xOffset_ = x;
  yOffset_ = y;
}

auto
GPSSensor::read() -> bool
{
  if (!sentenceDue_) {
    return false;
  }

  const auto xDelta = (random_.randint(1024) / 512.0F) - 1.0F;
  const auto yDelta = (random_.randint(1024) / 512.0F) - 1.0F;

  const auto latLon = ref_.toLatLon(AP::Vec2f{ xOffset_ + xDelta, yOffset_ + yDelta });

  GGA gga;

  gga.lat = latLon[0];
  gga.lon = latLon[1];
  gga.alt = 0;
  gga.geoidSeparation = 0;
  gga.hasFix = true;
  gga.numSatellites = 4;
  gga.timeOfDay = 0;

  notifyGGA(gga);

  sentenceDue_ = false;

  return true;
}

} // namespace SIM
