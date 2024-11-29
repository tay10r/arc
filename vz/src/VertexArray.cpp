#include "VertexArray.h"

namespace vz {

void
VertexArray::draw(const GLenum mode)
{
  draw(mode, totalBytes() / bytesPerVertex(), /*first=*/0);
}

VertexArrayImpl::VertexArrayImpl()
{
  glGenVertexArrays(1, &vertexArray_);

  glGenBuffers(1, &buffer_);
}

VertexArrayImpl::~VertexArrayImpl()
{
  glDeleteVertexArrays(1, &vertexArray_);

  glDeleteBuffers(1, &buffer_);
}

void
VertexArrayImpl::uploadData(const void* data, const GLsizeiptr size, const GLenum usage)
{
  glBindBuffer(GL_ARRAY_BUFFER, buffer_);
  glBufferData(GL_ARRAY_BUFFER, size, data, usage);
  totalBytes_ = size;
}

void
VertexArrayImpl::setVertexSizes(const std::vector<GLint>& sizes)
{
  numAttribs_ = sizes.size();

  bytesPerVertex_ = 0;
  for (const auto& s : sizes) {
    bytesPerVertex_ += sizeof(float) * s;
  }

  using Size = std::vector<GLint>::size_type;

  glBindVertexArray(vertexArray_);

  glBindBuffer(GL_ARRAY_BUFFER, buffer_);

  GLsizeiptr offset{};

  for (Size i = 0; i < sizes.size(); i++) {

    glEnableVertexAttribArray(i);

    glVertexAttribPointer(i, sizes[i], GL_FLOAT, GL_FALSE, bytesPerVertex_, reinterpret_cast<void*>(offset));

    offset += sizeof(float) * sizes[i];
  }
}

void
VertexArrayImpl::draw(const GLenum mode, const GLsizei count, const GLint first)
{
  glBindVertexArray(vertexArray_);

  glDrawArrays(mode, first, count);
}

auto
VertexArrayImpl::bytesPerVertex() const -> GLsizeiptr
{
  return bytesPerVertex_;
}

auto
VertexArrayImpl::totalBytes() const -> GLsizeiptr
{
  return totalBytes_;
}

} // namespace vz
