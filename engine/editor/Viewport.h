#pragma once

#include <memory>

namespace arc::editor {

class Viewport
{
public:
  static auto create() -> std::unique_ptr<Viewport>;

  virtual ~Viewport() = default;

  virtual void render() = 0;
};

} // namespace arc::editor
