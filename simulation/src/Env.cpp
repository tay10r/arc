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
  discreteActions_.at(action)->apply(*this, value);
}

void
Agent::registerAction(std::unique_ptr<Action<float>> action, std::string name)
{
  actions_.emplace_back(std::move(action));
  actionNames_.emplace_back(std::move(name));
}

void
Agent::registerAction(std::unique_ptr<Action<bool>> action, std::string name)
{
  discreteActions_.emplace_back(std::move(action));
  discreteActionNames_.emplace_back(std::move(name));
}

[[nodiscard]]
auto
Agent::getActionNames() const -> const std::vector<std::string>&
{
  return actionNames_;
}

[[nodiscard]]
auto
Agent::getDiscreteActionNames() const -> const std::vector<std::string>&
{
  return discreteActionNames_;
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
Env::getElapsedTime() const -> float
{
  return elapsedTime_;
}

auto
Env::getTimeDelta() const -> float
{
  return timeDelta_;
}

auto
Env::getCommon() -> rp3::PhysicsCommon*
{
  return &common_;
}

void
Env::step()
{
  world_->update(timeDelta_);

  elapsedTime_ += timeDelta_;
}

void
Env::setTimeDelta(float delta_t)
{
  timeDelta_ = delta_t;
}

auto
Env::createAgent(const rp3::Transform& transform) -> std::shared_ptr<Agent>
{
  auto agent = std::make_shared<Agent>(world_->createRigidBody(transform));
  agents_.emplace_back(agent);
  return agent;
}

auto
Env::createFloor(const float size, const float verticalOffset) -> rp3::RigidBody*
{
  auto* body = world_->createRigidBody(rp3::Transform());
  auto* floor = common_.createBoxShape(rp3::Vector3(size, size, 1));
  auto* collider =
    body->addCollider(floor, rp3::Transform(rp3::Vector3(0, 0, 1 + verticalOffset), rp3::Quaternion::identity()));
  collider->getMaterial().setBounciness(0.0);
  body->setType(rp3::BodyType::STATIC);
  return body;
}

void
Env::setRenderingEnabled(const bool enabled)
{
  world_->setIsDebugRenderingEnabled(enabled);

  const auto numBodies = world_->getNbRigidBodies();

  for (auto i = 0u; i < numBodies; i++) {
    auto* body = world_->getRigidBody(i);
    body->setIsDebugEnabled(enabled);
  }

  auto& renderer = world_->getDebugRenderer();

  renderer.setIsDebugItemDisplayed(rp3::DebugRenderer::DebugItem::COLLISION_SHAPE, enabled);
}

auto
Env::render() const -> std::vector<float>
{
  auto& renderer = world_->getDebugRenderer();

  renderer.computeDebugRenderingPrimitives(*world_);

  std::vector<float> buffer;

  const auto numTriangles = renderer.getNbTriangles();

  // 3 vertex per triangle, 6 floats per vertex
  buffer.resize(static_cast<std::size_t>(numTriangles) * 3 * 6);

  auto insertVertex = [&buffer](const rp3::Vector3& p, const rp3::Vector3& n, const std::uint32_t color) {
    buffer.emplace_back(p.x);
    buffer.emplace_back(p.y);
    buffer.emplace_back(p.z);

    buffer.emplace_back(n.x);
    buffer.emplace_back(n.y);
    buffer.emplace_back(n.z);

    buffer.emplace_back(0.8F);
    buffer.emplace_back(0.8F);
    buffer.emplace_back(0.8F);
  };

  const auto& triangles = renderer.getTriangles();

  for (auto i = 0u; i < numTriangles; i++) {
    const auto& tri = triangles[i];
    const auto e1 = tri.point2 - tri.point1;
    const auto e2 = tri.point3 - tri.point1;
    rp3::Vector3 n = e1.cross(e2);
    n.normalize();
    insertVertex(tri.point1, n, tri.color1);
    insertVertex(tri.point2, n, tri.color2);
    insertVertex(tri.point3, n, tri.color3);
  }

  return buffer;
}

auto
Env::getNumAgents() const -> std::size_t
{
  return agents_.size();
}

auto
Env::getAgent(const std::size_t index) -> std::shared_ptr<Agent>
{
  return agents_.at(index);
}

} // namespace sim
