#include <gtest/gtest.h>

#include <NN_Interpreter.h>
#include <NN_Lexer.h>
#include <NN_Parser.h>

#include <sstream>

namespace {

class NullInterpreter final : public NN::Interpreter
{
public:
  void beginAssignment(const uint8_t) override {}

  void interpret(const NN::LinearExpr&) override {}

  void interpret(const NN::ConcatExpr&) override {}

  void interpret(const NN::CompAddExpr&) override {}

  void interpret(const NN::CompMulExpr&) override {}

  void interpret(const NN::ReLUExpr&) override {}

  void interpret(const NN::SigmoidExpr&) override {}

  void interpret(const NN::TanhExpr&) override {}
};

auto
runErrorTest(const std::string& text)
{
  NN::Lexer lexer(text.c_str(), text.size());
  NullInterpreter interpreter;
  NN::Parser parser(&interpreter);
  return parser.parse(lexer);
}

} // namespace

TEST(Parser, UnexpectedToken)
{
  EXPECT_EQ(runErrorTest("Linear "), NN::SyntaxError::kUnexpectedToken);
}

TEST(Parser, UnexpectedTokenAssignment)
{
  // The '+' should be a '='
  EXPECT_EQ(runErrorTest("%1 +"), NN::SyntaxError::kUnexpectedToken);
}

TEST(Parser, RegisterOutOfBounds)
{
  EXPECT_EQ(runErrorTest("%256 = ReLU %0"), NN::SyntaxError::kRegisterOutOfBounds);
}

TEST(Parser, UnknownFunction)
{
  EXPECT_EQ(runErrorTest("%1 = Foo %0"), NN::SyntaxError::kUnknownFunction);
}

TEST(Parser, InvalidOperand)
{
  EXPECT_EQ(runErrorTest("%1 = ReLU 256"), NN::SyntaxError::kInvalidOperand);
}

TEST(Parser, ReLUExpr)
{
  EXPECT_EQ(runErrorTest("%1 = ReLU %0"), NN::SyntaxError::kNone);
}

TEST(Parser, LinearExpr)
{
  EXPECT_EQ(runErrorTest("%1 = Linear 16 32 %0"), NN::SyntaxError::kNone);
}

TEST(Parser, NumberOutOfBounds)
{
  EXPECT_EQ(runErrorTest("%1 = Linear 1000000 256 %0"), NN::SyntaxError::kNumberOutOfBounds);
}

namespace {

class Printer final : public NN::Interpreter
{
public:
  auto getString() const -> std::string { return stream_.str(); }

  void beginAssignment(uint8_t dstReg) override { stream_ << '%' << static_cast<int>(dstReg) << " = "; }

  void interpret(const NN::LinearExpr& expr) override
  {
    stream_ << "Linear " << static_cast<int>(expr.inFeatures) << ' ' << static_cast<int>(expr.outFeatures) << " %"
            << static_cast<int>(expr.inRegister) << '\n';
  }

  void interpret(const NN::ConcatExpr& expr) override
  {
    //
  }

  void interpret(const NN::CompAddExpr& expr) override
  {
    //
  }

  void interpret(const NN::CompMulExpr& expr) override
  {
    //
  }

  void interpret(const NN::ReLUExpr& expr) override
  {
    stream_ << "ReLU %" << static_cast<int>(expr.inRegister) << '\n';
  }

  void interpret(const NN::SigmoidExpr&) override {}

  void interpret(const NN::TanhExpr&) override {}

private:
  std::ostringstream stream_;
};

} // namespace

TEST(Parser, Full)
{
  const char src[] = R"(
  %1 = Linear 4 16 %0
  %2 = ReLU %1
  )";
  NN::Lexer lexer(src, sizeof(src) - 1);
  Printer printer;
  NN::Parser parser(&printer);
  const auto err = parser.parse(lexer);
  EXPECT_EQ(err, NN::SyntaxError::kNone);
  EXPECT_EQ(printer.getString(),
            "%1 = Linear 4 16 %0\n"
            "%2 = ReLU %1\n");
}
