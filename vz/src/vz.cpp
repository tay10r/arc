#include <vz.h>

#include <memory>
#include <set>
#include <vector>

#include <cctype>

#include <GLFW/glfw3.h>

#include <GLES3/gl3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <implot.h>

#include "Camera.h"
#include "Shader.h"
#include "VertexArray.h"

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

    return true;
  }

  void teardown()
  {
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

  [[nodiscard]] auto getCamera() -> Camera& { return camera_; }

  [[nodiscard]] auto getShaders() -> ObjectStore<Shader>& { return shaders_; }

  [[nodiscard]] auto getVertexArrays() -> ObjectStore<VertexArray>& { return vertexArrays_; }

private:
  GLFWwindow* window_{};

  Camera camera_;

  ObjectStore<Shader> shaders_;

  ObjectStore<VertexArray> vertexArrays_;
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
createVertexArray() -> VertexArray*
{
  return globalState->getVertexArrays().add(new VertexArrayImpl());
}

void
destroyVertexArray(VertexArray* vertexArray)
{
  globalState->getVertexArrays().destroy(vertexArray);
}

} // namespace vz
