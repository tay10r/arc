cmake_minimum_required(VERSION 3.14.7)

project(arc_fake_arduino)

add_library(arc_fake_arduino
  include/Arduino.h
  include/Print.h
  include/Stream.h
  include/HardwareSerial.h
  include/Wire.h
  src/Print.cpp
  src/Stream.cpp
  src/HardwareSerial.cpp
  src/Wire.cpp)

target_include_directories(arc_fake_arduino
  PUBLIC
    include)

add_library(arc::fake_arduino ALIAS arc_fake_arduino)
