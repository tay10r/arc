#include "Renderer.h"

#include <uikit/shader_compiler.hpp>

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <map>
#include <string>

#include <cassert>

namespace {

class Shader final
{
public:
  Shader(const char* vert, const char* frag)
    : id_(uikit::compile_shader(vert, frag, {}))
  {
  }

  Shader(const Shader&) = delete;

  ~Shader() { glDeleteProgram(id_); }

  void use() { glUseProgram(id_); }

  [[nodiscard]] auto findUniform(const char* name) -> GLint
  {
    auto it = uniforms_.find(name);
    if (it == uniforms_.end()) {
      const auto loc = glGetUniformLocation(id_, name);
      it = uniforms_.emplace(name, loc).first;
    }
    return it->second;
  }

  [[nodiscard]] auto findAttrib(const char* name) -> GLint
  {
    auto it = attribs_.find(name);
    if (it == attribs_.end()) {
      const auto loc = glGetAttribLocation(id_, name);
      it = attribs_.emplace(name, loc).first;
    }
    return it->second;
  }

private:
  GLuint id_{};

  std::map<std::string, GLint> uniforms_;

  std::map<std::string, GLint> attribs_;
};

const char lineVertSource[] = R"(
attribute vec3 position;

uniform mat4 mvp;

void
main()
{
  gl_Position = mvp * vec4(position, 1.0);
}
)";

const char lineFragSource[] = R"(
void
main()
{
  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)";

const char triVertSource[] = R"(
attribute vec3 position;

attribute vec3 normal;

attribute vec3 color;

uniform mat4 mvp;

varying highp vec3 interpolatedColor;

varying highp vec3 interpolatedNormal;

void
main()
{
  interpolatedColor = color;
  interpolatedNormal = normal;
  gl_Position = mvp * vec4(position, 1.0);
}
)";

const char triFragSource[] = R"(

varying highp vec3 interpolatedColor;

varying highp vec3 interpolatedNormal;

uniform highp vec3 lightDirection;

void
main()
{
  float lighting = (dot(lightDirection, interpolatedNormal) + 1.0) * 0.5;

  gl_FragColor = vec4(interpolatedColor * lighting, 1.0);
}
)";

struct VertexAttrib final
{
  GLint index{};
  GLint size{};
};

class VertexBuffer final
{
public:
  VertexBuffer() { glGenBuffers(1, &buffer_); }

  VertexBuffer(const VertexBuffer&) = delete;

  ~VertexBuffer() { glDeleteBuffers(1, &buffer_); }

  void setArrayData(const std::vector<float>& buffer, const GLenum usage = GL_DYNAMIC_DRAW)
  {
    assert(glGetError() == GL_NO_ERROR);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buffer[0]) * buffer.size(), buffer.data(), usage);
    numFloats_ = buffer.size();
    assert(glGetError() == GL_NO_ERROR);
  }

  template<typename... Attribs>
  void draw(GLenum mode, Attribs... attribs)
  {
    assert(glGetError() == GL_NO_ERROR);

    const auto elementSize = getElementSize(attribs...);

    const auto numVertices = (numFloats_ * sizeof(float)) / elementSize;

    glBindBuffer(GL_ARRAY_BUFFER, buffer_);

    setupAttrib(elementSize, /*offset=*/0, attribs...);

    glDrawArrays(mode, 0, numVertices);

    assert(glGetError() == GL_NO_ERROR);
  }

protected:
  template<typename... Attribs>
  [[nodiscard]] static auto getElementSize(const Attribs&... attribs) -> std::size_t
  {
    return getElementSizeImpl(attribs...);
  }

  template<typename... Attribs>
  [[nodiscard]] static auto getElementSizeImpl(const VertexAttrib& attrib, const Attribs&... others) -> std::size_t
  {
    return attrib.size * sizeof(float) + getElementSizeImpl(others...);
  }

  template<typename... Attribs>
  [[nodiscard]] static auto getElementSizeImpl() -> std::size_t
  {
    return static_cast<std::size_t>(0);
  }

  template<typename... Others>
  static void setupAttrib(const std::size_t elementSize,
                          const std::size_t offset,
                          const VertexAttrib& attrib,
                          const Others&... others)
  {
    glEnableVertexAttribArray(attrib.index);

    glVertexAttribPointer(
      attrib.index, attrib.size, GL_FLOAT, GL_FALSE, /*stride=*/elementSize, reinterpret_cast<const void*>(offset));

    setupAttrib(elementSize, offset + sizeof(float) * attrib.size, others...);
  }

  static void setupAttrib(std::size_t, std::size_t) {}

private:
  GLuint buffer_{};

  std::size_t numFloats_{};
};

class RendererImpl final : public Renderer
{
public:
  RendererImpl()
  {
    createGrid();

    glEnable(GL_DEPTH_TEST);
  }

  void render(const std::vector<float>& buffer, const float aspect) override
  {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    const auto view = glm::lookAt(glm::vec3(10, 10, -10), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1));
    const auto proj = glm::perspective(glm::radians(90.0F), aspect, 0.1F, 500.0F);
    const auto mvp = proj * view;

    {
      lineShader_.use();
      const auto mvpLocation = lineShader_.findUniform("mvp");
      glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
      gridBuffer_.draw(GL_LINES, VertexAttrib{ lineShader_.findAttrib("position"), 3 });
    }

    {
      triShader_.use();
      const auto lightDirLocation = triShader_.findUniform("lightDirection");
      glUniform3f(lightDirLocation, lightDirection_.x, lightDirection_.y, lightDirection_.z);
      const auto mvpLocation = triShader_.findUniform("mvp");
      glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
      sceneBuffer_.setArrayData(buffer);
      sceneBuffer_.draw(GL_TRIANGLES,
                        VertexAttrib{ triShader_.findAttrib("position"), 3 },
                        VertexAttrib{ triShader_.findAttrib("normal"), 3 },
                        VertexAttrib{ triShader_.findAttrib("color"), 3 });
    }
  }

protected:
  void createGrid()
  {
    const auto cells{ 10 };

    const auto separation{ 1.0F };

    std::vector<float> data;

    const auto vMin{ -cells * separation };
    const auto vMax{ cells * separation };

    for (auto y = -cells; y <= cells; y++) {
      data.emplace_back(vMin);
      data.emplace_back(y);
      data.emplace_back(0.0F);

      data.emplace_back(vMax);
      data.emplace_back(y);
      data.emplace_back(0.0F);
    }

    for (auto x = -cells; x <= cells; x++) {
      data.emplace_back(x);
      data.emplace_back(vMin);
      data.emplace_back(0.0F);

      data.emplace_back(x);
      data.emplace_back(vMax);
      data.emplace_back(0.0F);
    }

    gridBuffer_.setArrayData(data);
  }

private:
  Shader lineShader_{ lineVertSource, lineFragSource };

  Shader triShader_{ triVertSource, triFragSource };

  VertexBuffer gridBuffer_;

  VertexBuffer sceneBuffer_;

  glm::vec3 lightDirection_{ glm::normalize(glm::vec3(0.3, 0.6, -0.5)) };
};

} // namespace

auto
Renderer::create() -> std::unique_ptr<Renderer>
{
  return std::make_unique<RendererImpl>();
}
