cmake_minimum_required(VERSION 2.6)
project(julius)

option(BUILD_SDL2STATIC "Static sdl2" ON)
option(BUILD_SDL2MIXERSTATIC "Static sdl2-mixer" ON)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/")

#select compiler
if (CMAKE_COMPILER_IS_CLANG)
  # using Clang
  add_definitions("-DAPP_COMPILER_NAME=\"clang\"")
elseif (CMAKE_COMPILER_IS_GNUCXX)
  # using GCC
  add_definitions("-DAPP_COMPILER_NAME=\"gcc\"")
elseif (MSVC)
  # using Visual Studio C++
  add_definitions("-DAPP_COMPILER_NAME=\"msvc\"")
endif()

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
set(LIBS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs" )
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
set(SMK_FILES src/video/smacker.c src/video/smk_bitstream.c src/video/smk_hufftree.c)
set(APP_FILES linux/main2.cpp)
file(GLOB CORE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.*)
file(GLOB BUILDING_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/building/*.*)
file(GLOB EMPIRE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/empire/*.*)
file(GLOB FIGURE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/figure/*.*)
file(GLOB INCLUDE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/include/*)
file(GLOB GAME_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/game/*.*)
file(GLOB GRAPHICS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/*.*)
file(GLOB MAIN_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.*)
file(GLOB UI_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/*.*)
file(GLOB UICORE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/uicore/*.*)
file(GLOB CITY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/city/*.*)
file(GLOB SOUND_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/sound/*.*)
file(GLOB DATA_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/data/*.*)
file(GLOB SCENARIO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/scenario/*.*)
file(GLOB CI_FILES ${CMAKE_CURRENT_SOURCE_DIR}/.travis.yml)

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

if (BUILD_SDL2MIXERSTATIC)
  set(SDL2MIXERSTATIC_NAME "sdl2-mixer")
  set(SDL2MIXERSTATIC_HOME "${LIBS_SOURCE_DIR}/${SDL2MIXERSTATIC_NAME}" )
  set(SDL2MIXERSTATIC_INCLUDE_DIR "${SDL2MIXERSTATIC_HOME}")
  set(SDL2MIXERSTATIC_LIBRARY "${CMAKE_BINARY_DIR}/libs/sdl2-mixer")
  add_subdirectory(${SDL2MIXERSTATIC_HOME})
else()
  find_package(SDL2_mixer REQUIRED)
  include_directories(${SDL2_MIXER_INCLUDE_DIR})
endif()

add_executable(${PROJECT_NAME}
  ${APP_FILES}
  ${DATA_FILES}
  ${UI_FILES}
  ${UICORE_FILES}
  ${MAIN_FILES}
  ${CORE_FILES}
  ${BUILDING_FILES}
  ${EMPIRE_FILES}
  ${CITY_FILES}
  ${FIGURE_FILES}
  ${GAME_FILES}
  ${GRAPHICS_FILES}
  ${SOUND_FILES}
  ${SMK_FILES}
  ${INCLUDE_FILES}
  ${SCENARIO_FILES}
  ${CI_FILES}
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/include)

if (BUILD_SDL2STATIC)
  include_directories(${SDL2MIXERSTATIC_INCLUDE_DIR})
  link_directories(${SDL2MIXERSTATIC_LIBRARY})
  target_link_libraries(${PROJECT_NAME} "SDL2")
else()
  target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})
endif()

if (BUILD_SDL2MIXERSTATIC)
  include_directories(${SDL2STATIC_INCLUDE_DIR})
  link_directories(${SDL2STATIC_LIBRARY})
  target_link_libraries(${PROJECT_NAME} "mixer")
else()
  target_link_libraries(${PROJECT_NAME} ${SDL2_MIXER_LIBRARY})
endif()

#set(LIBS ${LIBS} ${SDL_LIBRARY})
#link_libraries(${LIBS})
target_link_libraries (julius)

include_directories(src)

######################## Win32 settings #################################

if(WIN32)
  if(MSVC)
    ########################## set visual studio virtual folders ###############
    source_group("building" FILES ${BUILDING_FILES})
    source_group("app" FILES ${APP_FILES})
    source_group("data" FILES ${DATA_FILES})
    source_group("ui" FILES ${UI_FILES})
    source_group("uicore" FILES ${UICORE_FILES})
    source_group("main" FILES ${MAIN_FILES})
    source_group("core" FILES ${CORE_FILES})
    source_group("empire" FILES ${EMPIRE_FILES})
    source_group("city" FILES ${CITY_FILES})
    source_group("figure" FILES ${FIGURE_FILES})
    source_group("game" FILES ${GAME_FILES})
    source_group("graphics" FILES ${GRAPHICS_FILES})
    source_group("smk" FILES ${SMK_FILES})

    #############################################################################

    #avoid compiler warning
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")

    set(COMPILER_SUPPORTS_CXX11 1)
  elseif(CMAKE_COMPILER_IS_GNUCXX)
    ####################### Win32 mingw settings ##################################
    #not show console window
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows -static-libstdc++ -static-libgcc")
  endif()

   target_link_libraries(${PROJECT_NAME} "opengl32")
   target_link_libraries(${PROJECT_NAME} "winmm")
   target_link_libraries(${PROJECT_NAME} "imagehlp")

  #include steam libraries
  set(GAME_BINARY_FILENAME "julius")
endif(WIN32)

######################## osx clang settings ##################################

if(APPLE)
  # Enforce C++11
  set(USE_CPP11 ON)

  # Check for C++11 suport
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)

  set(GAME_BINARY_FILENAME "julius.app")
  INCLUDE_DIRECTORIES ( /System/Library/Frameworks )
  FIND_LIBRARY(OpenGL_LIBRARY OpenGL)
  target_link_libraries(${PROJECT_NAME} ${OpenGL_LIBRARY} )
endif(APPLE)

####################### linux gcc settings ######################################

if(UNIX AND NOT APPLE)
  set(APP_BINARY_FILENAME "julius.x64")

  # set compiler options
  if(NOT CMAKE_BUILD_TYPE_RELEASE)
    message("App: building in debug mode")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-value -g")
    add_definitions(-DDEBUG)
  else()
    #release options strip bin
    message("App: building in release mode")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
  endif()

  # Check for C++11 suport
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
endif()

install(TARGETS julius RUNTIME DESTINATION bin)

message( "C++ compiler is ${CMAKE_CXX_COMPILER_ID}" )

set_property(TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME ${APP_BINARY_FILENAME})
# Unit tests
#enable_testing()
#add_subdirectory(test)

