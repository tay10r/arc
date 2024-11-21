#pragma once

#include <memory>

class GcsTest
{
public:
  static auto create() -> std::unique_ptr<GcsTest>;

  virtual ~GcsTest() = default;

  virtual void close() = 0;

  virtual void render() = 0;
};
