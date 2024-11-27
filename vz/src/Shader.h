#pragma once

#include <vz.h>

#include <GLES3/gl3.h>

namespace vz {

class ShaderImpl final : public Shader
{
public:
  ShaderImpl();

  ShaderImpl(const ShaderImpl&) = delete;

  ShaderImpl(ShaderImpl&&) = delete;

  ~ShaderImpl();

  auto operator=(const ShaderImpl&) -> ShaderImpl& = delete;

  auto operator=(ShaderImpl&&) -> ShaderImpl& = delete;

  [[nodiscard]] auto build(const char* vertSource, const char* fragSource) -> bool override;

  [[nodiscard]] auto getLog(LogKind kind) const -> std::string override;

  void use() override;

protected:
  void defineImpl(const char* name, const char* value) override;

private:
  std::string macros_;

  GLuint id_{};

  std::string fragLog_;

  std::string vertLog_;

  std::string linkLog_;
};

} // namespace vz
