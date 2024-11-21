#include <gtest/gtest.h>

#include <NN_Lexer.h>

#define DECL_LEXER(name, input)                                                                                        \
  const char src[] = input;                                                                                            \
  NN::Lexer lexer(src, sizeof(src) - 1)

TEST(Lexer, EndOfFile)
{
  DECL_LEXER(lexer, "");
  EXPECT_EQ(lexer.lex().kind, NN::TokenKind::kNone);
}

TEST(Lexer, Symbol)
{
  DECL_LEXER(lexer, "=");
  EXPECT_EQ(lexer.lex().kind, NN::TokenKind::kSymbol);
}

TEST(Lexer, Space)
{
  DECL_LEXER(lexer, " ");
  EXPECT_EQ(lexer.lex().kind, NN::TokenKind::kIgnore);
}

TEST(Lexer, Tab)
{
  DECL_LEXER(lexer, "\t");
  EXPECT_EQ(lexer.lex().kind, NN::TokenKind::kIgnore);
}

TEST(Lexer, CR)
{
  DECL_LEXER(lexer, "\r");
  EXPECT_EQ(lexer.lex().kind, NN::TokenKind::kIgnore);
}

TEST(Lexer, LF)
{
  DECL_LEXER(lexer, "\n");
  EXPECT_EQ(lexer.lex().kind, NN::TokenKind::kNewline);
}

TEST(Lexer, CRLF)
{
  DECL_LEXER(lexer, "\r\n ");
  const auto token = lexer.lex();
  EXPECT_EQ(token, NN::TokenKind::kNewline);
  EXPECT_EQ(token.length, 2);
}

TEST(Lexer, Identifier)
{
  DECL_LEXER(lexer, "Linear ");
  const auto token = lexer.lex();
  EXPECT_EQ(token.kind, NN::TokenKind::kIdentifier);
  EXPECT_EQ(token.length, 6);
}

TEST(Lexer, Register)
{
  DECL_LEXER(lexer, "%12 ");
  const auto token = lexer.lex();
  EXPECT_EQ(token.kind, NN::TokenKind::kRegister);
  EXPECT_EQ(token.length, 3);
}

TEST(Lexer, Number)
{
  DECL_LEXER(lexer, "3141");
  const auto token = lexer.lex();
  EXPECT_EQ(token.kind, NN::TokenKind::kNumber);
  EXPECT_EQ(token.length, 4);
}
