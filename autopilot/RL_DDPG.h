#pragma once

#include "RL_Policy.h"

#include "NN_Net.h"
#include "NN_NetRunner.h"

namespace RL {

enum class DDPGError
{
  kNone
};

/**
 * @brief This function uses a neural network to compute the best continous action for a given state.
 *
 * @note The acronym DDPG stands for deep deterministic policy gradient.
 * */
class DDPGPolicy final : public Policy
{
public:
  /**
   * @brief Constructs a new policy instance.
   *
   * @param net The network to compute the action with.
   *
   * @param source The source code of the network.
   *
   * @param sourceLength The length of the network source code.
   * */
  DDPGPolicy(const NN::Net* net, const char* source, uint16_t sourceLength);

  void reset() override;

  void computeAction(const State& state, Action& action) override;

private:
  NN::NetRunner runner_;

  const char* source_{};

  const uint16_t sourceLength_{};
};

} // namespace RL
