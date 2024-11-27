#include <vz.h>

#include <iostream>

#include <cstdlib>

namespace {

const char vertSrc[] = R"(
layout(location = 0) in vec2 position;

out lowp vec2 uv;

void
main()
{
    uv = position;
    gl_Position = vec4(position * 2.0 - 1.0, 0.0, 1.0);
}
)";

const char fragSrc[] = R"(
in lowp vec2 uv;

out lowp vec4 outColor;

void
main()
{
    outColor = vec4(uv, 1.0, 1.0);
}
)";

} // namespace

auto
main() -> int
{
  if (!vz::setup(/*windowTitle=*/"")) {
    std::cerr << "Failed to setup up VZ." << std::endl;
    return EXIT_FAILURE;
  }

  vz::setCameraPosition(-5, 0, -1);

  const float vertexData[] = { 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1 };
  auto vertices = vz::createVertexArray();
  vertices->uploadData(vertexData, sizeof(vertexData));
  vertices->setVertexSizes({ 2 });

  auto* shader = vz::createShader();
  if (!shader->build(vertSrc, fragSrc)) {
    shader->dumpLogs(std::cerr);
    vz::teardown();
    return EXIT_FAILURE;
  }

  shader->use();

  while (vz::beginFrame()) {
    vertices->draw(GL_TRIANGLES);
    vz::endFrame();
  }

  vz::teardown();

  return EXIT_SUCCESS;
}
