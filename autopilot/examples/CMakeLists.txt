cmake_minimum_required(VERSION 3.14.7)

function(add_example name)
  set(target arc_autopilot_example_${name})
  add_executable(${target} ${ARGN})
  target_link_libraries(${target} PUBLIC arc::autopilot)
  set_target_properties(${target} PROPERTIES OUTPUT_NAME ${name})
endfunction()

add_example(gps gps.cpp)
