#include "NN_Lexer.h"

namespace NN {

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

  return produceToken(TokenKind::kSymbol, 1);
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
