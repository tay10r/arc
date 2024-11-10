#include "NN_Lexer.h"

namespace NN {

auto
Token::operator==(const TokenKind otherKind) const -> bool
{
  return kind == otherKind;
}

auto
Token::operator!=(const TokenKind otherKind) const -> bool
{
  return kind != otherKind;
}

namespace {

auto
isDigit(const char c) -> bool
{
  return (c >= '0') && (c <= '9');
}

auto
isAlpha(const char c) -> bool
{
  return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'));
}

} // namespace

Lexer::Lexer(const char* source, uint16_t length)
  : source_(source)
  , length_(length)
{
}

auto
Lexer::lex() -> Token
{
  if (remaining() == 0) {
    return Token{};
  }

  const auto c = source_[offset_];
  if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r')) {
    return produceToken(TokenKind::kIgnore, 1);
  }

  if (c == '%') {
    uint16_t len = 1;
    while ((offset_ + len) < length_) {
      const auto c = source_[offset_ + len];
      if (!isDigit(c)) {
        break;
      }
      len++;
    }
    return produceToken(TokenKind::kRegister, len);
  }

  if (isDigit(c)) {
    uint16_t len = 1;
    while ((offset_ + len) < length_) {
      const auto c = source_[offset_ + len];
      if (!isDigit(c)) {
        break;
      }
      len++;
    }
    return produceToken(TokenKind::kNumber, len);
  }

  if (isAlpha(c)) {
    uint16_t len = 1;
    while ((offset_ + len) < length_) {
      const auto c = source_[offset_ + len];
      if (!(isDigit(c) || isAlpha(c) || (c == '_'))) {
        break;
      }
      len++;
    }
    return produceToken(TokenKind::kIdentifier, len);
  }

  return produceToken(TokenKind::kSymbol, 1);
}

auto
Lexer::toPointer(const uint16_t offset) const -> const char*
{
  return source_ + offset;
}

auto
Lexer::remaining() const -> uint16_t
{
  return (offset_ > length_) ? 0 : (length_ - offset_);
}

auto
Lexer::produceToken(TokenKind kind, const uint8_t length) -> Token
{
  Token token{ kind, length, offset_ };
  offset_ += length;
  return token;
}

} // namespace NN
