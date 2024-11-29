#include <vz.h>

#include "PropertiesEditor.h"
#include "SceneEditor.h"
#include "Viewport.h"

#include <engine/game.pb.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <spdlog/spdlog.h>

#include <fstream>

#include <cstdint>
#include <cstdlib>

namespace arc::editor {

namespace {

struct ViewMenu final
{
  bool sceneEditor{ true };

  bool propertiesEditor{ false };

  bool viewport{ false };
};

class MenuBar final
{
public:
  MenuBar(ViewMenu* viewMenu)
    : viewMenu_(viewMenu)
  {
  }

  void render()
  {
    if (!ImGui::BeginMainMenuBar()) {
      return;
    }

    if (ImGui::BeginMenu("View")) {

      ImGui::MenuItem("Scene Editor", nullptr, &viewMenu_->sceneEditor);

      ImGui::MenuItem("Properties Editor", nullptr, &viewMenu_->propertiesEditor);

      ImGui::MenuItem("Viewport", nullptr, &viewMenu_->viewport);

      ImGui::EndMenu();
    }

    return ImGui::EndMainMenuBar();
  }

private:
  ViewMenu* viewMenu_{};
};

const char* gamePath{ "game.bin" };

class Program final : public SceneEditor::Observer
{
public:
  [[nodiscard]] auto setup() -> bool
  {
    setupIniFile();

    sceneEditor_->setObserver(this);

    const auto* path{ gamePath };

    std::ifstream existingFile(path, std::ios::binary | std::ios::in);

    if (!existingFile.good()) {
      SPDLOG_INFO("Creating new game.");
      newGame();
      return true;
    }

    if (!game_.ParseFromIstream(&existingFile)) {
      SPDLOG_ERROR("Failed to load existing game file.");
      return false;
    }

    sceneEditor_->newGame(game_);

    SPDLOG_INFO("Loaded existing game file '{}'.", path);

    return true;
  }

  void teardown()
  {
    const auto* tmpPath{ "game.tmp.bin" };
    const auto* path{ gamePath };
    const auto data = game_.SerializeAsString();
    std::ofstream file(tmpPath, std::ios::binary | std::ios::out);
    file.write(data.data(), data.size());
    if (file.tellp() != static_cast<std::streampos>(data.size())) {
      SPDLOG_ERROR("Failed to save game data.");
      return;
    }
    std::rename(tmpPath, path);
  }

  void run()
  {
    while (vz::beginFrame()) {

      ImGui::DockSpaceOverViewport();

      menuBar_.render();

      if (viewMenu_.sceneEditor) {
        if (ImGui::Begin("Scene")) {
          sceneEditor_->render();
        }
        ImGui::End();
      }

      if (viewMenu_.propertiesEditor) {
        if (ImGui::Begin("Properties")) {
          propertiesEditor_->render();
        }
        ImGui::End();
      }

      if (viewMenu_.viewport) {
        if (ImGui::Begin("Viewport")) {
          viewport_->render();
        }
        ImGui::End();
      }

      vz::endFrame();
    }
  }

protected:
  void newGame()
  {
    game_ = engine::Game();
    sceneEditor_->newGame(game_);
  }

  void setupIniFile()
  {
    ImGui::GetIO().IniFilename = "editor.ini";

    ImGuiSettingsHandler settingsHandler{};
    settingsHandler.TypeName = "EditorFlags";
    settingsHandler.TypeHash = ImHashStr("EditorFlags");
    settingsHandler.ReadOpenFn = configOpen;
    settingsHandler.ReadLineFn = configReadLine;
    settingsHandler.WriteAllFn = configWriteAll;
    settingsHandler.UserData = this;
    ImGui::AddSettingsHandler(&settingsHandler);
  }

  auto getEditorFlags() const -> uint32_t
  {
    auto flags{ 0u };
    flags |= viewMenu_.sceneEditor ? 0x01 : 0x00;
    flags |= viewMenu_.propertiesEditor ? 0x02 : 0x00;
    flags |= viewMenu_.viewport ? 0x04 : 0x00;
    return flags;
  }

  void setEditorFlags(const uint32_t flags)
  {
    viewMenu_.sceneEditor = (flags & 0x01) ? true : false;
    viewMenu_.propertiesEditor = (flags & 0x02) ? true : false;
    viewMenu_.viewport = (flags & 0x04) ? true : false;
  }

  /* Functions for storing data in editor.ini */
  static auto configOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* /*name*/) -> void*
  {
    static int dummyVar{};
    return &dummyVar;
  }

  static void configReadLine(ImGuiContext*, ImGuiSettingsHandler* handler, void*, const char* line)
  {
    uint32_t flags{};
    if (sscanf(line, "%u", &flags) == 1) {
      auto* self = static_cast<Program*>(handler->UserData);
      self->setEditorFlags(flags);
    }
  }

  static void configWriteAll(ImGuiContext*, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
  {
    auto* self = static_cast<Program*>(handler->UserData);
    const auto flags = self->getEditorFlags();
    buf->appendf("[%s][Data]\n", "EditorFlags");
    buf->appendf("%u\n", flags);
    buf->append("\n");
  }

protected: /* scene editor observer */
  void onBodySelect(engine::physics::Body& body) override { propertiesEditor_->select(body); }

private:
  ViewMenu viewMenu_;

  MenuBar menuBar_{ &viewMenu_ };

  engine::Game game_;

  std::unique_ptr<SceneEditor> sceneEditor_{ SceneEditor::create() };

  std::unique_ptr<PropertiesEditor> propertiesEditor_{ PropertiesEditor::create() };

  std::unique_ptr<Viewport> viewport_{ Viewport::create() };
};

} // namespace

} // namespace arc::editor

auto
main() -> int
{
  if (!vz::setup("Editor")) {
    return EXIT_FAILURE;
  }

  {
    arc::editor::Program prg;
    if (!prg.setup()) {
      return EXIT_FAILURE;
    }
    prg.run();
    prg.teardown();
  }

  vz::teardown();

  return EXIT_SUCCESS;
}
