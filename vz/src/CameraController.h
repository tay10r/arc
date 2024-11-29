#pragma once

#include <vz.h>

#include <glm/glm.hpp>

namespace vz {

class CameraControllerImpl final : public CameraController
{
public:
  void setPosition(float x, float y, float z) override;

  void setGrabbed(bool grabbed) override;

  void setMouseCoords(float x, float y) override;

  void step(float timeDelta) override;

  [[nodiscard]] auto getCombinedMat4() const -> const float* override;

private:
  glm::vec3 position_{};

  glm::mat4 mvp_ = glm::mat4(1.0);

  bool grabbed_{ false };

  glm::vec2 mouseCoords_{};
};

} // namespace vz
