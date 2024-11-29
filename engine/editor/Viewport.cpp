#include "Viewport.h"

namespace arc::editor {

namespace {

class ViewportImpl final : public Viewport
{
public:
  void render() override
  {
    //
  }
};

} // namespace

auto
Viewport::create() -> std::unique_ptr<Viewport>
{
  return std::make_unique<ViewportImpl>();
}

} // namespace arc::editor
