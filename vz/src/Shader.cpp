#include "Shader.h"

#include <cstring>

namespace vz {

namespace {

void
dumpLog(const char* label, const std::string& log, std::ostream& stream)
{
  if (log.empty()) {
    return;
  }
  stream << label << std::endl;
  stream << "  ";
  for (size_t i = 0; i < log.size(); i++) {
    const auto c = log[i];
    stream << c;
    if ((c == '\n') && ((i + 1) < log.size())) {
      stream << "  ";
    }
  }
}

} // namespace

void
Shader::dumpLogs(std::ostream& stream) const
{
  dumpLog("Vertex:", getLog(LogKind::VertCompile), stream);
  dumpLog("Fragment:", getLog(LogKind::FragCompile), stream);
  dumpLog("Link:", getLog(LogKind::Link), stream);
}

namespace {

class ShaderComponent final
{
public:
  ShaderComponent(const GLenum type)
    : id_(glCreateShader(type))
  {
  }

  ShaderComponent(const ShaderComponent&) = delete;

  ~ShaderComponent() { glDeleteShader(id_); }

  [[nodiscard]] auto id() const -> GLuint { return id_; }

  [[nodiscard]] auto compile(const char* src) -> bool
  {
    const GLint len = static_cast<GLint>(std::strlen(src));

    glShaderSource(id_, 1, &src, &len);

    glCompileShader(id_);

    GLint status{};

    glGetShaderiv(id_, GL_COMPILE_STATUS, &status);

    return status == GL_TRUE;
  }

  [[nodiscard]] auto readLog() -> std::string
  {
    GLint size{};

    glGetShaderiv(id_, GL_INFO_LOG_LENGTH, &size);

    if (size < 0) {
      return "";
    }

    std::string log;

    log.resize(static_cast<size_t>(size));

    GLint readSize{};

    glGetShaderInfoLog(id_, size, &readSize, &log[0]);

    if (readSize < 0) {
      return "";
    }

    log.resize(static_cast<size_t>(readSize));

    return log;
  }

private:
  GLuint id_{};
};

} // namespace

ShaderImpl::ShaderImpl()
  : id_(glCreateProgram())
{
}

ShaderImpl::~ShaderImpl()
{
  glDeleteProgram(id_);
}

void
ShaderImpl::defineImpl(const char* name, const char* value)
{
  macros_ += name;
  macros_ += ' ';
  macros_ += value;
  macros_ += '\n';
}

auto
ShaderImpl::build(const char* vertSource, const char* fragSource) -> bool
{
  const std::string prefix = "#version 300 es\n" + macros_ + "#line 1\n";

  ShaderComponent vertComp(GL_VERTEX_SHADER);
  const auto vertSuccess = vertComp.compile((prefix + vertSource).c_str());
  vertLog_ = vertComp.readLog();

  ShaderComponent fragComp(GL_FRAGMENT_SHADER);
  const auto fragSuccess = fragComp.compile((prefix + fragSource).c_str());
  fragLog_ = fragComp.readLog();

  if (!vertSuccess || !fragSuccess) {
    return false;
  }

  glAttachShader(id_, vertComp.id());
  glAttachShader(id_, fragComp.id());

  glLinkProgram(id_);

  glDetachShader(id_, fragComp.id());
  glDetachShader(id_, vertComp.id());

  GLint linkStatus{};
  glGetProgramiv(id_, GL_LINK_STATUS, &linkStatus);
  if (linkStatus == GL_TRUE) {
    return true;
  }

  GLint logLength{};
  glGetProgramiv(id_, GL_INFO_LOG_LENGTH, &logLength);
  if (logLength < 0) {
    linkLog_ = "(error getting log length)";
    return false;
  }

  linkLog_.resize(static_cast<size_t>(logLength));

  GLsizei readSize{};

  glGetProgramInfoLog(id_, logLength, &readSize, &linkLog_[0]);
  if (readSize < 0) {
    linkLog_ = "(error reading log)";
    return false;
  }

  linkLog_.resize(readSize);

  return false;
}

auto
ShaderImpl::getLog(LogKind kind) const -> std::string
{
  switch (kind) {
    case LogKind::VertCompile:
      return vertLog_;
    case LogKind::FragCompile:
      return fragLog_;
    case LogKind::Link:
      return linkLog_;
  }

  return "";
}

void
ShaderImpl::use()
{
  glUseProgram(id_);
}

} // namespace vz
