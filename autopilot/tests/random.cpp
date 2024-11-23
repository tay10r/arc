#include <AP_Random.h>

#include <gtest/gtest.h>
#include <random>

TEST(Random, seed_zero)
{
  AP::Random rng1(/*seed=*/0);
  std::minstd_rand rng2(/*seed=*/0);
  EXPECT_EQ(rng1(), rng2());
}

TEST(Random, seed_ten)
{
  AP::Random rng1(/*seed=*/10);
  std::minstd_rand rng2(/*seed=*/10);
  EXPECT_EQ(rng1(), rng2());
}
