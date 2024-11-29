#include <vz.h>

#include <iostream>

#include <cstdlib>

#include "stb_image_write.h"

namespace {

const char vertSrc[] = R"(
layout(location = 0) in vec2 position;

out lowp vec2 uv;

void
main()
{
    uv = (position + 1.0) * 0.5;
    gl_Position = vec4(position, 0.0, 1.0);
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

#if 1
  const float vertexData[] = { -1, 1, -1, -1, 1, -1, 1, -1, 1, 1, -1, 1 };
  auto vertices = vz::createVertexArray();
  vertices->uploadData(vertexData, sizeof(vertexData));
  vertices->setVertexSizes({ 2 });
#else /* Alternatively for screen quads you can do this: */
  auto* vertices = vz::getScreenQuad();
#endif

  auto* shader = vz::createShader();
  if (!shader->build(vertSrc, fragSrc)) {
    shader->dumpLogs(std::cerr);
    vz::teardown();
    return EXIT_FAILURE;
  }

  shader->use();

  const auto w{ 640 };
  const auto h{ 480 };
  auto* framebuffer = vz::createFramebuffer(w, h, 1, /*includeDepthBuffer=*/false);
  framebuffer->use();
  vertices->draw(GL_TRIANGLES);
  const auto pixels = framebuffer->readPixels();
  stbi_write_png("result.png", w, h, 4, pixels.data(), w * 4);
  vz::teardown();

  return EXIT_SUCCESS;
}
