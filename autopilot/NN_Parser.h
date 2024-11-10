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

enum class KnownIdentifier : uint8_t
{
  kUnknown,
  kLinear,
  kReLU
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

  [[nodiscard]] auto parseReLUExpr(Lexer& lexer) -> SyntaxError;

  [[nodiscard]] auto parseLinearExpr(Lexer& lexer) -> SyntaxError;

  [[nodiscard]] static auto parseIdentifier(const Lexer& lexer, const Token& identifier) -> KnownIdentifier;

  [[nodiscard]] static auto parseNumber(const Lexer& lexer, const Token& token, uint16_t* out) -> SyntaxError;

  [[nodiscard]] static auto parseRegister(const Lexer& lexer, const Token& regToken, uint8_t* out) -> SyntaxError;

  [[nodiscard]] static auto nextToken(Lexer& lexer) -> Token;

  void interpret(const Expr&);

private:
  Interpreter* interpreter_{};
};

} // namespace NN
