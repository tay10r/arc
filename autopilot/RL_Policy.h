#pragma once

namespace RL {

struct State final
{
  float rotation[9]{};

  float altitudeError{};

  float speedError[2]{};
};

struct Action final
{
  float actuators[16]{};
};

class Policy
{
public:
  virtual ~Policy() = default;

  virtual void computeAction(const State& state, Action& action) = 0;

  virtual void reset() = 0;
};

} // namespace RL
