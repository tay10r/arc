#pragma once

#include "NN_Interpreter.h"
#include "NN_Net.h"

namespace NN {

class NetRunner final : public Interpreter
{
public:
  NetRunner(const Net* net);

  [[nodiscard]] auto getRegister(uint8_t reg) -> float*;

  void reset();

  void beginAssignment(const uint8_t dstReg) override;

  void interpret(const LinearExpr&) override;

  void interpret(const ReLUExpr&) override;

private:
  const Net* net_{};

  /**
   * @brief Where to write the next output.
   * */
  uint8_t currentReg_{};

  /**
   * @brief The current set of weights and bias.
   * */
  float* currentParameters_{};

  /**
   * @brief The current size of each register.
   * */
  uint16_t regSizes_[NN_MAX_REGS]{};
};

} // namespace NN
