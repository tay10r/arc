#include <uikit/main.hpp>
#include <uikit/shader_compiler.hpp>

#include <sim/Env.h>
#include <sim/EnvRegistry.h>

#include <AP_Program.h>

#include "Renderer.h"
#include "Simulation.h"

namespace {

struct ManualControlState final
{
  bool active{ false };

  std::vector<float> values;

  std::vector<int> discreteValues;
};

class AppImpl final : public uikit::app
{
public:
  void setup(uikit::platform&) override
  {
    renderer_ = Renderer::create();

    createEnv("LunarLander");

    env_->setRenderingEnabled(true);
  }

  void teardown(uikit::platform&) override { renderer_.reset(); }

  void loop(uikit::platform& plt) override
  {
    auto& io = ImGui::GetIO();
    auto aspect = io.DisplaySize.x / io.DisplaySize.y;

    if (manualControlState_.active) {
      auto& agent = agents_[selectedAgent_];
      for (auto i = 0; i < manualControlState_.values.size(); i++) {
        agent->setAction(i, manualControlState_.values[i]);
      }
      for (auto i = 0; i < manualControlState_.discreteValues.size(); i++) {
        agent->setAction(i, static_cast<bool>(manualControlState_.discreteValues[i]));
      }
    }
    env_->step();
    for (auto& sim : simulations_) {
      sim->loop(env_->getTimeDelta());
    }

    renderer_->render(env_->render(), aspect);

    if (ImGui::Begin("Manual Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
      renderManualControlUi();
    }
    ImGui::End();
  }

protected:
  void renderManualControlUi()
  {
    if (ImGui::SliderInt("Agent", &selectedAgent_, 0, agents_.size() - 1)) {
      manualControlState_ = ManualControlState();
    }

    auto& agent = *agents_[selectedAgent_];

    ImGui::Checkbox("Active", &manualControlState_.active);

    ImGui::BeginDisabled(!manualControlState_.active);

    const auto numActions = agent.getActionNames().size();

    manualControlState_.values.resize(numActions);
    for (auto i = 0; i < numActions; i++) {
      const auto& label = agent.getActionNames()[i];
      ImGui::SliderFloat(label.c_str(), &manualControlState_.values[i], 0, 1);
    }

    const auto numDiscreteActions = agent.getDiscreteActionNames().size();
    manualControlState_.discreteValues.resize(numDiscreteActions);
    for (auto i = 0; i < numDiscreteActions; i++) {
      const auto& label = agent.getDiscreteActionNames()[i];
      auto value = static_cast<bool>(manualControlState_.discreteValues[i]);
      ImGui::Checkbox(label.c_str(), &value);
      manualControlState_.discreteValues[i] = static_cast<int>(value);
    }

    ImGui::EndDisabled();
  }

  void createEnv(const std::string& name)
  {
    simulations_.clear();

    env_ = envRegistry_.createEnv(name);

    const auto numAgents = env_->getNumAgents();

    for (auto i = 0u; i < numAgents; i++) {
      auto agent = env_->getAgent(i);
      auto sim = Simulation::create(agent);
      simulations_.emplace_back(std::move(sim));
      agents_.emplace_back(std::move(agent));
    }
    for (auto& sim : simulations_) {
      sim->setup();
    }
  }

private:
  std::unique_ptr<Renderer> renderer_;

  sim::EnvRegistry envRegistry_;

  std::unique_ptr<sim::Env> env_;

  std::vector<std::unique_ptr<Simulation>> simulations_;

  std::vector<std::shared_ptr<sim::Agent>> agents_;

  ManualControlState manualControlState_;

  int selectedAgent_{};
};

} // namespace

namespace uikit {

auto
app::create() -> std::unique_ptr<app>
{
  return std::make_unique<AppImpl>();
}

} // namespace uikit
