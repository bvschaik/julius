cmake_minimum_required(VERSION 3.1...3.27.0)
include(CMakeDependentOption)

option(FORMAT_XML "Prettify the generated XML" OFF)
option(PACK_XMLS "Pack the images from the XML files" ON)
option(PACK_CURSORS "Pack the cursor images" ON)

set(SHORT_NAME "asset_packer")

project(${SHORT_NAME} C)

set(MAIN_DIR "${PROJECT_SOURCE_DIR}/../..")

set(SXML_FILES
    ${MAIN_DIR}/ext/sxml/sxml.c
)

set(SPNG_FILES
    ${MAIN_DIR}/ext/spng/spng.c
)

set(CORE_FILES
    ${MAIN_DIR}/src/core/array.c
    ${MAIN_DIR}/src/core/buffer.c
    ${MAIN_DIR}/src/core/calc.c
    ${MAIN_DIR}/src/core/dir.c
    ${MAIN_DIR}/src/core/file.c
    ${MAIN_DIR}/src/core/image_packer.c
    ${MAIN_DIR}/src/core/png_read.c
    ${MAIN_DIR}/src/core/string.c
    ${MAIN_DIR}/src/core/xml_exporter.c
    ${MAIN_DIR}/src/core/xml_parser.c
)

set(ASSETS_FILES
    ${MAIN_DIR}/src/assets/xml.c
    ${MAIN_DIR}/src/assets/group.c
    ${MAIN_DIR}/src/assets/image.c
    ${MAIN_DIR}/src/assets/layer.c
)

set(PLATFORM_FILES
    ${MAIN_DIR}/src/platform/file_manager.c
)

add_compile_definitions(BUILDING_ASSET_PACKER)
add_compile_definitions(MINIZ_IMPLEMENTATION)

if(FORMAT_XML)
    add_compile_definitions(FORMAT_XML)
endif()

if(PACK_XMLS)
    add_compile_definitions(PACK_XMLS)
endif()

if(PACK_CURSORS)
    add_compile_definitions(PACK_CURSORS)
endif()

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(${SHORT_NAME}
    ${PROJECT_SOURCE_DIR}/src/asset_packer.c
    ${ASSETS_FILES}
    ${CORE_FILES}
    ${PLATFORM_FILES}
    ${SXML_FILES}
    ${SPNG_FILES}
)

include_directories(${MAIN_DIR}/src)
include_directories(${MAIN_DIR}/ext)

if(MSVC)
    include_directories(${MAIN_DIR}/ext/dirent)
endif()

if(UNIX AND NOT APPLE AND(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang"))
    target_link_libraries(${SHORT_NAME} m)
endif()
