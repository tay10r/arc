#pragma once

#include <vz.h>

#include <GLES3/gl3.h>

#include <string>

#include <map>

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

  void syncUniforms() override;

  auto bindUniformMat4(const char* name, const float* data) -> bool override;

  auto bindUniformVec3(const char* name, const float* data) -> bool override;

protected:
  [[nodiscard]] auto bind(std::map<GLint, const float*>& m, const char* name, const float* data) -> bool;

  void defineImpl(const char* name, const char* value) override;

private:
  std::string macros_;

  GLuint id_{};

  std::string fragLog_;

  std::string vertLog_;

  std::string linkLog_;

  std::map<GLint, const float*> vec3Uniforms_;

  std::map<GLint, const float*> mat4Uniforms_;
};

} // namespace vz
