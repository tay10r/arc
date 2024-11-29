#pragma once

#include <memory>

#include <engine/game.pb.h>

namespace arc::editor {

class PropertiesEditor
{
public:
  static auto create() -> std::unique_ptr<PropertiesEditor>;

  virtual ~PropertiesEditor() = default;

  virtual void render() = 0;

  virtual void newGame(engine::Game& game) = 0;

  virtual void select(engine::physics::Body& body) = 0;
};

} // namespace arc::editor
