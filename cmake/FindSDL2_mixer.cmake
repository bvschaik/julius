# - Locate SDL_mixer library
# This module defines:
#  SDL2_MIXER_LIBRARIES, the name of the library to link against
#  SDL2_MIXER_INCLUDE_DIRS, where to find the headers
#  SDL2_MIXER_FOUND, if false, do not try to link against
#  SDL2_MIXER_VERSION_STRING - human-readable string containing the version of SDL_mixer
#
# $SDLDIR is an environment variable that would
# correspond to the ./configure --prefix=$SDLDIR
# used in building SDL.
#
# Created by Eric Wing. This was influenced by the FindSDL.cmake
# module, but with modifications to recognize OS X frameworks and
# additional Unix paths (FreeBSD, etc).

#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
# Copyright 2012 Benjamin Eikel
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

GET_SDL_EXT_DIR(SDL_MIXER_EXT_DIR "mixer")

IF(${TARGET_PLATFORM} STREQUAL "android")
    find_package(SDL2 REQUIRED CONFIG)
    string(TOLOWER ${CMAKE_BUILD_TYPE} ANDROID_BUILD_DIR)
    set(SDL2_MIXER_LIBRARY SDL2::SDL2_mixer)
ELSE()
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(SDL2_ARCH_64 TRUE)
      set(SDL2_PROCESSOR_ARCH "x64")
    else()
      set(SDL2_ARCH_64 FALSE)
      set(SDL2_PROCESSOR_ARCH "x86")
    endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

    if(MINGW AND DEFINED SDL_MIXER_EXT_DIR)
        if(SDL2_ARCH_64)
          set(SDL_MINGW_EXT_DIR "${SDL_MIXER_EXT_DIR}/x86_64-w64-mingw32")
        else()
          set(SDL_MINGW_EXT_DIR "${SDL_MIXER_EXT_DIR}/i686-w64-mingw32")
        endif()
    endif()

    SET(SDL2_SEARCH_PATHS
        ${SDL_MIXER_EXT_DIR}
        ${SDL_MINGW_EXT_DIR}
        ~/Library/Frameworks
        /Library/Frameworks
        /sw # Fink
        /opt/local # DarwinPorts
        /opt/csw # Blastwave
        /opt
        /boot/system/develop/headers/SDL2 # Haiku
        ${CMAKE_FIND_ROOT_PATH}
    )

    if(APPLE)
      # Try to find the include in the SDL2_mixer framework bundle
      # This fixes CMake finding the header from SDL_mixer 1.2 when both 1.2 and 2.0 are installed
      find_path(SDL2_MIXER_INCLUDE_DIR SDL2_mixer/SDL_mixer.h
        HINTS
          ENV SDL2MIXERDIR
          ENV SDL2DIR
        PATH_SUFFIXES include include/SDL2
        PATHS ${SDL2_SEARCH_PATHS}
      )
      set(SDL2_MIXER_INCLUDE_DIR "${SDL2_MIXER_INCLUDE_DIR}/Headers")
    endif()

    if(NOT APPLE OR NOT EXISTS "${SDL2_MIXER_INCLUDE_DIR}/SDL_mixer.h")
      find_path(SDL2_MIXER_INCLUDE_DIR SDL_mixer.h
        HINTS
          ENV SDL2MIXERDIR
          ENV SDL2DIR
        PATH_SUFFIXES include include/SDL2
        PATHS ${SDL2_SEARCH_PATHS}
        NO_CMAKE_FIND_ROOT_PATH
      )
    endif()

    find_library(SDL2_MIXER_LIBRARY
      NAMES SDL2_mixer
      HINTS
        ENV SDL2MIXERDIR
        ENV SDL2DIR
      PATH_SUFFIXES lib64 lib lib/${SDL2_PROCESSOR_ARCH}
      PATHS ${SDL2_SEARCH_PATHS}
      NO_CMAKE_FIND_ROOT_PATH
    )
ENDIF()

if(SDL2_MIXER_INCLUDE_DIR AND EXISTS "${SDL2_MIXER_INCLUDE_DIR}/SDL_mixer.h")
    file(STRINGS "${SDL2_MIXER_INCLUDE_DIR}/SDL_mixer.h" SDL2_MIXER_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_MIXER_MAJOR_VERSION[ \t]+[0-9]+$")
    file(STRINGS "${SDL2_MIXER_INCLUDE_DIR}/SDL_mixer.h" SDL2_MIXER_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_MIXER_MINOR_VERSION[ \t]+[0-9]+$")
    file(STRINGS "${SDL2_MIXER_INCLUDE_DIR}/SDL_mixer.h" SDL2_MIXER_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_MIXER_PATCHLEVEL[ \t]+[0-9]+$")
    string(REGEX REPLACE "^#define[ \t]+SDL_MIXER_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_MIXER_VERSION_MAJOR "${SDL2_MIXER_VERSION_MAJOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+SDL_MIXER_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_MIXER_VERSION_MINOR "${SDL2_MIXER_VERSION_MINOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+SDL_MIXER_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL2_MIXER_VERSION_PATCH "${SDL2_MIXER_VERSION_PATCH_LINE}")
    set(SDL2_MIXER_VERSION_STRING ${SDL2_MIXER_VERSION_MAJOR}.${SDL2_MIXER_VERSION_MINOR}.${SDL2_MIXER_VERSION_PATCH})
    unset(SDL2_MIXER_VERSION_MAJOR_LINE)
    unset(SDL2_MIXER_VERSION_MINOR_LINE)
    unset(SDL2_MIXER_VERSION_PATCH_LINE)
    unset(SDL2_MIXER_VERSION_MAJOR)
    unset(SDL2_MIXER_VERSION_MINOR)
    unset(SDL2_MIXER_VERSION_PATCH)
endif()


set(SDL2_MIXER_LIBRARIES ${SDL2_MIXER_LIBRARY})
set(SDL2_MIXER_INCLUDE_DIRS ${SDL2_MIXER_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2_mixer
                                  REQUIRED_VARS SDL2_MIXER_LIBRARIES
                                  VERSION_VAR SDL2_MIXER_VERSION_STRING)
