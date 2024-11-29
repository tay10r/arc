#pragma once

#include <vz.h>

namespace vz {

class FramebufferImpl final : public Framebuffer
{
public:
  FramebufferImpl();

  ~FramebufferImpl();

  [[nodiscard]] auto complete() const -> bool override;

  [[nodiscard]] auto setup(int width, int height, int numAttachments, bool depthBuffer) -> bool;

  [[nodiscard]] auto getColorAttachment(GLsizei index) -> GLuint override;

  [[nodiscard]] auto size() const -> Size override;

  [[nodiscard]] auto readPixels() const -> std::vector<std::uint8_t> override
  {
    std::vector<std::uint8_t> data;
    data.resize(size_.width * size_.height * 4);
    glReadPixels(0, 0, size_.width, size_.height, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    return data;
  }

  void use() override;

private:
  GLuint framebuffer_{};

  GLuint renderbuffer_{};

  std::vector<GLuint> colorAttachments_;

  std::vector<GLuint> drawBuffers_;

  Size size_{};

  bool complete_{ false };
};

} // namespace vz
