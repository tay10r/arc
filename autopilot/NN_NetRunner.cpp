#include "NN_NetRunner.h"

#include "NN_Net.h"

#include <assert.h>

namespace NN {

NetRunner::NetRunner(const Net* net)
  : net_(net)
{
}

auto
NetRunner::getRegister(const uint8_t reg) -> uint8_t*
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

  for (uint32_t i = 0; i < static_cast<uint32_t>(expr.outFeatures); i++) {

    uint32_t out{};

    for (uint32_t j = 0; j < static_cast<uint32_t>(expr.inFeatures); j++) {
      const auto in = static_cast<uint32_t>(input[j]);
      const auto weight = static_cast<uint32_t>(params[i * static_cast<uint32_t>(expr.inFeatures) + j]);
      const auto v = in * weight;
      out += static_cast<uint32_t>(v);
    }

    // TODO: fix normalization
    // TODO: add bias
    // out <<= 8 + (1 + (inFeatures));
    out >>= 8 + 3;

    assert(out < 256);

    net_->regs[currentReg_][i] = static_cast<uint8_t>(out);
  }

  currentParameters_ += expr.inFeatures * expr.outFeatures + expr.outFeatures;
}

void
NetRunner::interpret(const ReLUExpr& expr)
{
  const auto numFeatures = regSizes_[expr.inRegister];

  const auto* input = net_->regs[expr.inRegister];

  auto* output = net_->regs[currentReg_];

  for (uint32_t i = 0; i < numFeatures; i++) {
    const auto in = input[i];
    output[i] = (in > 128) ? in : 128;
  }
}

} // namespace NN
