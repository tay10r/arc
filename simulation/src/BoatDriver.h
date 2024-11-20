#pragma once

#include <sim/Env.h>

namespace sim {

auto
createBoatDriver(int seed) -> std::unique_ptr<Env>;

} // namespace sim
