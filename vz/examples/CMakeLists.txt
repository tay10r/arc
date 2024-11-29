cmake_minimum_required(VERSION 3.14.7)

add_library(stb_image_write
  stb_image_write.h
  stb_image_write.c)

function(add_example name)
  set(target vz_example_${name})
  add_executable(${target} ${ARGN})
  target_link_libraries(${target} PUBLIC vz stb_image_write)
  set_target_properties(${target} PROPERTIES OUTPUT_NAME ${name})
  target_compile_definitions(${target} PRIVATE "EXAMPLE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")
endfunction()

add_example(custom_shader custom_shader.cpp)
add_example(ui ui.cpp)
add_example(mesh mesh.cpp)
