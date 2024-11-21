#include "NN_Interpreter.h"

#include "NN_Lexer.h"
#include "NN_Parser.h"

namespace NN {

void
LinearExpr::accept(Interpreter& interp) const
{
  interp.interpret(*this);
}

void
MatMulExpr::accept(Interpreter& interp) const
{
  interp.interpret(*this);
}

void
ConcatExpr::accept(Interpreter& interp) const
{
  interp.interpret(*this);
}

void
CompAddExpr::accept(Interpreter& interp) const
{
  interp.interpret(*this);
}

void
CompMulExpr::accept(Interpreter& interp) const
{
  interp.interpret(*this);
}

void
ReLUExpr::accept(Interpreter& interp) const
{
  interp.interpret(*this);
}

void
SigmoidExpr::accept(Interpreter& interp) const
{
  interp.interpret(*this);
}

void
TanhExpr::accept(Interpreter& interp) const
{
  interp.interpret(*this);
}

auto
exec(const char* source, const uint16_t length, Interpreter& interp) -> SyntaxError
{
  Lexer lexer(source, length);

  Parser parser(&interp);

  return parser.parse(lexer);
}

auto
reverseExec(const char* source, const uint16_t length, Interpreter& interp) -> SyntaxError
{
  // Execute each line of code, starting from the last until the first.

  auto lastEnd = length;

  for (uint8_t i = length; i > 0; i--) {
    const auto c = source[i - 1];
    const auto first = i == 1;
    if ((c != '\n') && !first) {
      continue;
    }
    const auto offset = first ? 0 : i;
    const auto err = exec(source + offset, lastEnd - offset, interp);
    if (err != SyntaxError::kNone) {
      return err;
    }
    lastEnd = i - 1;
  }

  return SyntaxError::kNone;
}

} // namespace NN
