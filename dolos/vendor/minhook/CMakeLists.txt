set(MINHOOK_SRCS
  src/hde/hde32.c
  src/hde/hde64.c
  src/buffer.c
  src/hook.c
  src/trampoline.c)

add_library(minhook ${MINHOOK_SRCS})
add_library(minhook::minhook ALIAS minhook)

target_include_directories(minhook
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
