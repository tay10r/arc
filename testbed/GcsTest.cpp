#include "GcsTest.h"

#include <AP_Program.h>

#include <atomic>
#include <thread>
#include <vector>

#include <mavlink/common/mavlink.h>

#include <uv.h>

namespace {

[[nodiscard]] auto
toHandle(uv_udp_send_t* h) -> uv_handle_t*
{
  return reinterpret_cast<uv_handle_t*>(h);
}

[[nodiscard]] auto
toHandle(uv_udp_t* h) -> uv_handle_t*
{
  return reinterpret_cast<uv_handle_t*>(h);
}

[[nodiscard]] auto
toHandle(uv_timer_t* h) -> uv_handle_t*
{
  return reinterpret_cast<uv_handle_t*>(h);
}

class WriteOp final
{
public:
  WriteOp()
  {
    uv_handle_set_data(toHandle(&handle_), this);

    buffer_.base = data_;
  }

  [[nodiscard]] auto write(uv_udp_t* socket, const mavlink_message_t& msg, const sockaddr* destination) -> bool
  {
    buffer_.len = mavlink_msg_to_send_buffer(reinterpret_cast<uint8_t*>(&data_[0]), &msg);

    return uv_udp_send(&handle_, socket, &buffer_, 1, destination, onSend) == 0;
  }

protected:
  static void onSend(uv_udp_send_t* handle, const int /*status*/)
  {
    delete static_cast<WriteOp*>(uv_handle_get_data(toHandle(handle)));
  }

private:
  char data_[MAVLINK_MAX_PACKET_LEN]{};

  uv_buf_t buffer_{};

  uv_udp_send_t handle_{};
};

class StreamImpl final : public Stream
{
public:
  explicit StreamImpl(uv_loop_t* loop)
  {
    uv_udp_init(loop, &socket_);

    uv_handle_set_data(toHandle(&socket_), this);
  }

  void close() { uv_close(toHandle(&socket_), nullptr); }

  [[nodiscard]] auto setup(const char* bindIp, const char* sendIp, const int sendPort) -> bool
  {
    if (const auto err = uv_ip4_addr(sendIp, sendPort, &sendAddress_); err) {
      return false;
    }

    sockaddr_in bindAddress{};

    if (const auto err = uv_ip4_addr(bindIp, 0, &bindAddress); err) {
      return false;
    }

    if (const auto err = uv_udp_bind(&socket_, reinterpret_cast<const sockaddr*>(&bindAddress), 0); err) {
      return false;
    }

    if (const auto err = uv_udp_recv_start(&socket_, onAlloc, onRecv); err) {
      return false;
    }

    return true;
  }

  auto available() -> int override { return recvBuffer_.size(); }

  auto read() -> int override
  {
    if (recvBuffer_.empty()) {
      return -1;
    }
    auto v = recvBuffer_.at(0);
    recvBuffer_.erase(recvBuffer_.begin());
    return v;
  }

  auto write(const uint8_t c) -> size_t override
  {
    if (mavlink_parse_char(MAVLINK_COMM_0, c, &recvMessage_, &status_) == 1) {
      publish(recvMessage_);
    }
    return 1;
  }

  auto availableForWrite() -> int override
  {
    // 280 is the maximum length of a mavlink message
    return 280;
  }

protected:
  void publish(const mavlink_message_t& msg)
  {
    auto op = std::make_unique<WriteOp>();

    if (op->write(&socket_, msg, reinterpret_cast<const sockaddr*>(&sendAddress_))) {
      // gets deleted in the write callback
      op.release();
    }
  }

  static auto getSelf(uv_handle_t* h) -> StreamImpl* { return static_cast<StreamImpl*>(uv_handle_get_data(h)); }

  static void onAlloc(uv_handle_t* h, const size_t size, uv_buf_t* buf)
  {
    auto* self = getSelf(h);
    self->recvBuffer_.resize(self->recvSize_ + size);
    buf->base = reinterpret_cast<char*>(self->recvBuffer_.data() + self->recvSize_);
    buf->len = size;
  }

  static void onRecv(uv_udp_t* socket, const ssize_t readSize, const uv_buf_t*, const sockaddr* sender, unsigned)
  {
    if ((readSize < 0) || !sender) {
      return;
    }

    auto* self = getSelf(toHandle(socket));

    self->recvSize_ += static_cast<size_t>(readSize);
  }

private:
  uv_udp_t socket_{};

  sockaddr_in sendAddress_{};

  std::vector<uint8_t> recvBuffer_;

  size_t recvSize_{};

  mavlink_parse_state_t parser_{};

  mavlink_status_t status_{};

  mavlink_message_t recvMessage_{};
};

constexpr auto stepInterval{ 10 };

using TimePoint = std::chrono::high_resolution_clock::time_point;

class GcsTestThread final : public AP::Clock
{
public:
  void setShouldStop(const bool should_stop) { shouldStop_.store(should_stop); }

  void run()
  {
    uv_loop_init(&loop_);

    uv_timer_init(&loop_, &timer_);
    uv_handle_set_data(toHandle(&timer_), this);
    uv_timer_start(&timer_, onInterval, 0, stepInterval);

    StreamImpl stream_(&loop_);

    if (!stream_.setup("127.0.0.1", "127.0.0.1", 3678)) {
      // TODO : log
    }

    program_.setup(&stream_, this);

    uv_run(&loop_, UV_RUN_DEFAULT);

    // shutdown
    uv_close(toHandle(&timer_), nullptr);
    stream_.close();
    uv_run(&loop_, UV_RUN_DEFAULT);

    uv_loop_close(&loop_);
  }

  auto now() -> uint32_t override
  {
    const auto t = std::chrono::high_resolution_clock::now();

    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(t - bootTime_).count();
    return static_cast<uint32_t>(elapsed);
  }

protected:
  static auto getSelf(uv_handle_t* h) -> GcsTestThread* { return static_cast<GcsTestThread*>(uv_handle_get_data(h)); }

  static void onInterval(uv_timer_t* timer)
  {
    auto* self = getSelf(toHandle(timer));

    if (self->shouldStop_.load()) {
      uv_stop(uv_handle_get_loop(toHandle(timer)));
      return;
    }

    self->program_.loop();
  }

private:
  uv_loop_t loop_{};

  uv_timer_t timer_{};

  AP::Program program_;

  std::atomic<bool> shouldStop_{ false };

  TimePoint bootTime_{ std::chrono::high_resolution_clock::now() };
};

class GcsTestImpl final : public GcsTest
{
public:
  GcsTestImpl()
    : thread_(&GcsTestImpl::runThread, this)
  {
  }

  void close() override
  {
    test_->setShouldStop(true);
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  void render() override
  {
    //
  }

protected:
  void runThread()
  {
    test_.reset(new GcsTestThread());

    test_->run();
  }

private:
  std::thread thread_;

  std::unique_ptr<GcsTestThread> test_;
};

} // namespace

auto
GcsTest::create() -> std::unique_ptr<GcsTest>
{
  return std::make_unique<GcsTestImpl>();
}
