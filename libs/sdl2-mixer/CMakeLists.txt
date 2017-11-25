project(mixer)

set( MIXER_SOURCES_LIST
      mixer.c
      dynamic_ogg.c
      playmus.c
      playwave.c
      wavestream.c
      load_aiff.c
      load_ogg.c
      music.c
      load_voc.c
      music_ogg.c
      effects_internal.c
      effect_position.c )

# Enable this if you want to support loading OGG Vorbis music via Tremor
# The library path should be a relative path to this directory.
set(OGG_LIBRARY_PATH ${PROJECT_SOURCE_DIR}/external/libogg-1.3.1)
set(VORBIS_LIBRARY_PATH ${PROJECT_SOURCE_DIR}/external/libvorbisidec-1.2.1)

include_directories(${OGG_LIBRARY_PATH}/include)
include_directories(${VORBIS_LIBRARY_PATH})
include_directories(${SDL2STATIC_INCLUDE_DIR})

if(WIN32 OR APPLE)
include_directories(${XCOMPILE_SOURCE_DIR}/xcompile)
endif()

if(UNIX AND NOT APPLE AND X32_BUILD)
  message("WARNING! LibMixer mini build with x86 arch")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
endif()

file(GLOB VORBIS_SOURCES_LIST ${VORBIS_LIBRARY_PATH}/*.c)

add_definitions(-DOGG_MUSIC)
add_definitions(-DOGG_USE_TREMOR)

set( VORBIS_SRC
        ${VORBIS_LIBRARY_PATH}/mdct.c
        ${VORBIS_LIBRARY_PATH}/block.c
        ${VORBIS_LIBRARY_PATH}/window.c
        ${VORBIS_LIBRARY_PATH}/synthesis.c
        ${VORBIS_LIBRARY_PATH}/info.c
        ${VORBIS_LIBRARY_PATH}/floor1.c
        ${VORBIS_LIBRARY_PATH}/floor0.c
        ${VORBIS_LIBRARY_PATH}/vorbisfile.c
        ${VORBIS_LIBRARY_PATH}/res012.c
        ${VORBIS_LIBRARY_PATH}/mapping0.c
        ${VORBIS_LIBRARY_PATH}/registry.c
        ${VORBIS_LIBRARY_PATH}/codebook.c
        ${VORBIS_LIBRARY_PATH}/sharedbook.c
        ${OGG_LIBRARY_PATH}/src/framing.c
        ${OGG_LIBRARY_PATH}/src/bitwise.c
)

add_library(${PROJECT_NAME} STATIC ${MIXER_SOURCES_LIST} ${VORBIS_SRC})

target_link_libraries( ${PROJECT_NAME} ${SDL2MIXERSTATIC_LIBRARY} )

set(MIXER_INCLUDE_DIR ${PROJECT_SOURCE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)

set(MIXER_LIBRARY ${PROJECT_NAME}
    CACHE INTERNAL "${PROJECT_NAME}: Library Name" FORCE)
