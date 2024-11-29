#pragma once

#include <string>

#include <stdint.h>

#include <engine/game.pb.h>

namespace arc::editor {

auto
formatLabel(uint32_t id, const std::string& str) -> std::string;

auto
formatLabel(uint32_t id, const engine::physics::Collider& c) -> std::string;

auto
generateID(engine::Game& game) -> uint32_t;

void
initTransform(engine::Transform& transform);

} // namespace arc::editor
