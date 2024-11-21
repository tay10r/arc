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

  void registerAction(std::unique_ptr<Action<float>> action, std::string name);

  void registerAction(std::unique_ptr<Action<bool>> action, std::string name);

  [[nodiscard]] auto getPosition() const -> rp3::Vector3;

  [[nodiscard]] auto getActionNames() const -> const std::vector<std::string>&;

  [[nodiscard]] auto getDiscreteActionNames() const -> const std::vector<std::string>&;

private:
  rp3::RigidBody* body_{};

  std::vector<std::unique_ptr<Action<float>>> actions_;

  std::vector<std::unique_ptr<Action<bool>>> discreteActions_;

  std::vector<std::string> actionNames_;

  std::vector<std::string> discreteActionNames_;
};

class EnvEffect
{
public:
  virtual ~EnvEffect() = default;

  virtual void apply(std::vector<std::shared_ptr<Agent>>& agents) = 0;
};

class Env final
{
public:
  /**
   * @brief Initializes the environment.
   *
   * @param gravity The value for gravity, in terms of meters per second squared.
   * */
  Env(float gravity = 9.8);

  void step();

  [[nodiscard]] auto getTimeDelta() const -> float;

  [[nodiscard]] auto getElapsedTime() const -> float;

  void setTimeDelta(float delta_t);

#if 0 // TODO
  /**
   * @brief Simulates a body of water at a specific velocity and vertical offset.
   * */
  void addOceanEffect(float verticalOffset, float speedX, float speedY);

  void addEnvEffect(std::unique_ptr<EnvEffect> effect);
#endif

  auto createAgent(const rp3::Transform& transform) -> std::shared_ptr<Agent>;

  auto getCommon() -> rp3::PhysicsCommon*;

  auto createFloor(const float size, const float verticalOffset = 0) -> rp3::RigidBody*;

  void setRenderingEnabled(bool enabled);

  [[nodiscard]] auto render() const -> std::vector<float>;

  [[nodiscard]] auto getNumAgents() const -> std::size_t;

  [[nodiscard]] auto getAgent(std::size_t index) -> std::shared_ptr<Agent>;

private:
  rp3::PhysicsCommon common_;

  rp3::PhysicsWorld* world_{};

  float timeDelta_{ 0.01 };

  float elapsedTime_{ 0 };

  std::vector<std::shared_ptr<Agent>> agents_;

  std::vector<std::unique_ptr<EnvEffect>> effects_;
};

} // namespace sim
