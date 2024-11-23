#pragma once

#include <stdint.h>

namespace AP {

template<typename Scalar, int Dim>
struct Vector final
{
  Scalar data[Dim];

  auto operator+(const Vector& other) const -> Vector;

  auto operator-(const Vector& other) const -> Vector;

  auto operator[](int i) -> Scalar&;

  auto operator[](int i) const -> const Scalar&;
};

using Vec2f = Vector<float, 2>;

using Vec3f = Vector<float, 3>;

using Vec2i = Vector<int32_t, 2>;

using Vec3i = Vector<int32_t, 3>;

} // namespace AP
