cmake_minimum_required(VERSION 3.14.7)

project(vz)

include(CMakeRC.cmake)

option(VZ_EXAMPLES "Whether or not to build the example programs." OFF)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_subdirectory(glad)

set(IMGUI_URL "https://github.com/ocornut/imgui/archive/refs/tags/v1.91.5-docking.zip")
set(IMPLOT_URL "https://github.com/epezent/implot/archive/refs/tags/v0.16.zip")

include(imgui.cmake)
include(implot.cmake)
include(CMakeRC.cmake)

cmrc_add_resource_library(vz_font
  font/DroidSans.ttf)

cmrc_add_resource_library(vz_shaders
  shaders/grid.vert
  shaders/grid.frag
  shaders/mesh.vert
  shaders/mesh.frag)

add_library(vz
  "include/vz.h"
  "src/vz.cpp"
  "src/Camera.h"
  "src/Camera.cpp"
  "src/CameraController.h"
  "src/CameraController.cpp"
  "src/Shader.h"
  "src/Shader.cpp"
  "src/VertexArray.h"
  "src/VertexArray.cpp"
  "src/Framebuffer.h"
  "src/Framebuffer.cpp")

target_link_libraries(vz PUBLIC glfw imgui::imgui imgui::opengl3 imgui::glfw implot::implot vz_font vz_shaders)

target_include_directories(vz PUBLIC "include")

if(VZ_EXAMPLES)
  add_subdirectory(examples)
endif()
