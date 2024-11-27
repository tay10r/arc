cmake_minimum_required(VERSION 3.14.7)

add_library(glad
  include/glad/glad.h
  include/KHR/khrplatform.h
  src/glad.c)

target_compile_definitions(glad PUBLIC GLFW_INCLUDE_NONE=1)

target_include_directories(glad PUBLIC include)

add_library(glad::glad ALIAS glad)
