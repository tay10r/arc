#include <AP_GPS.h>

#include <gtest/gtest.h>

#include <string>

namespace {

class FakeSensor final : public AP::GPSSensor
{
public:
  FakeSensor(std::string data)
    : data_(std::move(data))
  {
  }

  [[nodiscard]] auto read() -> bool override { return parseData(&data_[0], data_.size()); }

private:
  std::string data_;
};

} // namespace

TEST(GPS, ParseGGA)
{
  auto onGGA = [](void*, const AP::GPSSensor::GGA& gga) {
    EXPECT_NEAR(gga.lat, 53.361337F, 0.01);
    EXPECT_NEAR(gga.lon, -6.50562F, 0.01);
    EXPECT_NEAR(gga.alt, 61.7F, 0.01);
  };
  FakeSensor sensor("$GPGGA,092750.000,5321.6802,N,00630.3372,W,1,8,1.03,61.7,M,55.2,M,,*76\r\n");
  sensor.setup(nullptr, onGGA, nullptr);
  EXPECT_EQ(sensor.read(), true);
}

TEST(GPS, ParseOther)
{
  const char data[] = "$GNRMC,195339.00,A,4127.80494,N,07157.42566,W,0.037,,231124,,,A*7D\r\n"
                      "$GNVTG,,T,,M,0.037,N,0.069,K,A*36\r\n"
                      "$GNGGA,195339.00,4127.80494,N,07157.42566,W,1,12,0.82,55.0,M,-33.9,M,,*4F\r\n";

  FakeSensor sensor(data);
  auto receivedGGA{ false };
  auto onGGA = [](void* receivedGGAPtr, const AP::GPSSensor::GGA& gga) {
    EXPECT_NEAR(gga.alt, 55.0F, 0.01F);
    *static_cast<bool*>(receivedGGAPtr) = true;
  };
  sensor.setup(&receivedGGA, onGGA, nullptr);
  EXPECT_EQ(sensor.read(), true);
  EXPECT_EQ(sensor.read(), true);
  EXPECT_EQ(sensor.read(), true);
  EXPECT_TRUE(receivedGGA);
}
