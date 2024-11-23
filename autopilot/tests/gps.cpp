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
  auto gga_callback = [](void*, const AP::GPSSensor::GGA& gga) {
    EXPECT_NEAR(gga.lat, 53.361337F, 0.01);
    EXPECT_NEAR(gga.lon, -6.50562F, 0.01);
    EXPECT_NEAR(gga.alt, 61.7F, 0.01);
  };
  FakeSensor sensor("$GPGGA,092750.000,5321.6802,N,00630.3372,W,1,8,1.03,61.7,M,55.2,M,,*76\r\n");

  sensor.setup(nullptr, gga_callback, nullptr);
  EXPECT_EQ(sensor.read(), true);
}
