include_directories(.)

if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

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
except_file(TEST_CORE_FILES "core/image.c" ${CORE_FILES})
except_file(TEST_CORE_FILES "core/lang.c" ${TEST_CORE_FILES})
except_file(TEST_CORE_FILES "core/speed.c" ${TEST_CORE_FILES})
except_file(TEST_BUILDING_FILES "building/model.c" ${BUILDING_FILES})

add_executable(compare
    sav/compare.c
    sav/sav_compare.c
    stub/log.c
    ${PROJECT_SOURCE_DIR}/src/core/zip.c
)

add_executable(autopilot
    sav/sav_compare.c
    sav/run.c
    stub/image.c
    stub/input.c
    stub/lang.c
    stub/log.c
    stub/model.c
    stub/sound_device.c
    stub/ui.c
    stub/video.c
    ${PROJECT_SOURCE_DIR}/src/platform/file_manager.c
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
    ${EDITOR_FILES}
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

# Distant battle
add_integration_test(sav_distantbattle1 db-fort1.sav db-fort1-done.sav 6328)
add_integration_test(sav_distantbattle2 db-fort2.sav db-fort2-done.sav 6335)
add_integration_test(sav_distantbattle3 db-fort2.sav db-fort2-done2.sav 11197)

# Routing table full kills figures
add_integration_test(sav_routing_full routing-full.sav routing-full-kill.sav 7)

# God curses
add_integration_test(sav_curses1 curses.sav curses-done.sav 13350)
add_integration_test(sav_curses2 mars-wrath.sav mars-wrath-after.sav 1016)

# Earthquake destroying buildings
add_integration_test(sav_earthquake0 earthquake.sav earthquake-start.sav 371)
add_integration_test(sav_earthquake1 earthquake.sav earthquake-during1.sav 551)
add_integration_test(sav_earthquake2 earthquake.sav earthquake-during2.sav 1071)
add_integration_test(sav_earthquake3 earthquake.sav earthquake-during3.sav 1602)
add_integration_test(sav_earthquake4 earthquake.sav earthquake-during4.sav 2155)
add_integration_test(sav_earthquake5 earthquake.sav earthquake-after.sav 3748)

# Testing map with tile offsets >127
add_integration_test(sav_edge1 edge-start.sav edge-battle-before.sav 835)
add_integration_test(sav_edge2 edge-start.sav edge-battle-start.sav 1278)
add_integration_test(sav_edge3 edge-start.sav edge-battle-during.sav 1513)
add_integration_test(sav_edge4 edge-start.sav edge-battle-after.sav 1890)

# Test with bigger cities
add_integration_test(sav_massilia1 brugle-massilia-start.sav brugle-massilia-1.sav 4)
add_integration_test(sav_massilia2 brugle-massilia-start.sav brugle-massilia-2.sav 57)
add_integration_test(sav_massilia3 brugle-massilia-start.sav brugle-massilia-3.sav 391)

add_integration_test(sav_valentia1 valentia57.sav valentia57-after.sav 1026)
add_integration_test(sav_lugdunum1 brugle-lugdunum.sav brugle-lugdunum-after.sav 1176)

add_integration_test(sav_native1 brugle-lugdunum-native.sav brugle-lugdunum-native-after.sav 1678)
add_integration_test(sav_native2 cicero-lugdunum-trade.sav cicero-lugdunum-trade-after.sav 926)

add_integration_test(sav_palace1 brugle-palacepeaks.sav brugle-palacepeaks-2.sav 2562)
