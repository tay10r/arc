#include "Util.h"

#include <iomanip>
#include <sstream>

namespace arc::editor {

auto
formatLabel(uint32_t id, const std::string& str) -> std::string
{
  uint32_t idLen{};

  if (id <= 0xfful) {
    idLen = 2;
  } else if (id <= 0xffffUL) {
    idLen = 4;
  } else if (id <= 0xffffffUL) {
    idLen = 6;
  } else {
    idLen = 8;
  }
  std::ostringstream stream;
  stream << std::setw(idLen) << std::setfill('0') << std::hex << id;
  if (!str.empty()) {
    stream << ": " << str;
  }
  return stream.str();
}

auto
formatLabel(uint32_t id, const engine::physics::Collider& c) -> std::string
{
  switch (c.shape_case()) {
    case engine::physics::Collider::ShapeCase::kBox:
      return formatLabel(id, "Box");
    case engine::physics::Collider::ShapeCase::kSphere:
      return formatLabel(id, "Sphere");
    case engine::physics::Collider::ShapeCase::kCapsule:
      return formatLabel(id, "Capsule");
    case engine::physics::Collider::ShapeCase::SHAPE_NOT_SET:
      break;
  }

  return formatLabel(id, "");
}

auto
generateID(engine::Game& game) -> uint32_t
{
  const auto id = game.idgenerator();
  game.set_idgenerator(id + 1);
  return id;
}

void
initTransform(engine::Transform& transform)
{
  auto* scale = transform.mutable_scale();
  scale->set_x(1);
  scale->set_y(1);
  scale->set_z(1);
}

} // namespace arc::editor
