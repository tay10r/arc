#include "NN_Parser.h"

#include "NN_Interpreter.h"
#include "NN_Lexer.h"

#include <string.h>

namespace NN {

Parser::Parser(Interpreter* interp)
  : interpreter_(interp)
{
}

auto
Parser::parse(Lexer& lexer) -> SyntaxError
{
  while (lexer.remaining() > 0) {
    const auto err = iterateParse(lexer);
    if (err != SyntaxError::kNone) {
      return err;
    }
  }

  return SyntaxError::kNone;
}

[[nodiscard]] auto
Parser::iterateParse(Lexer& lexer) -> SyntaxError
{
  const auto first = nextToken(lexer);
  switch (first.kind) {
    case TokenKind::kRegister: {
      uint8_t dstReg{};
      auto err = parseRegister(lexer, first, &dstReg);
      if (err != SyntaxError::kNone) {
        return err;
      }
      interpreter_->beginAssignment(dstReg);
      return parseAssignment(lexer);
    }
    case TokenKind::kIgnore:
      // This happens when there is whitespace after the last item in the file.
      // It can be safely ignored.
      break;
    default:
      return SyntaxError::kUnexpectedToken;
  }

  return SyntaxError::kNone;
}

auto
Parser::parseAssignment(Lexer& lexer) -> SyntaxError
{
  const auto equalSign = nextToken(lexer);
  if ((equalSign.kind != TokenKind::kSymbol) || (lexer.toPointer(equalSign.offset)[0] != '=')) {
    return SyntaxError::kUnexpectedToken;
  }

  return parseExpr(lexer);
}

auto
Parser::parseExpr(Lexer& lexer) -> SyntaxError
{
  const auto firstToken = nextToken(lexer);

  switch (firstToken.kind) {
    case TokenKind::kIdentifier:
      return parseFunctionExpr(lexer, firstToken);
    default:
      break;
  }

  return SyntaxError::kUnexpectedToken;
}

namespace {

[[nodiscard]] auto
matchIdentifier(const char* aPtr, const uint16_t aLen, const char* bPtr, const uint16_t bLen) -> bool
{
  return (aLen == bLen) && (memcmp(aPtr, bPtr, aLen) == 0);
}

} // namespace

auto
Parser::parseIdentifier(const Lexer& lexer, const Token& identifier) -> KnownIdentifier
{
  const char* idPtr = lexer.toPointer(identifier.offset);

#define MATCH_IDENTIFIER(enumValue, text)                                                                              \
  do {                                                                                                                 \
    if (matchIdentifier(idPtr, identifier.length, text, sizeof(text) - 1)) {                                           \
      return KnownIdentifier::enumValue;                                                                               \
    }                                                                                                                  \
  } while (0)

  MATCH_IDENTIFIER(kLinear, "Linear");
  MATCH_IDENTIFIER(kReLU, "ReLU");

#undef MATCH_IDENTIFIER

  return KnownIdentifier::kUnknown;
}

auto
Parser::parseFunctionExpr(Lexer& lexer, const Token& funcId) -> SyntaxError
{
  const auto id = parseIdentifier(lexer, funcId);
  switch (id) {
    case KnownIdentifier::kUnknown:
      break;
    case KnownIdentifier::kLinear:
      return parseLinearExpr(lexer);
    case KnownIdentifier::kReLU:
      return parseReLUExpr(lexer);
  }
  return SyntaxError::kUnknownFunction;
}

auto
Parser::parseReLUExpr(Lexer& lexer) -> SyntaxError
{
  const auto regToken = nextToken(lexer);
  if (regToken.kind != TokenKind::kRegister) {
    return SyntaxError::kInvalidOperand;
  }

  ReLUExpr expr;

  const auto err = parseRegister(lexer, regToken, &expr.inRegister);
  if (err != SyntaxError::kNone) {
    return err;
  }

  interpret(expr);

  return SyntaxError::kNone;
}

auto
Parser::parseLinearExpr(Lexer& lexer) -> SyntaxError
{
  LinearExpr expr;

  const auto inFeaturesToken = nextToken(lexer);
  if (inFeaturesToken != TokenKind::kNumber) {
    return SyntaxError::kInvalidOperand;
  }

  auto err = parseNumber(lexer, inFeaturesToken, &expr.inFeatures);
  if (err != SyntaxError::kNone) {
    return err;
  }

  const auto outFeaturesToken = nextToken(lexer);
  if (outFeaturesToken != TokenKind::kNumber) {
    return SyntaxError::kInvalidOperand;
  }

  err = parseNumber(lexer, outFeaturesToken, &expr.outFeatures);
  if (err != SyntaxError::kNone) {
    return err;
  }

  const auto inputToken = nextToken(lexer);
  if (inputToken != TokenKind::kRegister) {
    return SyntaxError::kInvalidOperand;
  }

  err = parseRegister(lexer, inputToken, &expr.inRegister);
  if (err != SyntaxError::kNone) {
    return err;
  }

  interpret(expr);

  return SyntaxError::kNone;
}

#undef CHECK_ID

auto
Parser::nextToken(Lexer& lexer) -> Token
{
  Token token{};
  while (lexer.remaining() > 0) {
    token = lexer.lex();
    if (token.kind != TokenKind::kIgnore) {
      break;
    }
  }
  return token;
}

auto
Parser::parseNumber(const Lexer& lexer, const Token& token, uint16_t* out) -> SyntaxError
{
  const auto* ptr = lexer.toPointer(token.offset);
  const auto len = token.length;
  uint16_t v{};
  for (uint8_t i = 0; i < len; i++) {
    const auto c = ptr[i];
    auto next = v;
    next *= 10;
    next += static_cast<uint8_t>(c - '0');
    if (next < v) {
      // overflow
      return SyntaxError::kNumberOutOfBounds;
    }
    v = next;
  }
  *out = v;
  return SyntaxError::kNone;
}

auto
Parser::parseRegister(const Lexer& lexer, const Token& regToken, uint8_t* out) -> SyntaxError
{
  // +1 since the first character is '%'
  const auto* ptr = lexer.toPointer(regToken.offset) + 1;
  const auto len = regToken.length - 1;
  uint8_t v{};
  for (uint8_t i = 0; i < len; i++) {
    const auto c = ptr[i];
    auto next = v;
    next *= 10;
    next += static_cast<uint8_t>(c - '0');
    if (next < v) {
      // overflow
      return SyntaxError::kRegisterOutOfBounds;
    }
    v = next;
  }
  *out = v;
  return SyntaxError::kNone;
}

void
Parser::interpret(const Expr& expr)
{
  expr.accept(*interpreter_);
}

} // namespace NN
