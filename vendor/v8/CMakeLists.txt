add_library(v8 INTERFACE)
target_include_directories(v8 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(v8 INTERFACE /std:c++latest)
target_link_libraries(v8 INTERFACE 
  dbghelp 
  winmm
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/v8_monolith_$<IF:$<CONFIG:Debug>,x64-debug,x64-release>.lib)
target_compile_definitions(v8 INTERFACE NOMINMAX)
target_compile_definitions(v8 INTERFACE $<$<CONFIG:Debug>:V8_ENABLE_CHECKS=1>)
  
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  target_compile_definitions(v8 INTERFACE V8_COMPRESS_POINTERS)
endif()
