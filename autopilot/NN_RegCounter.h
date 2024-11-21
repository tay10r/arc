#pragma once

#include "NN_Interpreter.h"

#include <stdint.h>

namespace NN {

class RegCounter final : public Interpreter
{
public:
  [[nodiscard]] auto getRegCount() const -> uint16_t;

  void beginAssignment(uint8_t dstReg) override;

  void interpret(const LinearExpr& expr) override;

  void interpret(const MatMulExpr& expr) override;

  void interpret(const ConcatExpr& expr) override;

  void interpret(const CompAddExpr& expr) override;

  void interpret(const CompMulExpr& expr) override;

  void interpret(const ReLUExpr& expr) override;

  void interpret(const SigmoidExpr& expr) override;

  void interpret(const TanhExpr& expr) override;

protected:
  void count(const UnaryExpr& expr);

  void count(const BinaryExpr& expr);

  void updateMaxReg(uint8_t reg);

private:
  uint8_t numRegs_{};
};

} // namespace NN
