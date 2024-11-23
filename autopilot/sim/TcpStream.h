#pragma once

#include <Stream.h>

#include <memory>

#include <uv.h>

class TcpStream : public Stream
{
public:
  static auto create(uv_loop_t* loop) -> std::unique_ptr<TcpStream>;

  ~TcpStream() override = default;

  virtual void close() = 0;

  [[nodiscard]] virtual auto setup(const char* ip, int port, int backlog = 128) -> bool = 0;
};
