set(MPG123_LOCATION ${PROJECT_SOURCE_DIR}/ext)

file(GLOB children
    RELATIVE ${MPG123_LOCATION}
    CONFIGURE_DEPENDS
    ${MPG123_LOCATION}/mpg123
    ${MPG123_LOCATION}/mpg123-*
)
foreach(child ${children})
    if(IS_DIRECTORY "${MPG123_LOCATION}/${child}")
        set(MPG123_EXT_DIR "${MPG123_LOCATION}/${child}")
        break()
    endif()
endforeach()
unset(children)
unset(MPG123_LOCATION)

SET(MPG123_SEARCH_PATHS
    ${MPG123_EXT_DIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local
    /usr
    /sw # Fink
    /opt/local # DarwinPorts
    /opt/csw # Blastwave
    /opt
    ${CMAKE_FIND_ROOT_PATH}
)

FIND_PATH(MPG123_INCLUDE_DIR mpg123.h
            PATH_SUFFIXES include
            PATHS ${MPG123_SEARCH_PATHS}
            NO_CMAKE_FIND_ROOT_PATH
          )

FIND_LIBRARY(MPG123_LIBRARY
                NAMES mpg123
                PATH_SUFFIXES lib64 lib
                PATHS ${MPG123_SEARCH_PATHS}
                NO_CMAKE_FIND_ROOT_PATH
            )

if(MPG123_INCLUDE_DIR)
    file(STRINGS "${MPG123_INCLUDE_DIR}/mpg123.h" MPG123_VERSION_LINE REGEX "^.*libmpg123.*\(version (.+)\)$")
    string(REGEX REPLACE "^.*libmpg123.*version ([0-9]+\.[0-9]+\.[0-9]+).*$" "\\1" MPG123_VERSION "${MPG123_VERSION_LINE}")
    unset(MPG123_VERSION_LINE)
endif()

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(MPG123
                                  REQUIRED_VARS MPG123_LIBRARY MPG123_INCLUDE_DIR
                                  VERSION_VAR MPG123_VERSION)
