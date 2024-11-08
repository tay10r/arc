#pragma once

#include "Print.h"

class Stream : public Print
{
public:
  ~Stream() override = default;
};
