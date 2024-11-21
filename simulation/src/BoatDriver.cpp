#include "BoatDriver.h"

#include <numbers>
#include <random>

#include "ThrusterAction.h"

namespace sim {

auto
createBoatDriver(const int seedValue) -> std::unique_ptr<Env>
{
  std::seed_seq seed{ seedValue };
  std::mt19937 rng(seed);

  constexpr auto pi{ 3.1415F };

  auto initPosition = rp3::Vector3(0, 0, 0);

  std::uniform_real_distribution<float> headingDist(-pi, pi);
  auto orientation = rp3::Quaternion::fromEulerAngles(0, 0, headingDist(rng));

  rp3::Transform initTransform(initPosition, orientation);

  std::uniform_real_distribution<float> forceDist(5, 10);
  const auto maxForce = forceDist(rng);

  auto env = std::make_unique<Env>(/*gravity=*/0.0F);
  auto ship = env->createAgent(initTransform);
  // ship->getBody()->setAngularDamping(1.0F);
  // ship->getBody()->setLinearDamping(1.0F);
  ship->getBody()->setLinearDamping(maxForce);
  // ship->getBody()->setAngularDamping(maxForce);
  auto shape = env->getCommon()->createBoxShape(rp3::Vector3(1, 1, 1));
  auto* collider = ship->getBody()->addCollider(shape, initTransform);
  ship->registerAction(
    std::make_unique<ThrusterAction>(/*position=*/rp3::Vector3(0, -1, 0), /*normal=*/rp3::Vector3(-1, 0, 0), maxForce),
    "l_thrust");
  ship->registerAction(
    std::make_unique<ThrusterAction>(/*position=*/rp3::Vector3(0, 1, 0), /*normal=*/rp3::Vector3(-1, 0, 0), maxForce),
    "r_thrust");
  return env;
}

} // namespace sim
