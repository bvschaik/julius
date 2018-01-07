include_directories(.)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

function(except_file var excluded_file)
    set(list_var "")
    foreach(f ${ARGN})
        if(NOT ${f} MATCHES ".*${excluded_file}")
            list(APPEND list_var ${f})
        endif()
    endforeach(f)
    set(${var} "${list_var}" PARENT_SCOPE)
endfunction(except_file)

# Replace some source files with stubs
except_file(TEST_CORE_FILES "core/lang.c" ${CORE_FILES})
except_file(TEST_BUILDING_FILES "building/model.c" ${BUILDING_FILES})

add_executable(compare
    sav/compare.c
    sav/sav_compare.c
    ${PROJECT_SOURCE_DIR}/src/core/zip.c
    ${PROJECT_SOURCE_DIR}/src/core/debug.c
)

add_executable(autopilot
    sav/sav_compare.c
    sav/run.c
    stub/image.c
    stub/input.c
    stub/lang.c
    stub/model.c
    stub/sound_device.c
    stub/system.c
    stub/ui.c
    stub/video.c
    ${TEST_CORE_FILES}
    ${TEST_BUILDING_FILES}
    ${CITY_FILES}
    ${EMPIRE_FILES}
    ${FIGURE_FILES}
    ${FIGURETYPE_FILES}
    ${GAME_FILES}
    ${MAP_FILES}
    ${SCENARIO_FILES}
    ${SOUND_FILES}
    ${PROJECT_SOURCE_DIR}/src/Data/Model.c
)

file(COPY data/c3.emp DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY data/c32.emp DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

function(add_integration_test name input_sav compare_sav ticks)
    string(REPLACE ".sav" "-actual.sav" output_sav ${compare_sav})
    file(COPY data/${input_sav} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY data/${compare_sav} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    add_test(NAME ${name} COMMAND autopilot ${input_sav} ${output_sav} ${compare_sav} ${ticks})
endfunction(add_integration_test)

add_integration_test(sav_tower tower.sav tower2.sav 1785)
add_integration_test(sav_request1 request_start.sav request_orig.sav 908)
add_integration_test(sav_request2 request_start.sav request_orig2.sav 6556)

# Caesar invasion plus ballista
add_integration_test(sav_caesar1 kknight.sav kknight2.sav 686)
add_integration_test(sav_caesar2 kknight.sav kknight3.sav 1087)
add_integration_test(sav_caesar3 kknight.sav kknight4.sav 1287)
add_integration_test(sav_caesar4 kknight.sav kknight5.sav 1494)

# Invasion
add_integration_test(sav_invasion1 inv0.sav inv1.sav 1973)
add_integration_test(sav_invasion2 inv0.sav inv2.sav 3521)
add_integration_test(sav_invasion3 inv0.sav inv3.sav 5105)
add_integration_test(sav_invasion4 inv0.sav inv4.sav 6777)
add_integration_test(sav_invasion5 inv0.sav inv5.sav 8563)

# Routing table full kills figures
add_integration_test(sav_routing_full routing-full.sav routing-full-kill.sav 7)
