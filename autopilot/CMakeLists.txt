cmake_minimum_required(VERSION 3.14.7)

project(arc_autopilot
  DESCRIPTION "The autopilot software for the ARC project."
  VERSION 1.0)

option(ARC_AUTOPILOT_TESTS "Whether or not to build the unit tests." OFF)
option(ARC_AUTOPILOT_SIM "Whether or not to build the simulation programs." OFF)
option(ARC_AUTOPILOT_EXAMPLES "Whether or not to build the example programs." OFF)

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
  AP_Hil.h
  AP_Hil.cpp
  AP_Magnetometer.h
  AP_Magnetometer.cpp
  AP_MMC5983MA.h
  AP_MMC5983MA.cpp
  AP_NMEA.h
  AP_NMEA.cpp
  AP_GPS.h
  AP_GPS.cpp
  AP_UbloxGPS.h
  AP_UbloxGPS.cpp
  AP_Random.h
  AP_Random.cpp
  AP_WGS84.h
  AP_WGS84.cpp
  AP_LinAlg.h
  AP_LinAlg.cpp
  NN_Module.h
  NN_Module.cpp
  NN_Lexer.h
  NN_Lexer.cpp
  NN_Parser.h
  NN_Parser.cpp
  NN_Interpreter.h
  NN_Interpreter.cpp
  NN_Net.h
  NN_Net.cpp
  NN_NetBuilder.h
  NN_NetBuilder.cpp
  NN_NetRunner.h
  NN_NetRunner.cpp
  NN_Loss.h
  NN_Loss.cpp
  NN_Optim.h
  NN_Optim.cpp
  NN_RegCounter.h
  NN_RegCounter.cpp
  RL_Policy.h
  RL_Policy.cpp
  RL_DDPG.h
  RL_DDPG.cpp
  SIM_Vehicle.h
  SIM_Vehicle.cpp
  SIM_GPS.h
  SIM_GPS.cpp
  SIM_Clock.h
  SIM_Clock.cpp
  SIM_Boat.h
  SIM_Boat.cpp)

if(CMAKE_COMPILER_IS_GNUCXX AND NOT ARC_NO_WARNINGS)
  target_compile_options(arc_autopilot
    PRIVATE
      -Wall -Wextra -Werror -Wfatal-errors -Wno-address-of-packed-member -std=c++11)
endif()

target_include_directories(arc_autopilot
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(arc_autopilot
  PUBLIC
    arc::fake_arduino)

add_library(arc::autopilot ALIAS arc_autopilot)

if(ARC_AUTOPILOT_TESTS)
  add_subdirectory(tests)
endif()

if(ARC_AUTOPILOT_SIM)
  add_subdirectory(sim)
endif()

if(ARC_AUTOPILOT_EXAMPLES)
  add_subdirectory(examples)
endif()
