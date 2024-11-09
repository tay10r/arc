#pragma once

#include <memory>
#include <vector>

class Renderer
{
public:
  static auto create() -> std::unique_ptr<Renderer>;

  virtual ~Renderer() = default;

  virtual void render(const std::vector<float>& scene, const float aspect) = 0;
};
