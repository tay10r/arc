#pragma once

#include <stdint.h>

namespace NN {

enum class SyntaxError : uint8_t;

struct LinearExpr;
struct ReLUExpr;

class Interpreter
{
public:
  virtual ~Interpreter() = default;

  virtual void beginAssignment(uint8_t dstReg) = 0;

  virtual void interpret(const LinearExpr&) = 0;

  virtual void interpret(const ReLUExpr&) = 0;
};

struct Expr
{
  virtual ~Expr() = default;

  virtual void accept(Interpreter&) const = 0;
};

struct LinearExpr final : public Expr
{
  uint16_t inFeatures{};

  uint16_t outFeatures{};

  uint8_t inRegister{};

  void accept(Interpreter& interp) const;
};

struct ReLUExpr final : public Expr
{
  uint8_t inRegister{};

  void accept(Interpreter& interp) const;
};

[[nodiscard]] auto
exec(const char* source, const uint16_t length, Interpreter& interp) -> SyntaxError;

} // namespace NN
