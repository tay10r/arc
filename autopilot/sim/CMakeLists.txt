cmake_minimum_required(VERSION 3.14.7)

# Use vcpkg to install these
find_package(libuv CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(cxxopts CONFIG REQUIRED)

add_executable(arc_sim
  main.cpp
  Options.h
  Options.cpp
  TcpStream.h
  TcpStream.cpp)

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(arc_sim PRIVATE -Wno-address-of-packed-member)
endif()

target_link_libraries(arc_sim
  PUBLIC
    spdlog::spdlog
    cxxopts::cxxopts
    libuv::uv_a
    arc::autopilot
    arc::fake_arduino)

set_target_properties(arc_sim
  PROPERTIES
    OUTPUT_NAME arc-sim
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
