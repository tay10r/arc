#include "Framebuffer.h"

namespace vz {

FramebufferImpl::FramebufferImpl()
{
  glGenFramebuffers(1, &framebuffer_);

  glGenRenderbuffers(1, &renderbuffer_);
}

FramebufferImpl::~FramebufferImpl()
{
  glDeleteFramebuffers(1, &framebuffer_);

  glDeleteRenderbuffers(1, &renderbuffer_);

  glDeleteTextures(colorAttachments_.size(), colorAttachments_.data());
}

void
FramebufferImpl::use()
{
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

  glViewport(0, 0, size_.width, size_.height);
}

auto
FramebufferImpl::complete() const -> bool
{
  return complete_;
}

auto
FramebufferImpl::setup(int width, int height, int numAttachments, bool depthBuffer) -> bool
{
  size_.width = width;
  size_.height = height;

  colorAttachments_.resize(numAttachments);

  drawBuffers_.resize(numAttachments);

  glGenTextures(numAttachments, colorAttachments_.data());

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

  for (auto i = 0; i < numAttachments; i++) {
    glBindTexture(GL_TEXTURE_2D, colorAttachments_[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorAttachments_[i], 0);
    drawBuffers_[i] = GL_COLOR_ATTACHMENT0 + i;
  }

  glDrawBuffers(numAttachments, drawBuffers_.data());

  if (depthBuffer) {
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer_);
  }

  complete_ = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return complete_;
}

auto
FramebufferImpl::getColorAttachment(GLsizei index) -> GLuint
{
  return colorAttachments_.at(index);
}

auto
FramebufferImpl::size() const -> Size
{
  return size_;
}

} // namespace vz
