include_guard()

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)

FetchContent_Declare(implot URL "${IMPLOT_URL}")
FetchContent_MakeAvailable(implot)
FetchContent_GetProperties(implot)

add_library(implot STATIC
  "${implot_SOURCE_DIR}/implot.cpp"
  "${implot_SOURCE_DIR}/implot_items.cpp")
target_include_directories(implot PUBLIC "${implot_SOURCE_DIR}")
target_link_libraries(implot PUBLIC imgui::imgui)
add_library(implot::implot ALIAS implot)
