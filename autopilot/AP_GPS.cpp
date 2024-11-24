#include "AP_GPS.h"

#include <stdlib.h>
#include <string.h>

namespace AP {

namespace {

class NullGPS final : public GPSSensor
{
public:
  [[nodiscard]] auto read() -> bool override { return false; }
};

} // namespace

auto
GPSSensor::null() -> GPSSensor*
{
  static NullGPS instance;
  return &instance;
}

void
GPSSensor::setup(void* userData, GGA_Callback ggaFunc, VTG_Callback vtgFunc)
{
  userData_ = userData;
  ggaFunc_ = ggaFunc;
  vtgFunc_ = vtgFunc;
}

auto
GPSSensor::parseData(const char* buffer, const uint8_t size) -> bool
{
  auto gotMessage{ false };

  for (uint8_t i = 0; i < size; i++) {
    gotMessage |= parser_.write(buffer[i]);
  }

  return gotMessage;
}

void
GPSSensor::onMessageBegin()
{
}

void
GPSSensor::onMessageEnd(const bool checksumPassed)
{
  if (!checksumPassed) {
    return;
  }

  switch (type_) {
    case Type::kOther:
      break;
    case Type::kGGA:
      notifyGGA(gga_);
      break;
    case Type::kVTG:
      notifyVTG(vtg_);
      break;
  }
}

void
GPSSensor::onTalker(const char*, uint8_t)
{
}

void
GPSSensor::onType(const char* type, const uint8_t)
{
  if (strcmp(type, "GGA") == 0) {
    type_ = Type::kGGA;
  } else if (strcmp(type, "VTG") == 0) {
    type_ = Type::kVTG;
  } else {
    type_ = Type::kOther;
  }
}

void
GPSSensor::onField(const char* field, const uint8_t fieldSize, const uint8_t fieldIndex)
{
  switch (type_) {
    case Type::kOther:
      break;
    case Type::kVTG:
      break;
    case Type::kGGA:
      onGGAField(field, fieldSize, fieldIndex);
      break;
  }
}

namespace {

auto
parseDegreeMinutes(const char* str, const uint8_t len, const uint8_t degDigits) -> float
{
  if (len != (7 + degDigits)) {
    return false;
  }

  auto deg{ 0.0F };

  for (uint8_t i = 0; i < degDigits; i++) {
    deg *= 10.0F;
    deg += static_cast<float>(str[i] - '0');
  }

  auto min{ 0.0F };

  for (uint8_t i = 0; i < 2; i++) {
    min *= 10.0F;
    min += static_cast<float>(str[degDigits + i] - '0');
  }

  auto fraction{ 0.1F };
  for (uint8_t i = 0; i < 4; i++) {
    min += static_cast<float>(str[degDigits + 3 + i] - '0') * fraction;
    fraction *= 0.1F;
  }

  return deg + min / 60.0F;
}

} // namespace

void
GPSSensor::onGGAField(const char* field, const uint8_t fieldSize, const uint8_t fieldIndex)
{
  switch (fieldIndex) {
    case 0: /* UTC Time */
      (void)parseTimeOfDay(field, fieldSize);
      break;
    case 1 /* Latitude */:
      gga_.lat = parseDegreeMinutes(field, fieldSize, /*degDigits=*/2);
      break;
    case 2 /* Latitude Direction */:
      if (field[0] == 'S') {
        gga_.lat = -gga_.lat;
      }
      break;
    case 3 /* Longitude */:
      gga_.lon = parseDegreeMinutes(field, fieldSize, /*degDigits=*/3);
      break;
    case 4 /* Longitude Direction */:
      if (field[0] == 'W') {
        gga_.lon = -gga_.lon;
      }
      break;
    case 5 /* GPS Quality Indicator */:
      if (field[0] != '0') {
        gga_.hasFix = true;
      }
      break;
    case 6 /* Number of Satellites */:
      gga_.numSatellites = atoi(field);
      break;
    case 7 /* HDOP */:
      break;
    case 8 /* MSL Altitude */:
      sscanf(field, "%f", &gga_.alt);
      break;
    case 9 /* MSL Units */:
      break;
    case 10 /* Geoid Separation */:
      sscanf(field, "%f", &gga_.geoidSeparation);
      break;
    case 11 /* Geoid Separation Units */:
      break;
    case 12 /* Age of Differential Data Record */:
      break;
    case 13 /* Reference Station ID */:
      break;
  }
}

auto
GPSSensor::parseTimeOfDay(const char* str, const uint8_t len) -> bool
{
  if (len < 10) {
    return false;
  }

  // begin validation

  for (auto i = 0u; i < 6; i++) {
    const auto c = str[i];
    if ((c < '0') || (c > '9')) {
      return false;
    }
  }

  if (str[6] != '.') {
    return false;
  }

  for (auto i = 0u; i < 3; i++) {
    const auto c = str[i + 7];
    if ((c < '0') || (c > '9')) {
      return false;
    }
  }

  // end validation

  const auto hour = static_cast<int32_t>(str[0] - '0') * 10l + static_cast<int32_t>(str[1] - '0');
  const auto min = static_cast<int32_t>(str[2] - '0') * 10l + static_cast<int32_t>(str[3] - '0');
  const auto sec = static_cast<int32_t>(str[4] - '0') * 10l + static_cast<int32_t>(str[5] - '0');
  const auto ms = static_cast<int32_t>(str[7] - '0') * 100l + static_cast<int32_t>(str[8] - '0') * 10l +
                  static_cast<int32_t>(str[9] - '0');

  gga_.timeOfDay = (hour * 60l * 60l + min * 60l + sec) * 1000l + ms;

  return true;
}

void
GPSSensor::notifyGGA(const GGA& gga)
{
  if (ggaFunc_) {
    ggaFunc_(userData_, gga);
  }
}

void
GPSSensor::notifyVTG(const VTG& vtg)
{
  if (vtgFunc_) {
    vtgFunc_(userData_, vtg);
  }
}

void
GPSComponent::setSensor(GPSSensor* sensor)
{
  sensor_ = sensor;

  sensor_->setup(this, onGGA, onVTG);
}

void
GPSComponent::loop(MAVLinkBus& bus, const uint32_t timeDelta)
{
  timeSinceBootFractional_ += timeDelta;
  const auto elapsedMs = timeSinceBootFractional_ / 1000;
  timeSinceBootFractional_ -= elapsedMs * 1000;
  timeSinceBootMs_ += elapsedMs;

  if (readTimer_.step(timeDelta)) {
    (void)readFromSensor();
  }

  if (publishTimer_.step(timeDelta)) {
    (void)publishReport(bus);
  }
}

void
GPSComponent::onGGA(void* selfPtr, const GPSSensor::GGA& gga)
{
  auto* self = static_cast<GPSComponent*>(selfPtr);

  self->lastGGA_ = gga;

  if (gga.hasFix && !self->receivedFirstHeight_) {
    self->initialHeight_ = gga.alt;
    self->receivedFirstHeight_ = true;
  }
}

void
GPSComponent::onVTG(void* selfPtr, const GPSSensor::VTG& vtg)
{
  (void)selfPtr;
  (void)vtg;
}

auto
GPSComponent::readFromSensor() -> bool
{
  return sensor_->read();
}

auto
GPSComponent::publishReport(MAVLinkBus& bus) -> bool
{
  mavlink_global_position_int_t payload;
  payload.lat = static_cast<int32_t>(lastGGA_.lat * 1.0e7F);
  payload.lon = static_cast<int32_t>(lastGGA_.lon * 1.0e7F);
  payload.alt = lastGGA_.alt * 1.0e3F;
  payload.relative_alt = (lastGGA_.alt - initialHeight_) * 1.0e3F;
  payload.time_boot_ms = timeSinceBootMs_;
  payload.hdg = 0; // TODO : heading (cdeg)
  payload.vx = 0;  // TODO : velocity (cm/s)
  payload.vy = 0;
  payload.vz = 0;

  mavlink_message_t msg{};
  mavlink_msg_global_position_int_encode(/*system_id=*/1, MAV_COMP_ID_GPS, &msg, &payload);
  return bus.send(msg);
}

} // namespace AP
