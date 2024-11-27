cmake_minimum_required(VERSION 3.14.7)

project(vz)

option(VZ_EXAMPLES "Whether or not to build the example programs." OFF)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_subdirectory(glad)

set(IMGUI_URL "https://github.com/ocornut/imgui/archive/refs/tags/v1.91.5-docking.zip")
set(IMPLOT_URL "https://github.com/epezent/implot/archive/refs/tags/v0.16.zip")

include(imgui.cmake)
include(implot.cmake)

add_library(vz
  "include/vz.h"
  "src/vz.cpp"
  "src/Camera.h"
  "src/Camera.cpp"
  "src/Shader.h"
  "src/Shader.cpp"
  "src/VertexArray.h"
  "src/VertexArray.cpp")

target_link_libraries(vz PUBLIC glfw imgui::imgui imgui::opengl3 imgui::glfw implot::implot)

target_include_directories(vz PUBLIC "include")

if(VZ_EXAMPLES)
  add_subdirectory(examples)
endif()
