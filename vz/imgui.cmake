include_guard()

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)

FetchContent_Declare(imgui URL "${IMGUI_URL}")
FetchContent_MakeAvailable(imgui)
FetchContent_GetProperties(imgui)

add_library(imgui STATIC
  "${imgui_SOURCE_DIR}/imgui.cpp"
  "${imgui_SOURCE_DIR}/imgui_tables.cpp"
  "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
  "${imgui_SOURCE_DIR}/imgui_draw.cpp"
  "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h"
  "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp")
target_include_directories(imgui PUBLIC "${imgui_SOURCE_DIR}" "${imgui_SOURCE_DIR}/misc/cpp")
target_compile_definitions(imgui PUBLIC ImDrawIdx=unsigned)
add_library(imgui::imgui ALIAS imgui)

if(EMSCRIPTEN)
  add_library(imgui_sdl2 STATIC
    "${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.h"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp")
  target_include_directories(imgui_sdl2 PUBLIC "${imgui_SOURCE_DIR}/backends")
  target_link_libraries(imgui_sdl2 PUBLIC imgui::imgui)
  target_compile_options(imgui_sdl2 PUBLIC "SHELL: -s USE_SDL=2")
  add_library(imgui::sdl2 ALIAS imgui_sdl2)
else()
  add_library(imgui_glfw STATIC
    "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp")
  target_include_directories(imgui_glfw PUBLIC "${imgui_SOURCE_DIR}/backends")
  target_link_libraries(imgui_glfw PUBLIC imgui::imgui glfw)
  target_compile_definitions(imgui_glfw PUBLIC GLFW_INCLUDE_NONE=1)
  add_library(imgui::glfw ALIAS imgui_glfw)
endif()

add_library(imgui_opengl3 STATIC
  "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.h"
  "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp")
target_include_directories(imgui_opengl3 PUBLIC "${imgui_SOURCE_DIR}/backends")
target_link_libraries(imgui_opengl3 PUBLIC imgui::imgui)
if(NOT EMSCRIPTEN)
  target_link_libraries(imgui_opengl3 PUBLIC glad::glad)
endif()
target_compile_definitions(imgui_opengl3 PUBLIC IMGUI_IMPL_OPENGL_ES3=1)
add_library(imgui::opengl3 ALIAS imgui_opengl3)
