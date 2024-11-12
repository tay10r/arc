#include "RL_DDPG.h"

#include "NN_Parser.h"

#include <string.h>

namespace RL {

DDPGPolicy::DDPGPolicy(const NN::Net* net, const char* source, const uint16_t sourceLength)
  : runner_(net)
  , source_(source)
  , sourceLength_(sourceLength)
{
}

void
DDPGPolicy::reset()
{
}

void
DDPGPolicy::computeAction(const State& state, Action& action)
{
  memcpy(runner_.getRegister(0), state.rotation, sizeof(state.rotation));
  runner_.getRegister(1)[0] = state.altitudeError;
  memcpy(runner_.getRegister(2), state.speedError, sizeof(state.speedError));

  runner_.reset();

  const auto err = NN::exec(source_, sourceLength_, runner_);
  if (err != NN::SyntaxError::kNone) {
    return;
  }

  auto* output = runner_.getRegister(3);
  memcpy(action.actuators, output, sizeof(action.actuators));
}

} // namespace RL
