#pragma once

#include <stddef.h>
#include <stdint.h>

class Print
{
public:
  virtual ~Print() = default;

  [[nodiscard]] virtual auto availableForWrite() -> int = 0;

  [[nodiscard]] virtual auto write(uint8_t) -> size_t = 0;
};
