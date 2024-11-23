#include <gtest/gtest.h>

#include <random>

#include <NN_Lexer.h>
#include <NN_Loss.h>
#include <NN_NetBuilder.h>
#include <NN_NetRunner.h>
#include <NN_Optim.h>
#include <NN_Parser.h>

#include <cstring>

#include <iostream>

namespace {

auto
buildNet(const std::string& source, const uint16_t inputSize) -> NN::Net
{
  NN::Net net;
  NN::Lexer lexer(source.c_str(), static_cast<uint16_t>(source.size()));
  NN::NetBuilder builder(&net, inputSize);
  NN::Parser parser(&builder);
  const auto err = parser.parse(lexer);
  EXPECT_EQ(err, NN::SyntaxError::kNone);
  EXPECT_TRUE(builder.finish());
  return net;
}

} // namespace

TEST(NetBuilder, Minimal)
{
  auto net = buildNet("%1 = Linear 16 32 %0\n"
                      "%2 = ReLU %1\n",
                      16);

  EXPECT_EQ(net.numParameters, 16 * 32 + 32);
  EXPECT_EQ(net.regSizes[0], 16);
  EXPECT_EQ(net.regSizes[1], 32);
  EXPECT_EQ(net.regSizes[2], 32);

  net.releaseMemory();
}

namespace {

class XorTest final
{
public:
  XorTest(const std::string& src)
    : source_(src)
    , net_(buildNet(source_, 8))
  {
  }

  ~XorTest()
  {
    net_.releaseMemory();

    optimizer_.releaseMemory();
  }

  void trainNet(const int numIterations)
  {
    randomizeWeights(net_);

    EXPECT_TRUE(optimizer_.allocMemory());

    for (auto i = 0; i < numIterations; i++) {
      (void)optimizer_.step(this, randomInt, randomFloat, this, computeLoss);
    }
  }

  void eval(const float* op1, const float* op2, float* out)
  {
    auto* in = runner_.getRegister(0);
    std::memcpy(in, op1, 4 * sizeof(float));
    std::memcpy(in + 4, op2, 4 * sizeof(float));

    runner_.reset();

    EXPECT_EQ(NN::exec(source_.c_str(), source_.size(), runner_), NN::SyntaxError::kNone);

    std::memcpy(out, runner_.getRegister(1), 4 * sizeof(float));
  }

protected:
  static void bitsToFloat(const int bits, float* out)
  {
    out[0] = (bits & 1) ? 1.0F : 0.0F;
    out[1] = (bits & 2) ? 1.0F : 0.0F;
    out[2] = (bits & 4) ? 1.0F : 0.0F;
    out[3] = (bits & 8) ? 1.0F : 0.0F;
  }

  [[nodiscard]] static auto computeLoss(void* selfPtr, const NN::Net& net) -> float
  {
    auto* self = static_cast<XorTest*>(selfPtr);

    const auto numSamples = 512;

    std::uniform_int_distribution<int> opDist(0, 15);

    float lossSum{};

    for (auto i = 0; i < numSamples; i++) {

      const auto a = opDist(self->rng_);
      const auto b = opDist(self->rng_);

      bitsToFloat(a, self->runner_.getRegister(0));
      bitsToFloat(b, self->runner_.getRegister(0) + 4);

      self->runner_.reset();

      EXPECT_EQ(NN::exec(self->source_.c_str(), self->source_.size(), self->runner_), NN::SyntaxError::kNone);

      const auto* output = self->runner_.getRegister(1);

      float expected[4]{};

      bitsToFloat(a ^ b, expected);

      const auto loss = NN::mseLoss(output, expected, 4);

      lossSum += loss;
    }

    return lossSum / numSamples;
  }

  [[nodiscard]] static auto randomInt(void* selfPtr, int32_t minVal, int32_t maxVal) -> int32_t
  {
    std::uniform_int_distribution<int32_t> dist(minVal, maxVal - 1);
    return dist(static_cast<XorTest*>(selfPtr)->rng_);
  }

  [[nodiscard]] static auto randomFloat(void* selfPtr, float minVal, float maxVal) -> float
  {
    std::uniform_real_distribution<float> dist(minVal, maxVal);
    return dist(static_cast<XorTest*>(selfPtr)->rng_);
  };

  void randomizeWeights(NN::Net& net)
  {
    auto rngFunc = [](void* rngPtr) -> float {
      std::uniform_real_distribution<float> dist(-1, 1);
      const auto v = dist(*static_cast<std::mt19937*>(rngPtr));
      return v;
    };

    net.randomize(&rng_, rngFunc);
  }

private:
  std::string source_;

  std::mt19937 rng_{ 0 };

  NN::Net net_;

  NN::NetRunner runner_{ &net_ };

  NN::LSOptimizer optimizer_{ &net_, /*batch_size=*/4 };
};

} // namespace

TEST(NetRunner, LearnXor)
{
  return;

  const char src[] = "%2 = Linear 8 8 %0\n"
                     "%3 = Linear 8 4 %2\n"
                     "%1 = Sigmoid %3\n";

  XorTest test(src);

  test.trainNet(/*numIterations=*/10'000);

  const float op1[4]{ 1, 0, 1, 1 };
  const float op2[4]{ 0, 0, 1, 0 };
  float output[4]{};

  test.eval(op1, op2, output);

  EXPECT_NEAR(output[0], 1.0F, 0.45F);
  EXPECT_NEAR(output[1], 0.0F, 0.45F);
  EXPECT_NEAR(output[2], 0.0F, 0.45F);
  EXPECT_NEAR(output[3], 1.0F, 0.45F);
}
