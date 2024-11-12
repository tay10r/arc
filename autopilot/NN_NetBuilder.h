#pragma once

#include "NN_Interpreter.h"
#include "NN_Net.h"

#include <stdint.h>

namespace NN {

class NetBuilder final : public Interpreter
{
public:
  NetBuilder(Net* net, uint16_t inputSize);

  void beginAssignment(uint8_t dstReg) override;

  void interpret(const LinearExpr&) override;

  void interpret(const ConcatExpr&) override;

  void interpret(const CompAddExpr&) override;

  void interpret(const CompMulExpr&) override;

  void interpret(const ReLUExpr&) override;

  void interpret(const SigmoidExpr&) override;

  void interpret(const TanhExpr&) override;

  [[nodiscard]] auto finish() -> bool;

protected:
  void expandCurrentRegSize(uint16_t size);

private:
  Net* net_{};

  uint8_t currentReg_{};
};

} // namespace NN
