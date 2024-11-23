#include <AP_Program.h>
#include <SIM_GPS.h>

#include <spdlog/spdlog.h>

#include <uv.h>

#include <chrono>
#include <sstream>

#include <stdlib.h>

#include "Options.h"
#include "TcpStream.h"

namespace {

[[nodiscard]] auto
toHandle(uv_signal_t* h) -> uv_handle_t*
{
  return reinterpret_cast<uv_handle_t*>(h);
}

[[nodiscard]] auto
toHandle(uv_timer_t* h) -> uv_handle_t*
{
  return reinterpret_cast<uv_handle_t*>(h);
}

class Clock final : public AP::Clock
{
public:
  using TimePoint = std::chrono::high_resolution_clock::time_point;

  [[nodiscard]] auto now() -> uint32_t override
  {
    const auto dt = std::chrono::high_resolution_clock::now() - startTime_;
    const auto microDeltaT = std::chrono::duration_cast<std::chrono::microseconds>(dt).count();
    return static_cast<uint32_t>(microDeltaT);
  }

private:
  TimePoint startTime_{ std::chrono::high_resolution_clock::now() };
};

constexpr auto stepInterval{ 10 };

class Program final
{
public:
  [[nodiscard]] auto run(const Options& opts) -> bool
  {
    if (!parseHome(opts.home)) {
      return false;
    }

    uv_loop_init(&loop_);

    uv_timer_init(&loop_, &stepTimer_);
    uv_handle_set_data(toHandle(&stepTimer_), this);
    uv_timer_start(&stepTimer_, onStepInterval, 0, stepInterval);

    setupSignalHandler(&loop_, &sigintHandler_, SIGINT);
    setupSignalHandler(&loop_, &sigtermHandler_, SIGTERM);

    Clock clock;

    auto stream = TcpStream::create(&loop_);

    auto ready{ true };

    ready &= stream->setup(opts.simAddress.c_str(), opts.basePort);

    program_.setup(stream.get(), &clock, &gpsSensor_);

    if (ready) {
      uv_run(&loop_, UV_RUN_DEFAULT);
    }

    // shutdown
    SPDLOG_INFO("Shutting down.");
    uv_close(toHandle(&sigintHandler_), nullptr);
    uv_close(toHandle(&sigtermHandler_), nullptr);
    uv_close(toHandle(&stepTimer_), nullptr);
    stream->close();
    uv_run(&loop_, UV_RUN_DEFAULT);

    uv_loop_close(&loop_);

    SPDLOG_INFO("Shutdown complete.");

    return true;
  }

protected:
  static auto getSelf(uv_handle_t* h) -> Program* { return static_cast<Program*>(uv_handle_get_data(h)); }

  static void onStepInterval(uv_timer_t* timer)
  {
    auto* self = getSelf(toHandle(timer));

    self->gpsSensor_.step(stepInterval * 1000ul);

    self->program_.loop();
  }

  static void onSignal(uv_signal_t* handle, const int signum)
  {
    SPDLOG_INFO("Caught signal {}, stopping loop.", signum);

    auto* loop = uv_handle_get_loop(toHandle(handle));

    uv_stop(loop);
  }

  static void setupSignalHandler(uv_loop_t* loop, uv_signal_t* handle, const int signum)
  {
    uv_signal_init(loop, handle);

    uv_signal_start(handle, onSignal, signum);
  }

  [[nodiscard]] auto parseHome(const std::string& home) -> bool
  {
    std::istringstream stream(home);

    std::vector<float> fields;

    while (stream) {
      std::string field;

      std::getline(stream, field, ',');

      if (field.empty()) {
        // eof
        break;
      }

      fields.emplace_back(atof(field.c_str()));
    }

    if (fields.size() != 4) {
      SPDLOG_ERROR("Missing field(s) in home specification '{}' (format: lat,lon,alt=0,heading=0).", home);
      return false;
    }

    gpsSensor_.setOrigin(fields.at(0), fields.at(1));

    return true;
  }

private:
  uv_loop_t loop_{};

  uv_timer_t stepTimer_{};

  uv_signal_t sigintHandler_{};

  uv_signal_t sigtermHandler_{};

  AP::Program program_;

  SIM::GPSSensor gpsSensor_{ /*seed=*/0 };
};

} // namespace

auto
main(int argc, char** argv) -> int
{
  Options options;

  if (!options.parse(argc, argv)) {
    return EXIT_FAILURE;
  }

  SPDLOG_INFO("Starting");

  Program program;

  return program.run(options) ? EXIT_SUCCESS : EXIT_FAILURE;
}
