#include "TcpStream.h"

#include <spdlog/spdlog.h>

#include <mavlink/common/mavlink.h>

#include <vector>

namespace {

auto
toHandle(uv_tcp_t* handle) -> uv_handle_t*
{
  return reinterpret_cast<uv_handle_t*>(handle);
}

auto
toHandle(uv_stream_t* handle) -> uv_handle_t*
{
  return reinterpret_cast<uv_handle_t*>(handle);
}

auto
toHandle(uv_write_t* handle) -> uv_handle_t*
{
  return reinterpret_cast<uv_handle_t*>(handle);
}

auto
toStream(uv_tcp_t* handle) -> uv_stream_t*
{
  return reinterpret_cast<uv_stream_t*>(handle);
}

struct Message final
{
  uint8_t data[MAVLINK_MAX_PACKET_LEN];

  uv_buf_t buffer{};

  static auto create(const mavlink_message_t& msg) -> std::shared_ptr<Message>
  {
    auto msg_ = std::make_shared<Message>();
    msg_->buffer.len = mavlink_msg_to_send_buffer(msg_->data, &msg);
    msg_->buffer.base = reinterpret_cast<char*>(msg_->data);
    return msg_;
  }
};

class GlobalRecvBuffer final
{
public:
  void push(const mavlink_message_t& msg)
  {
    uint8_t tmp[MAVLINK_MAX_PACKET_LEN];

    const auto size = mavlink_msg_to_send_buffer(tmp, &msg);

    const auto offset = buffer_.size();

    buffer_.resize(buffer_.size() + size);

    for (uint32_t i = 0; i < size; i++) {
      buffer_[i] = tmp[i];
    }
  }

  auto available() const -> int { return static_cast<int>(buffer_.size()); }

  auto pop() -> int
  {
    if (!buffer_.empty()) {
      auto c = buffer_[0];
      buffer_.erase(buffer_.begin());
      return static_cast<int>(c);
    }
    return -1;
  }

private:
  std::vector<uint8_t> buffer_;
};

class WriteOp final
{
public:
  WriteOp(std::shared_ptr<Message> msg)
    : message_(std::move(msg))
  {
    uv_handle_set_data(toHandle(&handle_), this);
  }

  [[nodiscard]] auto send(uv_tcp_t* socket) -> bool
  {
    auto err = uv_write(&handle_, toStream(socket), &message_->buffer, 1, onWrite);
    if (err != 0) {
      SPDLOG_ERROR("Failed to start write operation: {}", uv_strerror(err));
      return false;
    }
    return true;
  }

protected:
  static void onWrite(uv_write_t* handle, const int status)
  {
    if (status != 0) {
      SPDLOG_ERROR("Failed to finish message send operation.");
    }

    delete static_cast<WriteOp*>(uv_handle_get_data(toHandle(handle)));
  }

private:
  uv_write_t handle_{};

  std::shared_ptr<Message> message_;
};

class TcpClient final
{
public:
  using CloseCallback = void (*)(void* userData, TcpClient* client);

  explicit TcpClient(uv_loop_t* loop, std::shared_ptr<GlobalRecvBuffer> globalRecvBuffer)
    : globalRecvBuffer_(std::move(globalRecvBuffer))

  {
    uv_tcp_init(loop, &handle_);

    uv_handle_set_data(toHandle(&handle_), this);
  }

  void close()
  {
    if (!uv_is_closing(toHandle(&handle_))) {
      uv_close(toHandle(&handle_), onClose);
    }
  }

  [[nodiscard]] auto setup(uv_tcp_t* server) -> bool
  {
    auto err = uv_accept(toStream(server), toStream(&handle_));
    if (err) {
      SPDLOG_ERROR("Failed to accept client: {}", uv_strerror(err));
      return false;
    }

    err = uv_read_start(toStream(&handle_), onAlloc, onRead);
    if (err) {
      SPDLOG_ERROR("Failed to start reading from client: {}", uv_strerror(err));
      return false;
    }

    sockaddr_storage address{};

    auto nameLen{ static_cast<int>(sizeof(address)) };

    err = uv_tcp_getpeername(&handle_, reinterpret_cast<sockaddr*>(&address), &nameLen);
    if (err != 0) {
      SPDLOG_ERROR("Failed to get peer name: {}", uv_strerror(err));
      return false;
    }

    if (address.ss_family != AF_INET) {
      SPDLOG_ERROR("Cannot get peer name of non-IPv4 client.");
      return false;
    }

    char buffer[128]{};

    uv_ip4_name(reinterpret_cast<sockaddr_in*>(&address), buffer, sizeof(buffer));

    peerAddress_ = std::string(buffer);

    peerPort_ = ntohs(reinterpret_cast<const sockaddr_in*>(&address)->sin_port);

    SPDLOG_INFO("Connected to new client at '{}:{}", peerAddress_, peerPort_);

    return true;
  }

  void setUserData(void* userData) { userData_ = userData; }

  void setCloseCallback(const CloseCallback closeCallback) { closeCallback_ = closeCallback; }

  void send(std::shared_ptr<Message> msg)
  {
    auto writeOp = std::make_unique<WriteOp>(std::move(msg));

    if (writeOp->send(&handle_)) {
      // Gets deleted upon completion.
      (void)writeOp.release();
    }
  }

protected:
  static auto getSelf(uv_handle_t* h) -> TcpClient* { return static_cast<TcpClient*>(uv_handle_get_data(h)); }

  static void onAlloc(uv_handle_t* h, const size_t size, uv_buf_t* buf)
  {
    auto* self = getSelf(h);
    self->readBuffer_.resize(size);
    buf->base = reinterpret_cast<char*>(self->readBuffer_.data());
    buf->len = size;
  }

  static void onRead(uv_stream_t* stream, const ssize_t readSize, const uv_buf_t*)
  {
    auto* self = getSelf(toHandle(stream));

    if (readSize < 0) {
      self->close();
      return;
    }

    for (ssize_t i = 0; i < readSize; i++) {
      if (mavlink_parse_char(MAVLINK_COMM_0, self->readBuffer_[i], &self->recvMessage_, &self->status_) == 1) {
        self->globalRecvBuffer_->push(self->recvMessage_);
      }
    }
  }

  static void onClose(uv_handle_t* handle)
  {
    auto* self = getSelf(handle);

    if (self->closeCallback_) {
      self->closeCallback_(self->userData_, self);
    }
  }

private:
  uv_tcp_t handle_{};

  void* userData_{};

  CloseCallback closeCallback_{};

  std::vector<uint8_t> readBuffer_;

  std::string peerAddress_;

  int peerPort_{};

  std::shared_ptr<GlobalRecvBuffer> globalRecvBuffer_;

  mavlink_parse_state_t parser_{};

  mavlink_status_t status_{};

  mavlink_message_t recvMessage_{};
};

class TcpStreamImpl final : public TcpStream
{
public:
  TcpStreamImpl(uv_loop_t* loop)
  {
    uv_tcp_init(loop, &handle_);

    uv_handle_set_data(toHandle(&handle_), this);
  }

  void close() override
  {
    uv_close(toHandle(&handle_), nullptr);

    for (auto& c : clients_) {
      c->close();
    }
  }

  [[nodiscard]] auto setup(const char* ip, const int port, const int backlog) -> bool override
  {
    sockaddr_in address{};

    auto err = uv_ip4_addr(ip, port, &address);
    if (err) {
      SPDLOG_ERROR("Failed to parse '{}:{}': {}", ip, port, uv_strerror(err));
      return false;
    }

    err = uv_tcp_bind(&handle_, reinterpret_cast<const sockaddr*>(&address), 0);
    if (err) {
      SPDLOG_ERROR("Failed to bind to '{}:{}': {}", ip, port, uv_strerror(err));
      return false;
    }

    err = uv_listen(toStream(&handle_), backlog, onConnect);
    if (err) {
      SPDLOG_ERROR("Failed to start listening on '{}:{}': {}", ip, port, uv_strerror(err));
      return false;
    }

    SPDLOG_INFO("Listening for connections on '{}:{}'.", ip, port);

    return true;
  }

  [[nodiscard]] auto availableForWrite() -> int override { return MAVLINK_MAX_PACKET_LEN; }

  [[nodiscard]] auto write(uint8_t value) -> size_t override
  {
    if (mavlink_parse_char(MAVLINK_COMM_0, value, &message_, &status_) == 1) {
      publishToClients(message_);
    }

    return 0;
  }

  [[nodiscard]] auto read() -> int override { return globalRecvBuffer_->pop(); }

  [[nodiscard]] auto available() -> int override { return globalRecvBuffer_->available(); }

protected:
  static auto getSelf(uv_handle_t* handle) -> TcpStreamImpl*
  {
    return static_cast<TcpStreamImpl*>(uv_handle_get_data(handle));
  }

  static void onClientClose(void* selfPtr, TcpClient* c)
  {
    auto* self = static_cast<TcpStreamImpl*>(selfPtr);

    for (auto it = self->clients_.begin(); it != self->clients_.end(); ++it) {
      if (it->get() == c) {
        self->clients_.erase(it);
        break;
      }
    }
  }

  static void onConnect(uv_stream_t* server, const int status)
  {
    auto* self = getSelf(toHandle(server));

    if (status != 0) {
      SPDLOG_ERROR("Failed to connect to new client: {}", uv_strerror(status));
      return;
    }

    auto client = std::make_unique<TcpClient>(uv_handle_get_loop(toHandle(&self->handle_)), self->globalRecvBuffer_);

    client->setUserData(self);

    client->setCloseCallback(onClientClose);

    if (!client->setup(&self->handle_)) {
      client->close();
    }

    self->clients_.emplace_back(std::move(client));
  }

  void publishToClients(const mavlink_message_t& msg)
  {
    auto msg_ = Message::create(msg);

    for (auto& c : clients_) {
      c->send(msg_);
    }
  }

private:
  uv_tcp_t handle_{};

  mavlink_parse_state_t parser_{};

  mavlink_status_t status_{};

  mavlink_message_t message_{};

  std::vector<std::unique_ptr<TcpClient>> clients_;

  std::shared_ptr<GlobalRecvBuffer> globalRecvBuffer_{ new GlobalRecvBuffer() };
};

} // namespace

auto
TcpStream::create(uv_loop_t* loop) -> std::unique_ptr<TcpStream>
{
  return std::make_unique<TcpStreamImpl>(loop);
}
