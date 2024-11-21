#include <gtest/gtest.h>

#include <NN_Parser.h>
#include <NN_RegCounter.h>

TEST(RegCounter, CountZero)
{
  const char src[] = "\n";
  NN::RegCounter counter;
  const auto err = NN::exec(src, sizeof(src) - 1, counter);
  EXPECT_EQ(err, NN::SyntaxError::kNone);
  EXPECT_EQ(counter.getRegCount(), 0);
}

TEST(RegCounter, CountTwo)
{
  const char src[] = "%1 = ReLU %0\n";
  NN::RegCounter counter;
  const auto err = NN::exec(src, sizeof(src) - 1, counter);
  EXPECT_EQ(err, NN::SyntaxError::kNone);
  EXPECT_EQ(counter.getRegCount(), 2);
}

TEST(RegCounter, CountMany)
{
  const char src[] = "%2 = MatMul %0 %1\n"
                     "%3 = ReLU %2\n";
  NN::RegCounter counter;
  const auto err = NN::exec(src, sizeof(src) - 1, counter);
  EXPECT_EQ(err, NN::SyntaxError::kNone);
  EXPECT_EQ(counter.getRegCount(), 4);
}
