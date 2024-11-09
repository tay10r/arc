cmake_minimum_required(VERSION 3.14.7)

project(arc_autopilot
  DESCRIPTION "The autopilot software for the ARC project."
  VERSION 1.0)

if(NOT TARGET arc::fake_arduino)
  add_subdirectory(../arduino arduino)
endif()

add_library(arc_autopilot
  AP_Program.h
  AP_Program.cpp
  AP_Mavlink.h
  AP_Mavlink.cpp
  AP_Heartbeat.h
  AP_Heartbeat.cpp
  AP_Time.h
  AP_Time.cpp
  NN_Module.h
  NN_Module.cpp
  NN_Linear.h
  NN_Linear.cpp
  NN_ReLU.h
  NN_ReLU.cpp
  NN_Lexer.h
  NN_Lexer.cpp
  NN_Parser.h
  NN_Parser.cpp
  NN_Interpreter.h
  NN_Interpreter.cpp
  NN_NetBuilder.h
  NN_NetBuilder.cpp
  NN_NetRunner.h
  NN_NetRunner.cpp)

target_include_directories(arc_autopilot
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(arc_autopilot
  PUBLIC
    arc::fake_arduino)

add_library(arc::autopilot ALIAS arc_autopilot)
