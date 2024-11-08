#pragma once

#include <sim/Env.h>

namespace sim {

auto
createLunarLander(int seed) -> std::unique_ptr<Env>;

} // namespace sim
