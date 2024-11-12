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

} // namespace NN
