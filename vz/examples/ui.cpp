#include <vz.h>

#include <imgui.h>

#include <iostream>

#include <cstdlib>

auto
main() -> int
{
  if (!vz::setup(/*windowTitle=*/"")) {
    std::cerr << "Failed to setup up VZ." << std::endl;
    return EXIT_FAILURE;
  }

  auto counter{ 0 };

  while (vz::beginFrame()) {
    if (ImGui::Begin("Example Window")) {
      if (ImGui::Button("Click me")) {
        counter++;
      }
      ImGui::Text("Button clicked: %d times", counter);
    }
    ImGui::End();
    vz::endFrame();
  }

  vz::teardown();

  return EXIT_SUCCESS;
}
