cmake_minimum_required(VERSION 3.14.7)

project(engine)

find_package(spdlog CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)

FetchContent_Declare(rp3
  URL "https://github.com/DanielChappuis/reactphysics3d/archive/refs/tags/v0.10.2.zip"
  URL_HASH "SHA256=04b582249c57121618d9a821de9371324e796c095307fb3c7b274b2c6319b21d")
FetchContent_MakeAvailable(rp3)

if(NOT TARGET vz)
  add_subdirectory(../vz vz)
endif()

add_library(engine
  include/engine/game.h
  src/game.cpp
  proto/engine/game.proto
  proto/engine/scene.proto
  proto/engine/float3.proto
  proto/engine/transform.proto
  proto/engine/camera.proto
  proto/engine/mesh.proto
  proto/engine/physics/body.proto
  proto/engine/physics/collider.proto
  proto/engine/physics/box.proto
  proto/engine/physics/sphere.proto
  proto/engine/physics/capsule.proto
)

target_include_directories(engine
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_BINARY_DIR}")

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(engine PUBLIC -Wall -Wextra -std=c++17)
endif()

target_link_libraries(engine
  PUBLIC
    vz
    reactphysics3d
    protobuf::libprotobuf)

protobuf_generate(TARGET engine
  IMPORT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/proto")

add_executable(editor
  editor/main.cpp
  editor/Util.h
  editor/Util.cpp
  editor/SceneEditor.h
  editor/SceneEditor.cpp
  editor/PropertiesEditor.h
  editor/PropertiesEditor.cpp
  editor/Viewport.h
  editor/Viewport.cpp)

target_link_libraries(editor
  PUBLIC
    engine
    spdlog::spdlog)
