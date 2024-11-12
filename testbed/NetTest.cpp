#include "NetTest.h"

#include <imgui.h>
#include <imgui_stdlib.h>

#include <implot.h>

#include <random>
#include <string>
#include <vector>

#include <NN_Lexer.h>
#include <NN_Loss.h>
#include <NN_NetBuilder.h>
#include <NN_NetRunner.h>
#include <NN_Optim.h>
#include <NN_Parser.h>

namespace {

const char initSource[] = "%2 = Linear 8 8 %0\n"
                          "%3 = Linear 8 4 %2\n"
                          "%1 = Sigmoid %3\n";

class NetTestImpl final : public NetTest
{
public:
  NetTestImpl() { buildNet(); }

  void render() override
  {
    if (optimize_ && optimizer_) {
      stepOptim();
    }

    if (ImGui::BeginTabBar("Tabs")) {
      if (ImGui::BeginTabItem("Optimization")) {
        renderOptim();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Source")) {
        renderSource();
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
  }

protected:
  [[nodiscard]] static auto randomParam(void* selfPtr) -> float
  {
    auto* self = static_cast<NetTestImpl*>(selfPtr);
    std::uniform_real_distribution<float> dist(-1, 1);
    return dist(self->rng_);
  }

  void buildNet()
  {
    net_.releaseMemory();

    NN::NetBuilder builder(&net_, 8);

    (void)NN::exec(netSource_.c_str(), netSource_.size(), builder);

    (void)builder.finish();

    net_.randomize(this, randomParam);
  }

  void renderOptim()
  {
    ImGui::Checkbox("Optimize", &optimize_);

    auto changed{ false };

    changed |= ImGui::SliderInt("Batch Size", &batchSize_, 1, net_.numParameters);

    changed |= ImGui::SliderFloat("Noise", &noise_, 0, 2.0F, "%.5F");

    changed |= ImGui::SliderFloat("Decay", &decay_, 0, 0.1F, "%.5F");

    if (!optimizer_ || changed) {
      buildNet();
      optimizer_.reset(new NN::LSOptimizer(&net_, batchSize_, -noise_, noise_, decay_));
      (void)optimizer_->allocMemory();
      loss_.clear();
      bestLoss_.clear();
    }

    if (optimizer_) {
      renderOptimState();
    }
  }

  static auto randomFloat(void* selfPtr, float minValue, float maxValue) -> float
  {
    std::uniform_real_distribution<float> dist(minValue, maxValue);
    auto* self = static_cast<NetTestImpl*>(selfPtr);
    return dist(self->rng_);
  }

  static auto randomInt(void* selfPtr, int minValue, int maxValue) -> int
  {
    std::uniform_int_distribution<int> dist(minValue, maxValue - 1);
    auto* self = static_cast<NetTestImpl*>(selfPtr);
    return dist(self->rng_);
  }

  static auto computeLoss(void* selfPtr, const NN::Net& net) -> float
  {
    auto* self = static_cast<NetTestImpl*>(selfPtr);

    NN::NetRunner runner(&net);

    constexpr auto maxIterations{ 128 };

    auto loss{ 0.0F };

    for (auto i = 0; i < maxIterations; i++) {

      std::uniform_int_distribution<int> operandDist(0, 15);
      const auto a = operandDist(self->rng_);
      const auto b = operandDist(self->rng_);

      auto* input = runner.getRegister(0);
      input[0] = (a & 1) ? 1.0F : 0.0F;
      input[1] = (a & 2) ? 1.0F : 0.0F;
      input[2] = (a & 4) ? 1.0F : 0.0F;
      input[3] = (a & 8) ? 1.0F : 0.0F;

      input[4] = (b & 1) ? 1.0F : 0.0F;
      input[5] = (b & 2) ? 1.0F : 0.0F;
      input[6] = (b & 4) ? 1.0F : 0.0F;
      input[7] = (b & 8) ? 1.0F : 0.0F;

      runner.reset();

      if (NN::exec(self->netSource_.c_str(), self->netSource_.size(), runner) != NN::SyntaxError::kNone) {
        return 0.0F;
      }

      const auto* output = runner.getRegister(2);
      const auto result = a ^ b;
      const float target[4]{
        (result & 1) ? 1.0F : 0.0F, (result & 2) ? 1.0F : 0.0F, (result & 4) ? 1.0F : 0.0F, (result & 8) ? 1.0F : 0.0F
      };
      loss += NN::l1Loss(output, target, 4);
    }

    return loss / maxIterations;
  }

  void stepOptim()
  {
    const auto loss = optimizer_->step(this, randomInt, randomFloat, this, computeLoss);

    loss_.emplace_back(loss);

    bestLoss_.emplace_back(optimizer_->getBestLoss());
  }

  void renderOptimState()
  {
    if (!ImPlot::BeginPlot("Optimization", ImVec2(-1, -1), ImPlotFlags_NoFrame | ImPlotFlags_Crosshairs)) {
      return;
    }

    ImPlot::SetupAxes("Sample", "Loss", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);

    ImPlot::PlotLine("Loss", loss_.data(), loss_.size());
    ImPlot::PlotLine("Best Loss", bestLoss_.data(), bestLoss_.size());

    ImPlot::EndPlot();
  }

  void renderSource()
  {
    NN::Lexer lexer(netSource_.c_str(), netSource_.size());

    NN::NetBuilder builder(&net_, 8);

    NN::Parser parser(&builder);

    const auto err = parser.parse(lexer);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
    switch (err) {
      case NN::SyntaxError::kNone:
        break;
      case NN::SyntaxError::kUnexpectedToken:
        ImGui::TextUnformatted("unexpected token");
        break;
      case NN::SyntaxError::kInvalidOperand:
        ImGui::TextUnformatted("invalid operand");
        break;
      case NN::SyntaxError::kUnknownFunction:
        ImGui::TextUnformatted("rnknown function");
        break;
      case NN::SyntaxError::kRegisterOutOfBounds:
        ImGui::TextUnformatted("register out of bounds");
        break;
      case NN::SyntaxError::kNumberOutOfBounds:
        ImGui::TextUnformatted("number out of bounds");
        break;
    }
    ImGui::PopStyleColor();

    ImGui::InputTextMultiline("##Source", &netSource_, ImVec2(-1, -1));
  }

private:
  NN::Net net_;

  bool optimize_{ false };

  int batchSize_{ 8 };

  float noise_{ 2e-1F };

  float decay_{ 1e-3F };

  std::unique_ptr<NN::LSOptimizer> optimizer_;

  std::string netSource_{ initSource };

  std::vector<float> loss_;

  std::vector<float> bestLoss_;

  std::mt19937 rng_{ 0 };
};

} // naemspace

auto
NetTest::create() -> std::unique_ptr<NetTest>
{
  return std::make_unique<NetTestImpl>();
}
