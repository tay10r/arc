#include "NN_Parser.h"

#include "NN_Interpreter.h"
#include "NN_Lexer.h"

#include <string.h>

namespace NN {

namespace {

[[nodiscard]] auto
matchIdentifier(const char* aPtr, const uint16_t aLen, const char* bPtr, const uint16_t bLen) -> bool
{
  return (aLen == bLen) && (memcmp(aPtr, bPtr, aLen) == 0);
}

enum class KnownIdentifier : uint8_t
{
  kUnknown,
  kLinear,
  kConcat,
  kCompAdd,
  kCompMul,
  kReLU,
  kSigmoid,
  kTanh
};

[[nodiscard]] auto
parseIdentifier(const Lexer& lexer, const Token& identifier) -> KnownIdentifier
{
  const char* idPtr = lexer.toPointer(identifier.offset);

#define MATCH_IDENTIFIER(enumValue, text)                                                                              \
  do {                                                                                                                 \
    if (matchIdentifier(idPtr, identifier.length, text, sizeof(text) - 1)) {                                           \
      return KnownIdentifier::enumValue;                                                                               \
    }                                                                                                                  \
  } while (0)

  MATCH_IDENTIFIER(kLinear, "Linear");
  MATCH_IDENTIFIER(kConcat, "Concat");
  MATCH_IDENTIFIER(kCompAdd, "CompAdd");
  MATCH_IDENTIFIER(kCompMul, "CompMul");
  MATCH_IDENTIFIER(kReLU, "ReLU");
  MATCH_IDENTIFIER(kSigmoid, "Sigmoid");
  MATCH_IDENTIFIER(kTanh, "Tanh");

#undef MATCH_IDENTIFIER

  return KnownIdentifier::kUnknown;
}

[[nodiscard]] auto
nextToken(Lexer& lexer) -> Token
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

[[nodiscard]] auto
parseNumber(const Lexer& lexer, const Token& token, uint16_t* out) -> SyntaxError
{
  const auto* ptr = lexer.toPointer(token.offset);
  const auto len = token.length;
  uint16_t v{};
  for (uint8_t i = 0; i < len; i++) {
    const auto c = ptr[i];
    auto next = static_cast<uint32_t>(v);
    next *= 10;
    next += static_cast<uint8_t>(c - '0');
    if (next > 65535) {
      // overflow
      return SyntaxError::kNumberOutOfBounds;
    }
    v = next;
  }
  *out = v;
  return SyntaxError::kNone;
}

[[nodiscard]] auto
parseRegister(const Lexer& lexer, const Token& regToken, uint8_t* out) -> SyntaxError
{
  // +1 since the first character is '%'
  const auto* ptr = lexer.toPointer(regToken.offset) + 1;
  const auto len = regToken.length - 1;
  uint8_t v{};
  for (uint8_t i = 0; i < len; i++) {
    const auto c = ptr[i];
    auto next = static_cast<uint16_t>(v);
    next *= 10;
    next += static_cast<uint8_t>(c - '0');
    if (next > 255) {
      // overflow
      return SyntaxError::kRegisterOutOfBounds;
    }
    v = static_cast<uint8_t>(next);
  }
  *out = v;
  return SyntaxError::kNone;
}

} // namespace

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

template<typename ExprT>
auto
parseActivation(const Lexer& lexer, const Token& regToken, Interpreter& interp) -> SyntaxError
{
  if (regToken.kind != TokenKind::kRegister) {
    return SyntaxError::kInvalidOperand;
  }

  ExprT expr;

  const auto err = parseRegister(lexer, regToken, &expr.inRegister);
  if (err != SyntaxError::kNone) {
    return err;
  }

  expr.accept(interp);

  return SyntaxError::kNone;
}

template<typename BinaryExprT>
auto
parseBinaryExpr(Lexer& lexer, Interpreter& interp) -> SyntaxError
{
  BinaryExprT expr;

  const auto lToken = nextToken(lexer);
  if (lToken != TokenKind::kRegister) {
    return SyntaxError::kInvalidOperand;
  }
  auto err = parseRegister(lexer, lToken, &expr.leftOpReg);
  if (err != SyntaxError::kNone) {
    return err;
  }

  const auto rToken = nextToken(lexer);
  if (rToken != TokenKind::kRegister) {
    return SyntaxError::kInvalidOperand;
  }
  err = parseRegister(lexer, rToken, &expr.rightOpReg);
  if (err != SyntaxError::kNone) {
    return err;
  }

  expr.accept(interp);

  return SyntaxError::kNone;
}

} // namespace

auto
Parser::parseFunctionExpr(Lexer& lexer, const Token& funcId) -> SyntaxError
{
  const auto id = parseIdentifier(lexer, funcId);
  switch (id) {
    case KnownIdentifier::kUnknown:
      break;
    case KnownIdentifier::kLinear:
      return parseLinearExpr(lexer);
    case KnownIdentifier::kConcat:
      return parseBinaryExpr<ConcatExpr>(lexer, *interpreter_);
    case KnownIdentifier::kCompAdd:
      return parseBinaryExpr<CompAddExpr>(lexer, *interpreter_);
    case KnownIdentifier::kCompMul:
      return parseBinaryExpr<CompMulExpr>(lexer, *interpreter_);
    case KnownIdentifier::kReLU:
      return parseActivation<ReLUExpr>(lexer, nextToken(lexer), *interpreter_);
    case KnownIdentifier::kSigmoid:
      return parseActivation<SigmoidExpr>(lexer, nextToken(lexer), *interpreter_);
    case KnownIdentifier::kTanh:
      return parseActivation<TanhExpr>(lexer, nextToken(lexer), *interpreter_);
  }
  return SyntaxError::kUnknownFunction;
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

void
Parser::interpret(const Expr& expr)
{
  expr.accept(*interpreter_);
}

} // namespace NN
