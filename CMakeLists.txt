add_subdirectory(tools)
add_subdirectory(vendor)

# TODO: make optional
add_subdirectory(dolos)

set(NYX_SOURCES
  src/nyx/base_object.cc
  src/nyx/builtins.cc
  src/nyx/console_binding.cc
  src/nyx/env.cc
  src/nyx/errors.cc
  src/nyx/extension.cc
  src/nyx/game_lock.cc
  src/nyx/widget.cc
  src/nyx/widget_binding.cc
  src/nyx/imgui_draw_context.cc
  src/nyx/imgui_input_event.cc
  src/nyx/imgui_draw_data_store.cc
  src/nyx/isolate_data.cc
  src/nyx/module_wrap.cc
  src/nyx/nyx.cc
  src/nyx/nyx_binding.cc
  src/nyx/nyx_fs.cc
  src/nyx/nyx_imgui.cc
  src/nyx/nyx_memory.cc
  src/nyx/process_binding.cc
  src/nyx/realm.cc
  src/nyx/timers.cc
  src/nyx/util.cc)

add_library(nyx STATIC ${NYX_SOURCES})
add_library(nyx::nyx ALIAS nyx)

target_js_sources(nyx lib ${CMAKE_CURRENT_BINARY_DIR}/compiled_builtins.cc)
target_include_directories(nyx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(nyx PUBLIC uv_a simdutf v8 imgui)
target_compile_definitions(nyx PUBLIC NYX_DEBUG WIN32_LEAN_AND_MEAN)
