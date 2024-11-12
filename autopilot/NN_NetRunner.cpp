#include "NN_NetRunner.h"

#include "NN_Net.h"

#include <math.h>

namespace NN {

NetRunner::NetRunner(const Net* net)
  : net_(net)
{
}

auto
NetRunner::getRegister(const uint8_t reg) -> float*
{
  return net_->regs[reg];
}

void
NetRunner::reset()
{
  currentReg_ = 0;
  currentParameters_ = net_->parameters;
}

void
NetRunner::beginAssignment(uint8_t dstReg)
{
  currentReg_ = dstReg;
}

void
NetRunner::interpret(const LinearExpr& expr)
{
  const auto* input = net_->regs[expr.inRegister];

  const auto* params = currentParameters_;

  const auto* weights = params;

  const auto* bias = weights + expr.inFeatures * expr.outFeatures;

  for (uint32_t i = 0; i < static_cast<uint32_t>(expr.outFeatures); i++) {

    float out{};

    for (uint32_t j = 0; j < static_cast<uint32_t>(expr.inFeatures); j++) {
      const auto in = input[j];
      const auto weight = params[i * static_cast<uint32_t>(expr.inFeatures) + j];
      const auto v = in * weight;
      out += v;
    }

    net_->regs[currentReg_][i] = out + bias[i];
  }

  regSizes_[currentReg_] = expr.outFeatures;

  currentParameters_ += expr.inFeatures * expr.outFeatures + expr.outFeatures;
}

void
NetRunner::interpret(const ConcatExpr& expr)
{
  const auto* leftOp = net_->regs[expr.leftOpReg];
  const auto* rightOp = net_->regs[expr.rightOpReg];
  auto* output = net_->regs[currentReg_];
  const auto lSize = regSizes_[expr.leftOpReg];
  const auto rSize = regSizes_[expr.rightOpReg];

  for (uint32_t i = 0; i < lSize; i++) {
    output[i] = leftOp[i];
  }

  for (uint32_t i = 0; i < rSize; i++) {
    output[lSize + i] = rightOp[i];
  }

  regSizes_[currentReg_] = lSize + rSize;
}

void
NetRunner::interpret(const CompAddExpr& expr)
{
  const auto* leftOp = net_->regs[expr.leftOpReg];
  const auto* rightOp = net_->regs[expr.rightOpReg];
  auto* output = net_->regs[currentReg_];
  const auto lSize = regSizes_[expr.leftOpReg];
  const auto rSize = regSizes_[expr.rightOpReg];

  const auto minSize = (lSize < rSize) ? lSize : rSize;

  for (uint32_t i = 0; i < minSize; i++) {
    output[i] = leftOp[i] + rightOp[i];
  }

  regSizes_[currentReg_] = lSize + rSize;
}

void
NetRunner::interpret(const CompMulExpr& expr)
{
  const auto* leftOp = net_->regs[expr.leftOpReg];
  const auto* rightOp = net_->regs[expr.rightOpReg];
  auto* output = net_->regs[currentReg_];
  const auto lSize = regSizes_[expr.leftOpReg];
  const auto rSize = regSizes_[expr.rightOpReg];

  const auto minSize = (lSize < rSize) ? lSize : rSize;

  for (uint32_t i = 0; i < minSize; i++) {
    output[i] = leftOp[i] * rightOp[i];
  }

  regSizes_[currentReg_] = lSize + rSize;
}

void
NetRunner::interpret(const ReLUExpr& expr)
{
  const auto numFeatures = regSizes_[expr.inRegister];

  const auto* input = net_->regs[expr.inRegister];

  auto* output = net_->regs[currentReg_];

  for (uint32_t i = 0; i < numFeatures; i++) {
    const auto in = input[i];
    output[i] = (in >= 0.0F) ? in : 0.0F;
  }

  regSizes_[currentReg_] = regSizes_[expr.inRegister];
}

void
NetRunner::interpret(const SigmoidExpr& expr)
{
  const auto numFeatures = regSizes_[expr.inRegister];
  const auto* input = net_->regs[expr.inRegister];
  auto* output = net_->regs[currentReg_];

  for (uint32_t i = 0; i < numFeatures; i++) {
    const auto ex = expf(input[i]);
    output[i] = ex / (1.0F + ex);
  }

  regSizes_[currentReg_] = regSizes_[expr.inRegister];
}

void
NetRunner::interpret(const TanhExpr& expr)
{
  const auto numFeatures = regSizes_[expr.inRegister];
  const auto* input = net_->regs[expr.inRegister];
  auto* output = net_->regs[currentReg_];

  for (uint32_t i = 0; i < numFeatures; i++) {
    const auto a = expf(input[i]);
    const auto b = expf(-input[i]);
    output[i] = (a - b) / (a + b);
  }
  regSizes_[currentReg_] = regSizes_[expr.inRegister];
}

} // namespace NN
