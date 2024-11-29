#include "PropertiesEditor.h"

#include <imgui.h>
#include <imgui_stdlib.h>

namespace arc::editor {

namespace {

class Widget
{
public:
  virtual ~Widget() = default;

  virtual void render() = 0;

protected:
  auto editFloat3(const char* label, engine::Float3* v3, const float speed, const float minValue, const float maxValue)
    -> bool
  {
    float data[3]{ v3->x(), v3->y(), v3->z() };
    const auto changed = ImGui::DragFloat3(label, data, speed, minValue, maxValue);
    v3->set_x(data[0]);
    v3->set_y(data[1]);
    v3->set_z(data[2]);
    return changed;
  }

  auto editTransform(engine::Transform* transform) -> bool
  {
    auto changed{ false };
    changed |= editFloat3("Position", transform->mutable_position(), 0.01F, 0, 0);
    changed |= editFloat3("Rotation", transform->mutable_rotation(), 0.1F, -180.0F, 180.0F);
    changed |= editFloat3("Scale", transform->mutable_scale(), 0.1F, 0.0F, 0.0F);
    return changed;
  }
};

class PhysicsColliderWidget final : public Widget
{
public:
  PhysicsColliderWidget(engine::physics::Collider& collider)
    : collider_(&collider)
  {
  }

  void render() override
  {
    editTransform(collider_->mutable_transform());

    if (ImGui::BeginCombo("Shape", toString(collider_->shape_case()))) {

      if (ImGui::Selectable("Box")) {
        auto* box = collider_->mutable_box();
        auto* extents = box->mutable_extents();
        extents->set_x(1);
        extents->set_y(1);
        extents->set_z(1);
      }

      if (ImGui::Selectable("Sphere")) {
        auto* sphere = collider_->mutable_sphere();
        sphere->set_radius(1);
      }

      if (ImGui::Selectable("Capsule")) {
        auto* capsule = collider_->mutable_capsule();
        capsule->set_height(1);
        capsule->set_radius(0.25F);
      }

      ImGui::EndCombo();
    }

    using Case = engine::physics::Collider::ShapeCase;

    switch (collider_->shape_case()) {
      case Case::SHAPE_NOT_SET:
        break;
      case Case::kBox:
        render(*collider_->mutable_box());
        break;
      case Case::kSphere:
        render(*collider_->mutable_sphere());
        break;
      case Case::kCapsule:
        render(*collider_->mutable_capsule());
        break;
    }
  }

protected:
  void render(engine::physics::Box& box)
  {
    auto* extents = box.mutable_extents();
    float data[3]{ extents->x(), extents->y(), extents->z() };
    ImGui::DragFloat3("Extents [m]", data);
    extents->set_x(data[0]);
    extents->set_x(data[1]);
    extents->set_x(data[2]);
  }

  void render(engine::physics::Sphere& sphere)
  {
    auto r = sphere.radius();
    ImGui::DragFloat("Radius [m]", &r);
    sphere.set_radius(r);
  }

  void render(engine::physics::Capsule& capsule)
  {
    auto r = capsule.radius();
    auto h = capsule.height();
    ImGui::DragFloat("Radius [m]", &r);
    ImGui::DragFloat("Height [m]", &h);
    capsule.set_radius(r);
    capsule.set_height(h);
  }

  static auto toString(const engine::physics::Collider::ShapeCase shape) -> const char*
  {
    using Case = engine::physics::Collider::ShapeCase;

    switch (shape) {
      case Case::SHAPE_NOT_SET:
        break;
      case Case::kBox:
        return "Box";
      case Case::kSphere:
        return "Sphere";
      case Case::kCapsule:
        return "Capsule";
    }
    return "None";
  }

private:
  engine::physics::Collider* collider_{};
};

class PhysicsBodyWidget final : public Widget
{
public:
  explicit PhysicsBodyWidget(engine::physics::Body& body)
    : body_(&body)
  {
  }

  void render()
  {
    if (ImGui::BeginTabBar("Tabs")) {

      if (ImGui::BeginTabItem("Common")) {
        renderCommonProperties();
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Colliders")) {
        renderColliders();
        ImGui::EndTabItem();
      }

      ImGui::EndTabBar();
    }
  }

protected:
  void renderCommonProperties()
  {
    ImGui::InputText("Name", body_->mutable_name());

    editTransform(body_->mutable_transform());
  }

  void renderColliders()
  {
    auto& colliders = *body_->mutable_colliders();

    for (auto i = 0; i < colliders.size(); i++) {

      ImGui::PushID(i);

      ImGui::Separator();

      PhysicsColliderWidget subWidget(colliders[i]);

      subWidget.render();

      ImGui::PopID();
    }
  }

private:
  engine::physics::Body* body_{};
};

class PropertiesEditorImpl final : public PropertiesEditor
{
public:
  void render() override
  {
    for (auto& w : widgets_) {
      w->render();
    }
  }

  void newGame(engine::Game& game) override { game_ = &game; }

  void select(engine::physics::Body& body) override
  {
    widgets_.clear();

    widgets_.emplace_back(new PhysicsBodyWidget(body));
  }

private:
  engine::Game* game_{};

  std::vector<std::unique_ptr<Widget>> widgets_;
};

} // namespace

auto
PropertiesEditor::create() -> std::unique_ptr<PropertiesEditor>
{
  return std::make_unique<PropertiesEditorImpl>();
}

} // namespace arc::editor
