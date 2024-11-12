#pragma once

#include <stdint.h>

namespace NN {

enum class SyntaxError : uint8_t;

struct LinearExpr;
struct ConcatExpr;
struct CompAddExpr;
struct CompMulExpr;
struct ReLUExpr;
struct SigmoidExpr;
struct TanhExpr;

class Interpreter
{
public:
  virtual ~Interpreter() = default;

  virtual void beginAssignment(uint8_t dstReg) = 0;

  virtual void interpret(const LinearExpr&) = 0;

  virtual void interpret(const ConcatExpr&) = 0;

  virtual void interpret(const CompAddExpr&) = 0;

  virtual void interpret(const CompMulExpr&) = 0;

  virtual void interpret(const ReLUExpr&) = 0;

  virtual void interpret(const SigmoidExpr&) = 0;

  virtual void interpret(const TanhExpr&) = 0;
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

  void accept(Interpreter& interp) const override;
};

struct BinaryExpr : public Expr
{
  uint8_t leftOpReg{};

  uint8_t rightOpReg{};
};

struct ConcatExpr final : public BinaryExpr
{
  void accept(Interpreter& interp) const override;
};

struct CompAddExpr final : public BinaryExpr
{
  void accept(Interpreter& interp) const override;
};

struct CompMulExpr final : public BinaryExpr
{
  void accept(Interpreter& interp) const override;
};

struct UnaryExpr : public Expr
{
  uint8_t inRegister{};

  ~UnaryExpr() override = default;
};

struct ReLUExpr final : public UnaryExpr
{
  void accept(Interpreter& interp) const override;
};

struct SigmoidExpr final : public UnaryExpr
{
  void accept(Interpreter& interp) const override;
};

struct TanhExpr final : public UnaryExpr
{
  void accept(Interpreter& interp) const override;
};

[[nodiscard]] auto
exec(const char* source, const uint16_t length, Interpreter& interp) -> SyntaxError;

} // namespace NN
