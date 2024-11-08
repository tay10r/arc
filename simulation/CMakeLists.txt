cmake_minimum_required(VERSION 3.14.7)

project(arc_sim)

include(FetchContent)

FetchContent_Declare(rp3 URL "https://github.com/DanielChappuis/reactphysics3d/archive/refs/tags/v0.10.2.zip")
FetchContent_MakeAvailable(rp3)

add_library(arc_sim
  include/sim/Env.h
  include/sim/EnvRegistry.h
  src/Env.cpp
  src/EnvRegistry.cpp
  src/LunarLander.h
  src/LunarLander.cpp)

target_include_directories(arc_sim
  PUBLIC
    include)

target_link_libraries(arc_sim
  PUBLIC
    reactphysics3d)
