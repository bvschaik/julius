cmake_minimum_required(VERSION 2.6)
project(julius)

option(BUILD_SDL2STATIC "Static sdl2" ON)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/")

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
set(LIBS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs" )
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
set(SMK_FILES src/video/smacker.c src/video/smk_bitstream.c src/video/smk_hufftree.c)
file(GLOB CORE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.*)
file(GLOB BUILDING_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/building/*.*)
file(GLOB EMPIRE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/empire/*.*)
file(GLOB FIGURE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/figure/*.*)
file(GLOB GAME_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/game/*.*)
file(GLOB GRAPHICS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/*.*)
file(GLOB MAIN_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.*)
file(GLOB UI_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/*.*)
file(GLOB CITY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/city/*.*)
file(GLOB DATA_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/data/*.*)

if (BUILD_SDL2STATIC)
  set(SDL2STATIC_NAME "sdl2")
  set(SDL2STATIC_HOME "${LIBS_SOURCE_DIR}/${SDL2STATIC_NAME}" )
  set(SDL2STATIC_INCLUDE_DIR "${SDL2STATIC_HOME}/include")
  set(SDL2STATIC_LIBRARY "${CMAKE_BINARY_DIR}/libs/sdl2")
  add_subdirectory(${SDL2STATIC_HOME})
else()
  find_package(SDL2 REQUIRED)
  include_directories(${SDL2_INCLUDE_DIR})
endif()

add_executable(${PROJECT_NAME}
  linux/main2.cpp
  linux/SDLSoundDevice.c
  ${DATA_FILES}
  ${UI_FILES}
  ${MAIN_FILES}
  ${CORE_FILES}
  ${BUILDING_FILES}
  ${EMPIRE_FILES}
  ${CITY_FILES}
  ${FIGURE_FILES}
  ${GAME_FILES}
  ${GRAPHICS_FILES}
  ${SMK_FILES})

#include(FindPkgConfig)
#pkg_search_module(SDL2 REQUIRED sdl2)
#pkg_search_module(SDL2_MIXER REQUIRED SDL2_mixer)

find_package(SDL2_mixer REQUIRED)

include_directories(${SDL2_MIXER_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/include)

if (BUILD_SDL2STATIC)
  include_directories(${SDL2MINI_INCLUDE_DIR})
  link_directories(${SDL2STATIC_LIBRARY})
  target_link_libraries(${PROJECT_NAME} "SDL2")
else()
  target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})
endif()

#set(LIBS ${LIBS} ${SDL_LIBRARY})
#link_libraries(${LIBS})
target_link_libraries (julius ${SDL2_MIXER_LIBRARY})

include_directories(src)

install(TARGETS julius RUNTIME DESTINATION bin)

message( "C++ compiler is ${CMAKE_CXX_COMPILER_ID}" )
# Unit tests
#enable_testing()
#add_subdirectory(test)

