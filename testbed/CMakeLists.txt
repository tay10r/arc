cmake_minimum_required(VERSION 3.14.7)

project(arc_testbed)

include(FetchContent)

set(UIKIT_MAIN ON CACHE BOOL "" FORCE)
set(UIKIT_BUILD_GLFW ON CACHE BOOL "" FORCE)
FetchContent_Declare(uikit URL "https://github.com/tay10r/uikit/archive/refs/heads/main.zip")
FetchContent_MakeAvailable(uikit)

add_executable(main
  main.cpp)

target_link_libraries(main
  PUBLIC
    uikit::main)
