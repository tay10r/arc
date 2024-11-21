#include "ThrusterAction.h"

namespace sim {

ThrusterAction::ThrusterAction(const rp3::Vector3& position, const rp3::Vector3& normal, const float maxForce)
  : position_(position)
  , normal_(normal)
  , maxForce_(maxForce)
{
}

void
ThrusterAction::apply(Agent& agent, const float value)
{
  auto* body = agent.getBody();

  const auto clippedValue = std::min(std::max(value, 0.0F), 1.0F);

  body->applyLocalForceAtLocalPosition(normal_ * value * maxForce_, position_);
}

} // namespace sim
