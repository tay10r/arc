#pragma once

#include <vz.h>

#include <vector>

namespace vz {

struct Camera final
{
  enum class Axis : char
  {
    X = 'x',
    Y = 'y',
    Z = 'z'
  };

  std::vector<Axis> rotationOrder{ Axis::X, Axis::Y, Axis::Z };

  float rotation[3]{ 0, 0, 0 };

  float position[3]{ 0, 0, 0 };

  bool interactive{ true };
};

} // namespace vz
