cmake_minimum_required(VERSION 3.14.7)

project(fake_arduino)

add_library(fake_arduino
  include/Arduino.h
  include/Print.h
  include/Stream.h
  include/HardwareSerial.h
  src/Print.cpp
  src/Stream.cpp
  src/HardwareSerial.cpp)

target_include_directories(fake_arduino
  PUBLIC
    include)
