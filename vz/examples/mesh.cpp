#include <vz.h>

#include <glm/glm.hpp>

#include <imgui.h>

#include <fstream>
#include <iostream>
#include <vector>

#include <cstdint>
#include <cstdlib>

namespace {

auto
readFile(std::ifstream& file) -> std::vector<std::uint8_t>
{
  file.seekg(0, std::ios::end);
  const auto fileSize = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<std::uint8_t> buffer;
  buffer.resize(fileSize);

  file.read(reinterpret_cast<char*>(&buffer[0]), buffer.size());

  const auto readSize = file.gcount();
  buffer.resize(readSize);

  return buffer;
}

auto
loadMonkey(float r = 0.8, float g = 0.8, float b = 0.8) -> vz::VertexArray*
{
  std::ifstream file(EXAMPLE_DIR "/monkey.stl", std::ios::in | std::ios::binary);

  if (!file.good()) {
    return nullptr;
  }

  const auto data = readFile(file);

  const auto* ptr = data.data();

  const auto numTris = *reinterpret_cast<const std::uint32_t*>(ptr + 80);

  std::vector<float> buffer;
  // three vertices per triangle, three attributes per vertex, three floats per attribute
  buffer.resize(numTris * 3 * 3 * 3);

  for (auto i = 0u; i < numTris; i++) {
    const auto* normal = reinterpret_cast<const float*>(ptr + 84 + 50 * i);

    const auto* pos = normal + 3;

    auto* dst = &buffer[i * 27u];

    for (auto j = 0; j < 3; j++) {

      dst[0] = pos[0 + j * 3];
      dst[1] = pos[1 + j * 3];
      dst[2] = pos[2 + j * 3];
      dst[3] = normal[0];
      dst[4] = normal[1];
      dst[5] = normal[2];
      dst[6] = r;
      dst[7] = g;
      dst[8] = b;

      dst += 9;
    }
  }

  auto* vertexArray = vz::createVertexArray();

  vertexArray->setVertexSizes({ 3, 3, 3 });

  vertexArray->uploadData(buffer.data(), buffer.size() * sizeof(buffer[0]));
  return vertexArray;
}

} // namespace

auto
main() -> int
{
  if (!vz::setup(/*windowTitle=*/"Mesh Example")) {
    std::cerr << "Failed to setup up VZ." << std::endl;
    return EXIT_FAILURE;
  }

  auto cameraController = vz::CameraController::create();

  cameraController->setPosition(-10, 0, -1);

  auto* monkey = loadMonkey();
  if (!monkey) {
    std::cerr << "Failed to load monkey" << std::endl;
    vz::teardown();
    return EXIT_FAILURE;
  }

  auto* meshShader = vz::getMeshShader();
  if (!meshShader) {
    std::cerr << "Failed to get mesh shader." << std::endl;
    vz::teardown();
    return EXIT_FAILURE;
  }

  meshShader->use();

  (void)meshShader->bindUniformMat4("iViewProjection", cameraController->getCombinedMat4());

  const glm::vec3 lightDir = glm::normalize(glm::vec3(0.0F, 1.0F, -1.0F));
  (void)meshShader->bindUniformVec3("iLightDir", &lightDir[0]);

  const glm::vec3 lightColor(1.0F, 1.0F, 1.0F);
  (void)meshShader->bindUniformVec3("iLightColor", &lightColor[0]);

  const glm::vec3 ambientColor(0.1F, 0.1F, 0.1F);
  (void)meshShader->bindUniformVec3("iAmbientColor", &ambientColor[0]);

  while (vz::beginFrame()) {

    // vz::renderGrid();

    cameraController->step(0.01F);

    meshShader->syncUniforms();

    monkey->draw(GL_TRIANGLES);

    vz::endFrame();
  }

  vz::teardown();

  return EXIT_SUCCESS;
}
