#include "AP_LinAlg.h"

namespace AP {

template<typename Scalar, int Dim>
auto
Vector<Scalar, Dim>::operator+(const Vector& other) const -> Vector
{
  Vector result;
  for (auto i = 0; i < Dim; i++) {
    result.data[i] = data[i] + other.data[i];
  }
  return result;
}

template<typename Scalar, int Dim>
auto
Vector<Scalar, Dim>::operator-(const Vector& other) const -> Vector
{
  Vector result;
  for (auto i = 0; i < Dim; i++) {
    result.data[i] = data[i] - other.data[i];
  }
  return result;
}

template<typename Scalar, int Dim>
auto
Vector<Scalar, Dim>::operator[](const int i) -> Scalar&
{
  return data[i];
}

template<typename Scalar, int Dim>
auto
Vector<Scalar, Dim>::operator[](const int i) const -> const Scalar&
{
  return data[i];
}

template struct Vector<float, 2>;

template struct Vector<float, 3>;

template struct Vector<int, 2>;

template struct Vector<int, 3>;

} // namespace AP
