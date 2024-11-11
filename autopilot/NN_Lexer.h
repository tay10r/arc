#pragma once

#include <stdint.h>

namespace NN {

enum class TokenKind : uint8_t
{
  kNone,
  kRegister,
  kIdentifier,
  kNumber,
  kSymbol,
  kIgnore
};

struct Token final
{
  TokenKind kind{ TokenKind::kNone };

  uint8_t length{};

  uint16_t offset{};

  auto operator==(const TokenKind otherKind) const -> bool;

  auto operator!=(const TokenKind otherKind) const -> bool;
};

class Lexer final
{
public:
  Lexer(const char* source, uint16_t length);

  [[nodiscard]] auto lex() -> Token;

  [[nodiscard]] auto remaining() const -> uint16_t;

  [[nodiscard]] auto toPointer(const uint16_t offset) const -> const char*;

protected:
  [[nodiscard]] auto produceToken(TokenKind kind, uint8_t length) -> Token;

private:
  const char* source_{};

  uint16_t length_{};

  uint16_t offset_{};
};

} // namespace NN