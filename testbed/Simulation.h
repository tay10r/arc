#pragma once

#include <sim/Env.h>

#include <memory>

class Simulation
{
public:
  static auto create(std::shared_ptr<sim::Agent> agent) -> std::unique_ptr<Simulation>;

  virtual ~Simulation() = default;

  virtual void setup() = 0;

  virtual void loop(float timeDelta) = 0;
};
