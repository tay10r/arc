#include "NN_RegCounter.h"

namespace NN {

auto
RegCounter::getRegCount() const -> uint16_t
{
  return numRegs_;
}

void
RegCounter::beginAssignment(const uint8_t dstReg)
{
  updateMaxReg(dstReg);
}

void
RegCounter::interpret(const LinearExpr& expr)
{
  updateMaxReg(expr.inRegister);
}

void
RegCounter::interpret(const MatMulExpr& expr)
{
  count(expr);
}

void
RegCounter::interpret(const ConcatExpr& expr)
{
  count(expr);
}

void
RegCounter::interpret(const CompAddExpr& expr)
{
  count(expr);
}

void
RegCounter::interpret(const CompMulExpr& expr)
{
  count(expr);
}

void
RegCounter::interpret(const ReLUExpr& expr)
{
  count(expr);
}

void
RegCounter::interpret(const SigmoidExpr& expr)
{
  count(expr);
}

void
RegCounter::interpret(const TanhExpr& expr)
{
  count(expr);
}

void
RegCounter::count(const UnaryExpr& expr)
{
  updateMaxReg(expr.inRegister);
}

void
RegCounter::count(const BinaryExpr& expr)
{
  updateMaxReg(expr.leftOpReg);
  updateMaxReg(expr.rightOpReg);
}

void
RegCounter::updateMaxReg(const uint8_t reg)
{
  const auto tmpNumRegs = static_cast<uint16_t>(reg) + 1;

  numRegs_ = (tmpNumRegs > numRegs_) ? tmpNumRegs : numRegs_;
}

} // namespace NN
