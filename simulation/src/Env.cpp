#include "sim/Env.h"

namespace sim {

namespace {

auto
getDefaultSettings() -> rp3::PhysicsWorld::WorldSettings
{
  rp3::PhysicsWorld::WorldSettings settings;
  settings.gravity = rp3::Vector3(0, 0, 9.8);
  return settings;
}

} // namespace

Agent::Agent(rp3::RigidBody* body)
  : body_(body)
{
}

auto
Agent::getBody() -> rp3::RigidBody*
{
  return body_;
}

void
Agent::setAction(const int action, const float value)
{
  actions_.at(action)->apply(*this, value);
}

void
Agent::setAction(const int action, const bool value)
{
  discrete_actions_.at(action)->apply(*this, value);
}

void
Agent::registerAction(std::unique_ptr<Action<float>> action)
{
  actions_.emplace_back(std::move(action));
}

void
Agent::registerAction(std::unique_ptr<Action<bool>> action)
{
  discrete_actions_.emplace_back(std::move(action));
}

auto
Agent::getPosition() const -> rp3::Vector3
{
  return body_->getTransform().getPosition();
}

Env::Env()
  : world_(common_.createPhysicsWorld(getDefaultSettings()))
{
}

auto
Env::getCommon() -> rp3::PhysicsCommon*
{
  return &common_;
}

void
Env::step()
{
  world_->update(delta_t_);
}

void
Env::setTimeDelta(float delta_t)
{
  delta_t_ = delta_t;
}

auto
Env::createAgent(const rp3::Transform& transform) -> std::unique_ptr<Agent>
{
  auto agent = std::make_unique<Agent>(world_->createRigidBody(transform));
  return agent;
}

auto
Env::createFloor() -> rp3::RigidBody*
{
  auto* body = world_->createRigidBody(rp3::Transform());
  auto* floor = common_.createBoxShape(rp3::Vector3(1000, 1000, 1));
  body->addCollider(floor, rp3::Transform(rp3::Vector3(0, 0, 1), rp3::Quaternion::identity()));
  return body;
}

} // namespace sim
