#pragma once

#include <vz.h>

namespace vz {

class VertexArrayImpl final : public VertexArray
{
public:
  VertexArrayImpl();

  VertexArrayImpl(const VertexArrayImpl&) = delete;

  VertexArrayImpl(VertexArrayImpl&&) = delete;

  auto operator=(const VertexArrayImpl&) -> VertexArrayImpl& = delete;

  auto operator=(VertexArrayImpl&&) -> VertexArrayImpl& = delete;

  ~VertexArrayImpl();

  void setVertexSizes(const std::vector<GLint>& sizes) override;

  void uploadData(const void* data, GLsizeiptr size, GLenum usage) override;

  void draw(GLenum mode, const GLsizei count, GLint offset) override;

  [[nodiscard]] auto bytesPerVertex() const -> GLsizeiptr override;

  [[nodiscard]] auto totalBytes() const -> GLsizeiptr override;

private:
  GLuint vertexArray_{};

  GLuint buffer_{};

  std::vector<GLint>::size_type numAttribs_{};

  GLsizeiptr bytesPerVertex_{};

  GLsizeiptr totalBytes_{};
};

} // namespace vz
