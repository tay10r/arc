#include <uikit/main.hpp>

namespace {

class AppImpl final : public uikit::app
{
public:
  void setup(uikit::platform&) override {}

  void teardown(uikit::platform&) override {}

  void loop(uikit::platform&) override {}
};

} // namespace

namespace uikit {

auto
app::create() -> std::unique_ptr<app>
{
  return std::make_unique<AppImpl>();
}

} // namespace uikit
