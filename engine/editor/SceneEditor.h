#pragma once

#include <memory>

#include <engine/game.pb.h>

namespace arc::editor {

class SceneEditor
{
public:
  class Observer
  {
  public:
    virtual ~Observer() = default;

    virtual void onBodySelect(engine::physics::Body& body) = 0;
  };

  static auto create() -> std::unique_ptr<SceneEditor>;

  virtual ~SceneEditor() = default;

  virtual void render() = 0;

  virtual void newGame(engine::Game& game) = 0;

  virtual void setObserver(Observer* observer) = 0;
};

} // namespace arc::editor
