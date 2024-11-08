#pragma once

#include <vector>

#include <reactphysics3d/reactphysics3d.h>

namespace sim {

namespace rp3 = reactphysics3d;

class Agent;

template<typename Value>
class Action
{
public:
  virtual ~Action() = default;

  virtual void apply(Agent& agent, Value value) = 0;
};

class Agent final
{
public:
  Agent(rp3::RigidBody* body);

  [[nodiscard]] auto getBody() -> rp3::RigidBody*;

  void setAction(int action, float value);

  void setAction(int action, bool value);

  void registerAction(std::unique_ptr<Action<float>> action);

  void registerAction(std::unique_ptr<Action<bool>> action);

  [[nodiscard]] auto getPosition() const -> rp3::Vector3;

private:
  rp3::RigidBody* body_{};

  std::vector<std::unique_ptr<Action<float>>> actions_;

  std::vector<std::unique_ptr<Action<bool>>> discrete_actions_;
};

class Env final
{
public:
  Env();

  void step();

  void setTimeDelta(float delta_t);

  auto createAgent(const rp3::Transform& transform) -> std::unique_ptr<Agent>;

  auto getCommon() -> rp3::PhysicsCommon*;

  auto createFloor() -> rp3::RigidBody*;

private:
  rp3::PhysicsCommon common_;

  rp3::PhysicsWorld* world_{};

  float delta_t_{ 0.01 };
};

} // namespace sim
