#include "CameraController.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace vz {

void
CameraControllerImpl::setPosition(const float x, const float y, const float z)
{
  position_ = glm::vec3(x, y, z);
}

void
CameraControllerImpl::setGrabbed(const bool grabbed)
{
  grabbed_ = grabbed;
}

void
CameraControllerImpl::setMouseCoords(const float x, const float y)
{
  mouseCoords_ = glm::vec2(x, y);
}

void
CameraControllerImpl::step(float timeDelta)
{
  const glm::vec3 fwd(1, 0, 0);

  const glm::vec3 up(0, 0, -1);

  const auto view = glm::lookAt(position_, position_ + fwd, up);

  const auto proj = glm::perspective(glm::radians(60.0F), 1.0F, 0.01F, 100.0F);

  mvp_ = proj * view;
}

auto
CameraControllerImpl::getCombinedMat4() const -> const float*
{
  return glm::value_ptr(mvp_);
}

auto
CameraController::create() -> std::unique_ptr<CameraController>
{
  return std::make_unique<CameraControllerImpl>();
}

} // namespace vz
