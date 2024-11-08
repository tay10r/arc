#pragma once

#include <sim/Env.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace sim {

class EnvRegistry final
{
public:
  EnvRegistry();

  auto createEnv(const std::string& name, int seed = 0) -> std::unique_ptr<Env>;

  auto listEnvs() -> std::vector<std::string>;

protected:
  using FactoryMethod = auto(*)(int seed) -> std::unique_ptr<Env>;

  void registerEnv(std::string name, FactoryMethod method);

private:
  std::map<std::string, FactoryMethod> factories_;
};

} // namespace sim
