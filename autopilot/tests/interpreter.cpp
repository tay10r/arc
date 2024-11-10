#include <gtest/gtest.h>

#include <random>

#include <NN_Lexer.h>
#include <NN_Loss.h>
#include <NN_NetBuilder.h>
#include <NN_NetRunner.h>
#include <NN_Optim.h>
#include <NN_Parser.h>

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

struct XorProblem final
{
  NN::NetRunner* runner;

  const uint8_t* expected;

  const char* source;

  uint16_t sourceLen;
};

} // namespace

TEST(NetRunner, LearnXor)
{
  const char src[] = "%1 = Linear 8 8 %0\n"
                     "%2 = Linear 8 4 %1\n";

  auto net = buildNet(src, 8);

  std::seed_seq seed{ 0 };

  std::mt19937 rng{ seed };

  auto rngFunc = [](void* rngPtr) -> uint8_t {
    std::uniform_int_distribution<int> dist(0, 255);
    const auto v = dist(*static_cast<std::mt19937*>(rngPtr));
    return static_cast<uint8_t>(v);
  };

  net.randomize(&rng, rngFunc);

  NN::NetRunner runner(&net);

  NN::LSOptimizer optimizer(&net);
  // NN::LSOptimizer optimizer(&net, 2, 0, 4, 1);

  EXPECT_TRUE(optimizer.allocMemory());

  // const auto numEpochs = 10'000'000;
  const auto numEpochs = 1000;

  for (auto i = 0; i < numEpochs; i++) {

    // generate input data
    std::uniform_int_distribution<int> opDist(0, 15);
    const auto l = opDist(rng);
    const auto r = opDist(rng);
    const auto result = l ^ r;

    auto* input = runner.getRegister(0);

    // op 1
    input[0] = (l & 1) ? 255 : 0;
    input[1] = (l & 2) ? 255 : 0;
    input[2] = (l & 4) ? 255 : 0;
    input[3] = (l & 8) ? 255 : 0;

    // op 2
    input[4] = (r & 1) ? 255 : 0;
    input[5] = (r & 2) ? 255 : 0;
    input[6] = (r & 4) ? 255 : 0;
    input[7] = (r & 8) ? 255 : 0;

    const uint8_t target[4]{ static_cast<uint8_t>((result & 1) ? 255 : 0),
                             static_cast<uint8_t>((result & 2) ? 255 : 0),
                             static_cast<uint8_t>((result & 4) ? 255 : 0),
                             static_cast<uint8_t>((result & 8) ? 255 : 0) };

    runner.reset();

    XorProblem problem{ &runner, &target[0], src, sizeof(src) - 1 };

    auto rngFunc = [](void* rngPtr, int32_t minVal, int32_t maxVal) -> int32_t {
      std::uniform_int_distribution<int32_t> dist(minVal, maxVal - 1);
      return dist(*static_cast<std::mt19937*>(rngPtr));
    };

    auto lossFunc = [](void* lossData, const NN::Net& net) -> uint32_t {
      auto* problem = static_cast<XorProblem*>(lossData);
      EXPECT_EQ(NN::exec(problem->source, problem->sourceLen, *problem->runner), NN::SyntaxError::kNone);
      const auto* output = problem->runner->getRegister(2);
      const auto loss = NN::l1Loss(output, problem->expected, 4);
      return loss;
    };

    const auto loss = optimizer.step(&rng, rngFunc, &problem, lossFunc);

    std::cout << static_cast<int>(loss) << std::endl;
  }

  optimizer.releaseMemory();

  net.releaseMemory();
}
