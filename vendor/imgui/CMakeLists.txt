set(IMGUI_SRCS
  misc/cpp/imgui_stdlib.cpp
  imgui.cpp
  imgui_demo.cpp
  imgui_draw.cpp
  imgui_tables.cpp
  imgui_widgets.cpp)

add_library(imgui ${IMGUI_SRCS})
add_library(imgui::imgui ALIAS imgui)

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(imgui PRIVATE lazy_importer)
target_compile_definitions(imgui PUBLIC -DIMGUI_DISABLE_TTY_FUNCTIONS)
