#pragma once

#include <memory>

class NetTest
{
public:
  static auto create() -> std::unique_ptr<NetTest>;

  virtual ~NetTest() = default;

  virtual void render() = 0;
};
