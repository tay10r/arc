#pragma once

#include "AP_GPS.h"

#include "AP_Random.h"
#include "AP_Time.h"
#include "AP_WGS84.h"

namespace SIM {

class GPSSensor final : public AP::GPSSensor
{
public:
  GPSSensor(uint32_t seed);

  void step(uint32_t timeDelta);

  void setOrigin(float lat, float lon);

  void setOffset(float x, float y);

  [[nodiscard]] auto read() -> bool override;

private:
  AP::Random random_;

  AP::WGS84Ref ref_;

  float xOffset_{};

  float yOffset_{};

  float noise_{ 0.2F };

  AP::Timer sampleTimer_{ 1000000ul };

  bool sentenceDue_{ false };
};

} // namespace SIM
