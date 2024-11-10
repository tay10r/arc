#include "NN_NetBuilder.h"

#include <stdlib.h>

namespace NN {

NetBuilder::NetBuilder(Net* net, const uint16_t inputSize)
  : net_(net)
{
  net_->regSizes[0] = inputSize;
}

auto
NetBuilder::finish() -> bool
{
  size_t allocSize = net_->numParameters;
  for (uint8_t i = 0; i < NN_MAX_REGS; i++) {
    allocSize += net_->regSizes[i];
  }
  auto* memory = static_cast<uint8_t*>(malloc(allocSize));
  if (!memory) {
    return false;
  }
  net_->parameters = memory;
  memory += net_->numParameters;
  for (uint8_t i = 0; i < NN_MAX_REGS; i++) {
    net_->regs[i] = memory;
    memory += net_->regSizes[i];
  }
  return true;
}

void
NetBuilder::beginAssignment(const uint8_t dstReg)
{
  currentReg_ = dstReg;
}

void
NetBuilder::interpret(const LinearExpr& expr)
{
  expandCurrentRegSize(expr.outFeatures);
  net_->numParameters += expr.inFeatures * expr.outFeatures + expr.outFeatures;
}

void
NetBuilder::interpret(const ReLUExpr& expr)
{
  expandCurrentRegSize(net_->regSizes[expr.inRegister]);
}

void
NetBuilder::expandCurrentRegSize(const uint16_t size)
{
  const auto v = net_->regSizes[currentReg_];
  net_->regSizes[currentReg_] = (v < size) ? size : v;
}

} // namespace NN
