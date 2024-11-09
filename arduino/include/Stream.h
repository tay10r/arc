#pragma once

#include "Print.h"

class Stream : public Print
{
public:
  ~Stream() override = default;

  virtual auto available() -> int = 0;

  [[nodiscard]] virtual auto read() -> int = 0;
};
