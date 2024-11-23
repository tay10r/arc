#include "AP_Random.h"

namespace AP {

namespace {

constexpr uint32_t a{ 48271ul };
constexpr uint32_t c{ 0 };
constexpr uint32_t m{ 2147483647ul };

} // namespace

Random::Random(uint32_t seed)
  : x_((seed == 0) ? 1 : (seed % m))
{
}

auto
Random::operator()() -> uint32_t
{
  x_ = (a * x_ + c) % m;
  return x_;
}

auto
Random::randint(const uint32_t max) -> uint32_t
{
  return (*this)() % (max + 1);
}

} // namespace AP
