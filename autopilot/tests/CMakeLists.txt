cmake_minimum_required(VERSION 3.14.7)

find_package(GTest REQUIRED)

add_executable(arc_autopilot_tests
  lexer.cpp
  parser.cpp
  interpreter.cpp
  reg_counter.cpp
  gps.cpp
  nmea.cpp
  random.cpp)

target_link_libraries(arc_autopilot_tests
  PUBLIC
    arc::autopilot
    GTest::GTest
    GTest::Main)

set_target_properties(arc_autopilot_tests
  PROPERTIES
    OUTPUT_NAME run_tests
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(arc_autopilot_tests PUBLIC -Wno-address-of-packed-member)
endif()
