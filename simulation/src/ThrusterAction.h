#pragma once

#include <sim/Env.h>

namespace sim {

class ThrusterAction final : public Action<float>
{
public:
  ThrusterAction(const rp3::Vector3& position, const rp3::Vector3& normal, const float maxForce);

  void apply(Agent& agent, const float value) override;

private:
  rp3::Vector3 position_{};

  rp3::Vector3 normal_{};

  float maxForce_{};
};

} // namespace sim
