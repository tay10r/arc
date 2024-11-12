#pragma once

#include <stdint.h>

namespace NN {

class Interpreter;
class Lexer;
struct Token;
struct Expr;

enum class SyntaxError : uint8_t
{
  kNone,
  kUnexpectedToken,
  kRegisterOutOfBounds,
  kNumberOutOfBounds,
  kUnknownFunction,
  kInvalidOperand
};

class Parser final
{
public:
  Parser(Interpreter* interp);

  [[nodiscard]] auto parse(Lexer& lexer) -> SyntaxError;

protected:
  [[nodiscard]] auto iterateParse(Lexer& lexer) -> SyntaxError;

  [[nodiscard]] auto parseAssignment(Lexer& lexer) -> SyntaxError;

  [[nodiscard]] auto parseExpr(Lexer& lexer) -> SyntaxError;

  [[nodiscard]] auto parseFunctionExpr(Lexer& lexer, const Token& funcId) -> SyntaxError;

  [[nodiscard]] auto parseLinearExpr(Lexer& lexer) -> SyntaxError;

  void interpret(const Expr&);

private:
  Interpreter* interpreter_{};
};

} // namespace NN
