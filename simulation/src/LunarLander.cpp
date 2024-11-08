#include "LunarLander.h"

#include <algorithm>
#include <random>

namespace sim {

namespace {

class ThrusterAction final : public Action<float>
{
public:
  ThrusterAction(const rp3::Vector3& position, const rp3::Vector3& normal, const float max_force)
    : position_(position)
    , normal_(normal)
    , max_force_(max_force)
  {
  }

  void apply(Agent& agent, const float value) override
  {
    auto* body = agent.getBody();

    const auto clippedValue = std::min(std::max(value, 0.0F), 1.0F);

    body->applyLocalForceAtLocalPosition(normal_ * value * max_force_, position_);
  }

private:
  rp3::Vector3 position_{};

  rp3::Vector3 normal_{};

  float max_force_{};
};

} // namespace

auto
createLunarLander(int seed) -> std::unique_ptr<Env>
{
  std::mt19937 rng(seed);
  std::uniform_real_distribution<float> heightDist(-40, -60);
  std::uniform_real_distribution<float> yDist(-50, 50);

  auto initPosition = rp3::Vector3(0, yDist(rng), heightDist(rng));
  auto orientation = rp3::Quaternion::fromEulerAngles(0, 0, 0);
  rp3::Transform initTransform(initPosition, orientation);

  auto env = std::make_unique<Env>();
  env->createFloor();
  auto ship = env->createAgent(initTransform);
  auto shape = env->getCommon()->createBoxShape(rp3::Vector3(1, 1, 1));
  ship->getBody()->addCollider(shape, initTransform);
  const auto max_force{ 20.0F };
  ship->registerAction(std::make_unique<ThrusterAction>(rp3::Vector3(0, 1, 0), rp3::Vector3(0, 0, -1), max_force));
  ship->registerAction(std::make_unique<ThrusterAction>(rp3::Vector3(0, -1, 0), rp3::Vector3(0, 0, -1), max_force));
  return env;
}

} // namespace sim
