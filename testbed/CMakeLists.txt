cmake_minimum_required(VERSION 3.14.7)

project(arc_testbed
  DESCRIPTION "A program for testing the ARC software."
  VERSION 1.0)

if(NOT TARGET arc::sim)
  add_subdirectory(../simulation simulation)
endif()

if(NOT TARGET arc::fake_arduino)
  add_subdirectory(../arduino arduino)
endif()

if(NOT TARGET arc::autopilot)
  add_subdirectory(../autopilot autopilot)
endif()

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)

set(UIKIT_MAIN ON CACHE BOOL "" FORCE)
set(UIKIT_BUILD_GLFW ON CACHE BOOL "" FORCE)
FetchContent_Declare(uikit URL "https://github.com/tay10r/uikit/archive/refs/heads/main.zip")
FetchContent_MakeAvailable(uikit)

FetchContent_Declare(glm URL "https://github.com/g-truc/glm/archive/refs/tags/1.0.1.zip")
FetchContent_MakeAvailable(glm)

FetchContent_Declare(libuv URL "https://github.com/libuv/libuv/archive/refs/tags/v1.49.2.zip")
FetchContent_MakeAvailable(libuv)

set(extra_sources)
if(WIN32)
  list(APPEND win32/Serial.cpp)
else()
  list(APPEND NullSerial.cpp)
endif()

add_executable(main WIN32
  main.cpp
  NetTest.h
  NetTest.cpp
  Simulation.h
  Simulation.cpp
  Renderer.h
  Renderer.cpp
  Serial.h
  ${extra_sources})

target_link_libraries(main
  PUBLIC
    uikit::main
    glm::glm
    uv_a
    arc::sim
    arc::fake_arduino
    arc::autopilot)

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(main PUBLIC -Wno-address-of-packed-member)
endif()
