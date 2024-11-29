#include "SceneEditor.h"

#include "Util.h"

#include <imgui.h>
#include <imgui_stdlib.h>

#include <map>
#include <stdexcept>

#include <stdint.h>

namespace arc::editor {

namespace {

class SceneEditorImpl final : public SceneEditor
{
public:
  void render() override
  {
    if (!game_) {
      return;
    }

    if (ImGui::Button("New Scene")) {
      scene_ = game_->mutable_scenes()->Add();
      scene_->set_id(generateID(*game_));
    }

    ImGui::SameLine();

    const std::string currentSceneName = scene_ ? formatLabel(scene_->id(), scene_->name()).c_str() : "(None)";

    if (ImGui::BeginCombo("Current", currentSceneName.c_str())) {
      for (auto& scn : *game_->mutable_scenes()) {
        if (ImGui::Selectable(formatLabel(scn.id(), scn.name()).c_str())) {
          scene_ = &scn;
        }
      }
      ImGui::EndCombo();
    }

    if (!scene_) {
      return;
    }

    ImGui::InputText("Scene Name", scene_->mutable_name());

    ImGui::Separator();

    for (auto& body : *scene_->mutable_bodies()) {

      const auto label = formatLabel(body.id(), body.name());

      auto selected = &body == selectedBody_;

      if (ImGui::Selectable(label.c_str(), selected)) {
        selectBody(body);
        selected = true;
      }

      if (selected) {
        const auto indentSize = ImGui::CalcTextSize("  ").x;
        ImGui::Indent(indentSize);
        for (auto& c : *body.mutable_colliders()) {
          ImGui::Selectable(formatLabel(c.id(), c).c_str());
        }
        ImGui::Indent(-indentSize);
      }
    }

    ImGui::Separator();

    if (ImGui::Button("Add Physics Body")) {
      addPhysicsBody();
    }

    ImGui::SameLine();

    ImGui::BeginDisabled(selectedBody_ == nullptr);
    if (ImGui::Button("Add Collider")) {
      addCollider();
    }
    ImGui::EndDisabled();
  }

  void setObserver(Observer* observer) override { observer_ = observer; }

  void newGame(engine::Game& game) override
  {
    game_ = &game;
    auto* scenes = game.mutable_scenes();
    scene_ = scenes->empty() ? static_cast<engine::Scene*>(nullptr) : &scenes->at(0);
  }

protected:
  void addPhysicsBody()
  {
    auto* body = scene_->mutable_bodies()->Add();
    body->set_id(generateID(*game_));
    initTransform(*body->mutable_transform());
  }

  void addCollider()
  {
    auto* collider = selectedBody_->mutable_colliders()->Add();
    collider->set_id(generateID(*game_));
    initTransform(*collider->mutable_transform());
  }

  void selectBody(engine::physics::Body& body)
  {
    selectedBody_ = &body;
    observer_->onBodySelect(body);
  }

private:
  engine::Game* game_{};

  engine::Scene* scene_{};

  engine::physics::Body* selectedBody_{};

  Observer* observer_{};
};

} // namespace

auto
SceneEditor::create() -> std::unique_ptr<SceneEditor>
{
  return std::make_unique<SceneEditorImpl>();
}

} // namespace arc::editor
