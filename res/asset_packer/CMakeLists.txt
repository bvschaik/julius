cmake_minimum_required(VERSION 3.1)
include(CMakeDependentOption)

option(SYSTEM_LIBS "Use system libraries when available." ON)
option(FORMAT_XML "Prettify the generated XML" OFF)
option(PACK_XMLS "Pack the images from the XML files" ON)
option(PACK_CURSORS "Pack the cursor images" ON)

set(SHORT_NAME "asset_packer")

project(${SHORT_NAME} C)

set(MAIN_DIR "${PROJECT_SOURCE_DIR}/../..")

set(EXPAT_FILES
    ${MAIN_DIR}/ext/expat/xmlparse.c
    ${MAIN_DIR}/ext/expat/xmlrole.c
    ${MAIN_DIR}/ext/expat/xmltok.c
)

set(PNG_FILES
    ${MAIN_DIR}/ext/png/png.c
    ${MAIN_DIR}/ext/png/pngerror.c
    ${MAIN_DIR}/ext/png/pngget.c
    ${MAIN_DIR}/ext/png/pngmem.c
    ${MAIN_DIR}/ext/png/pngpread.c
    ${MAIN_DIR}/ext/png/pngread.c
    ${MAIN_DIR}/ext/png/pngrio.c
    ${MAIN_DIR}/ext/png/pngrtran.c
    ${MAIN_DIR}/ext/png/pngrutil.c
    ${MAIN_DIR}/ext/png/pngset.c
    ${MAIN_DIR}/ext/png/pngtrans.c
    ${MAIN_DIR}/ext/png/pngwio.c
    ${MAIN_DIR}/ext/png/pngwrite.c
    ${MAIN_DIR}/ext/png/pngwtran.c
    ${MAIN_DIR}/ext/png/pngwutil.c
)

set(ZLIB_FILES
    ${MAIN_DIR}/ext/zlib/adler32.c
    ${MAIN_DIR}/ext/zlib/crc32.c
    ${MAIN_DIR}/ext/zlib/deflate.c
    ${MAIN_DIR}/ext/zlib/inffast.c
    ${MAIN_DIR}/ext/zlib/inflate.c
    ${MAIN_DIR}/ext/zlib/inftrees.c
    ${MAIN_DIR}/ext/zlib/trees.c
    ${MAIN_DIR}/ext/zlib/zutil.c
)

set(CORE_FILES
    ${MAIN_DIR}/src/core/array.c
    ${MAIN_DIR}/src/core/calc.c
    ${MAIN_DIR}/src/core/dir.c
    ${MAIN_DIR}/src/core/file.c
    ${MAIN_DIR}/src/core/image_packer.c
    ${MAIN_DIR}/src/core/png_read.c
    ${MAIN_DIR}/src/core/string.c
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

if(FORMAT_XML)
    add_compile_definitions(FORMAT_XML)
endif()

if (PACK_XMLS)
    add_compile_definitions(PACK_XMLS)
endif()

if (PACK_CURSORS)
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
)

if(SYSTEM_LIBS)
    find_package(ZLIB)
    find_package(PNG)
    find_package(EXPAT)
endif()

if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    target_link_libraries(${SHORT_NAME} ${ZLIB_LIBRARIES})
else()
    if(SYSTEM_LIBS)
        message(STATUS "Zlib was not found but that's ok: falling back to internal version")
    endif()
    include_directories(${MAIN_DIR}/ext/zlib)
    target_sources(${SHORT_NAME} PRIVATE "${ZLIB_FILES}")
endif()

if(PNG_FOUND)
    include_directories(${PNG_INCLUDE_DIRS})
    target_link_libraries(${SHORT_NAME} ${PNG_LIBRARIES})
else()
    if(SYSTEM_LIBS)
        message(STATUS "PNG was not found but that's ok: falling back to internal version")
    endif()
    include_directories(${MAIN_DIR}/ext/png)
    target_sources(${SHORT_NAME} PRIVATE "${PNG_FILES}")
endif()

if(EXPAT_FOUND)
    include_directories(${EXPAT_INCLUDE_DIRS})
    target_link_libraries(${SHORT_NAME} ${EXPAT_LIBRARIES})
else()
    if(SYSTEM_LIBS)
        message(STATUS "Expat was not found but that's ok: falling back to internal version")
    endif()
    include_directories(${MAIN_DIR}/ext/expat)
    target_sources(${SHORT_NAME} PRIVATE "${EXPAT_FILES}")
    if(NOT WIN32)
        add_compile_definitions(XML_DEV_URANDOM)
    endif()
endif()

include_directories(${MAIN_DIR}/src)
if(MSVC)
    include_directories(${MAIN_DIR}/ext/dirent)
endif()

if (UNIX AND NOT APPLE AND (CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang"))
    target_link_libraries(${SHORT_NAME} m)
endif()
