#include <sim/EnvRegistry.h>

#include "LunarLander.h"

namespace sim {

EnvRegistry::EnvRegistry()
{
  registerEnv("LunarLander", createLunarLander);
}

auto
EnvRegistry::createEnv(const std::string& name, const int seed)
  -> std::unique_ptr<Env>
{
  return factories_.at(name)(seed);
}

auto
EnvRegistry::listEnvs() -> std::vector<std::string>
{
  std::vector<std::string> names;
  for (auto& f : factories_) {
    names.emplace_back(f.first);
  }
  return names;
}

void
EnvRegistry::registerEnv(std::string name, FactoryMethod method)
{
  factories_.emplace(std::move(name), method);
}

} // namespace sim
