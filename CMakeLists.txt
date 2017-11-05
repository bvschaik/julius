cmake_minimum_required(VERSION 2.6)
project(julius)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror=implicit-function-declaration")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
set(SMK_FILES src/Video/smacker.c src/Video/smk_bitstream.c src/Video/smk_hufftree.c)
file(GLOB CORE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.*)
file(GLOB BUILDING_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/building/*.*)
file(GLOB EMPIRE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/empire/*.*)
file(GLOB FIGURE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/figure/*.*)
file(GLOB GAME_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/game/*.*)
file(GLOB GRAPHICS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/*.*)
file(GLOB MAIN_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.*)
file(GLOB UI_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/UI/*.*)
file(GLOB DATA_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/data/*.*)

add_executable(julius
  linux/main2.c
  linux/SDLSoundDevice.c
  ${DATA_FILES}
  ${UI_FILES}
  ${MAIN_FILES}
  ${CORE_FILES}
  ${BUILDING_FILES}
  ${EMPIRE_FILES}
  ${FIGURE_FILES}
  ${GAME_FILES}
  ${GRAPHICS_FILES}
  ${SMK_FILES})

#include(FindPkgConfig)
#pkg_search_module(SDL2 REQUIRED sdl2)
#pkg_search_module(SDL2_MIXER REQUIRED SDL2_mixer)

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)

include_directories(${SDL2_INCLUDE_DIR})
include_directories(${SDL2_MIXER_INCLUDE_DIR})

#set(LIBS ${LIBS} ${SDL_LIBRARY})
#link_libraries(${LIBS})
target_link_libraries (julius ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY})

include_directories(src)

install(TARGETS julius RUNTIME DESTINATION bin)

# Unit tests
#enable_testing()
#add_subdirectory(test)

