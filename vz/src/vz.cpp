#include <vz.h>

#include <memory>
#include <set>
#include <vector>

#include <cctype>
#include <cstdlib>
#include <cstring>

#include <GLFW/glfw3.h>

#include <GLES3/gl3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <implot.h>

#include <cmrc/cmrc.hpp>

#include "Camera.h"
#include "Framebuffer.h"
#include "Shader.h"
#include "VertexArray.h"

CMRC_DECLARE(vz_font);
CMRC_DECLARE(vz_shaders);

#include <iostream>

namespace vz {

namespace {

template<typename T>
class ObjectStore final
{
public:
  struct Comparator final
  {
    auto operator()(const std::unique_ptr<T>& a, const std::unique_ptr<T>& b) const -> bool
    {
      return a.get() < b.get();
    }
  };

  [[nodiscard]] auto add(T* object) -> T*
  {
    objects_.emplace(object);
    return object;
  }

  void destroy(T* obj)
  {
    // We temporarily do this just to make the types match, but unique_ptr
    // is not meant to actually release the object afters. This is slightly hacky.
    std::unique_ptr<T> tmp(obj);
    auto it = objects_.find(tmp);
    tmp.release();
    if (it != objects_.end()) {
      objects_.erase(it);
    }
  }

  void clear() { objects_.clear(); }

private:
  std::set<std::unique_ptr<T>, Comparator> objects_;
};

class GlobalState final
{
public:
  [[nodiscard]] auto setup(const char* windowTitle) -> bool
  {
    if (glfwInit() != GLFW_TRUE) {
      return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    auto* monitor = glfwGetPrimaryMonitor();

    const auto* videoMode = glfwGetVideoMode(monitor);

    window_ = glfwCreateWindow(videoMode->width, videoMode->height, windowTitle, nullptr, nullptr);
    if (!window_) {
      glfwTerminate();
      return false;
    }

    glfwMakeContextCurrent(window_);

    gladLoadGLES2Loader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));

    glClearColor(0, 0, 0, 1);

    glEnable(GL_DEPTH_TEST);

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForOpenGL(window_, true);

    ImGui_ImplOpenGL3_Init("#version 300 es");

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    loadFont();

    setupStyle();

    setupScreenQuad();

    return true;
  }

  void teardown()
  {
    shaders_.clear();
    vertexArrays_.clear();
    framebuffers_.clear();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window_);

    glfwTerminate();
  }

  [[nodiscard]] auto beginFrame() -> bool
  {
    glfwPollEvents();

    if (glfwWindowShouldClose(window_) == GLFW_TRUE) {
      return false;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Escape, /*repeat=*/false)) {
      return false;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    int w{};
    int h{};
    glfwGetFramebufferSize(window_, &w, &h);
    glViewport(0, 0, w, h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return true;
  }

  void endFrame()
  {
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window_);
  }

  void renderGrid()
  {
    auto* shader = getGridShader();
    if (!shader) {
      return;
    }

    shader->use();

    screenQuad_->draw(GL_TRIANGLES);
  }

  [[nodiscard]] auto getCamera() -> Camera& { return camera_; }

  [[nodiscard]] auto getShaders() -> ObjectStore<Shader>& { return shaders_; }

  [[nodiscard]] auto getVertexArrays() -> ObjectStore<VertexArray>& { return vertexArrays_; }

  [[nodiscard]] auto getFramebuffers() -> ObjectStore<Framebuffer>& { return framebuffers_; }

  [[nodiscard]] auto getScreenQuad() -> VertexArray* { return screenQuad_; }

  [[nodiscard]] auto getMeshShader() -> Shader*
  {
    if (meshShader_) {
      return meshShader_;
    }

    meshShader_ = shaders_.add(new ShaderImpl());

    if (!buildInternalShader(meshShader_, "shaders/mesh.vert", "shaders/mesh.frag")) {
      meshShader_->dumpLogs(std::cerr);
      return nullptr;
    }

    return meshShader_;
  }

protected:
  void loadFont()
  {
    const auto fs = cmrc::vz_font::get_filesystem();
    const auto file = fs.open("font/DroidSans.ttf");
    auto* data = std::malloc(file.size());
    if (!data) {
      return;
    }
    std::memcpy(data, file.begin(), file.size());
    auto& io = ImGui::GetIO();
    io.Fonts->AddFontFromMemoryTTF(data, file.size(), 18.0F);
    io.Fonts->Build();
  }

  void setupStyle()
  {
    auto& style = ImGui::GetStyle();
    style.FrameRounding = 3;
    style.WindowRounding = 3;
    style.ChildRounding = 3;
    style.PopupRounding = 3;
    style.WindowBorderSize = 0;
  }

  void setupScreenQuad()
  {
    screenQuad_ = vertexArrays_.add(new VertexArrayImpl());

    const float data[12]{ -1, -1, 1, -1, 1, 1, 1, 1, -1, 1, -1, -1 };

    screenQuad_->uploadData(data, sizeof(data), GL_STATIC_DRAW);

    screenQuad_->setVertexSizes({ 2 });
  }

  static auto openShader(const char* path) -> std::string
  {
    const auto fs = cmrc::vz_shaders::get_filesystem();
    const auto file = fs.open(path);
    return std::string(file.begin(), file.size());
  }

  [[nodiscard]] auto buildInternalShader(Shader* shader, const char* vertPath, const char* fragPath) -> bool
  {
    const auto vertSource = openShader(vertPath);
    const auto fragSource = openShader(fragPath);
    return shader->build(vertSource.c_str(), fragSource.c_str());
  }

  [[nodiscard]] auto getGridShader() -> Shader*
  {
    if (gridShader_) {
      return gridShader_;
    }

    gridShader_ = shaders_.add(new ShaderImpl());

    if (!buildInternalShader(gridShader_, "shaders/grid.vert", "shaders/grid.frag")) {
      return nullptr;
    }

    return gridShader_;
  }

private:
  GLFWwindow* window_{};

  Camera camera_;

  ObjectStore<Shader> shaders_;

  ObjectStore<VertexArray> vertexArrays_;

  ObjectStore<Framebuffer> framebuffers_;

  VertexArray* screenQuad_{};

  Shader* gridShader_{};

  Shader* meshShader_{};
};

std::unique_ptr<GlobalState> globalState;

} // namespace

auto
setup(const char* windowTitle) -> bool
{
  globalState.reset(new GlobalState());
  return globalState->setup(windowTitle);
}

void
teardown()
{
  globalState->teardown();
  globalState.reset();
}

auto
beginFrame() -> bool
{
  return globalState->beginFrame();
}

void
endFrame()
{
  globalState->endFrame();
}

void
setCameraPosition(float x, float y, float z)
{
  auto& cam = globalState->getCamera();
  cam.position[0] = x;
  cam.position[1] = y;
  cam.position[2] = z;
}

void
setCameraRotation(float x, float y, float z)
{
  auto& cam = globalState->getCamera();
  cam.rotation[0] = x;
  cam.rotation[1] = y;
  cam.rotation[2] = z;
}

auto
setCameraRotationOrder(const char* order) -> bool
{
  auto& cam = globalState->getCamera();
  cam.rotationOrder.clear();
  for (auto i = 0; order[i] != 0; i++) {
    const auto c = std::tolower(order[i]);
    switch (c) {
      case 'x':
      case 'y':
      case 'z':
        cam.rotationOrder.emplace_back(static_cast<Camera::Axis>(c));
        break;
      default:
        return false;
    }
  }
  return true;
}

void
setCameraInteractive(bool interactive)
{
  globalState->getCamera().interactive = interactive;
}

void
renderGrid()
{
  globalState->renderGrid();
}

auto
getScreenQuad() -> VertexArray*
{
  return globalState->getScreenQuad();
}

auto
createShader() -> Shader*
{
  return globalState->getShaders().add(new ShaderImpl());
}

void
destroyShader(Shader* shader)
{
  globalState->getShaders().destroy(shader);
}

auto
getMeshShader() -> Shader*
{
  return globalState->getMeshShader();
}

auto
createVertexArray() -> VertexArray*
{
  return globalState->getVertexArrays().add(new VertexArrayImpl());
}

void
destroyVertexArray(VertexArray* vertexArray)
{
  globalState->getVertexArrays().destroy(vertexArray);
}

auto
createFramebuffer(int width, int height, int numAttachments, bool includeDepth) -> Framebuffer*
{
  auto& framebuffers = globalState->getFramebuffers();
  auto* framebuffer = framebuffers.add(new FramebufferImpl());
  (void)static_cast<FramebufferImpl*>(framebuffer)->setup(width, height, numAttachments, includeDepth);
  return framebuffer;
}

} // namespace vz
