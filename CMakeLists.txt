cmake_minimum_required(VERSION 3.1)
include(CMakeDependentOption)

set(TARGET_PLATFORM "default" CACHE STRING "Platform to cross-compile for. Options: vita switch android emscripten. Leave blank for no cross compilation")
set_property(CACHE TARGET_PLATFORM PROPERTY STRINGS default vita switch android emscripten)

string(TOLOWER ${TARGET_PLATFORM} TARGET_PLATFORM)

option(DRAW_FPS "Draw FPS on the top left corner of the window." OFF)
option(DRAW_ROUTING "Draw routing debug information." OFF)
option(DRAW_HIGHWAY_TERRAIN "Draw highway debug information." OFF)
option(DRAW_ROAD_NETWORK_IDS "Draw road network IDs for debugging." OFF)
option(DRAW_TILE_COORDS "Draw tile coordinates." OFF)
option(SYSTEM_LIBS "Use system libraries when available." ON)
option(EMSCRIPTEN_LOAD_SDL_PORTS "Load SDL and SDL_mixer emscripten ports instead of compiling them" OFF)
option(LINK_MPG123 "Link mpg123 statically to Julius instead of relying on a library." OFF)

if(${TARGET_PLATFORM} STREQUAL "vita" AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VITASDK})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
    else()
        message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
    endif()
endif()

if(${TARGET_PLATFORM} STREQUAL "switch" AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{DEVKITPRO})
        set(DEVKITPRO $ENV{DEVKITPRO})
        set(CMAKE_TOOLCHAIN_FILE "./cmake/switch.cmake" CACHE PATH "toolchain file")
    else()
        message(FATAL_ERROR "Please define DEVKITPRO to point to your SDK path!")
    endif()
endif()

if(${TARGET_PLATFORM} STREQUAL "emscripten" AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{EMSDK})
        set(EMSDK $ENV{EMSDK})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" CACHE PATH "toolchain file")
    else()
        message(FATAL_ERROR "Please define EMSDK to point to your Emscripten SDK path!")
    endif()
endif()

set(SHORT_NAME augustus)
set(USER_FRIENDLY_NAME Augustus)
project(${SHORT_NAME} C)

if (${TARGET_PLATFORM} STREQUAL "vita")
    include("${VITASDK}/share/vita.cmake" REQUIRED)
    set(VITA_TITLEID  "AUGUSTUS1")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/")

if (CMAKE_VERSION VERSION_LESS 3.1)
    if (CMAKE_COMPILER_IS_GNUCC)
        set(CMAKE_C_FLAGS "-std=gnu99 ${CMAKE_C_FLAGS}")
    endif()
else()
    set(CMAKE_C_STANDARD 99)
endif()

if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror=implicit-function-declaration")
    if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 5.0)
        set(CMAKE_C_FLAGS "-Werror=incompatible-pointer-types -Werror=int-conversion -Wstrict-prototypes ${CMAKE_C_FLAGS}")
    endif()
endif()

# Fix bug in cmake 3.24.0 that expects the resource compiler for mingw to have the toolchain prefix in the name
if (CMAKE_VERSION VERSION_EQUAL 3.24.0 AND MINGW)
    set(CMAKE_RC_COMPILER "windres")
endif()

set(PROJECT_VERSION_MAJOR 3)
set(PROJECT_VERSION_MINOR 2)
set(PROJECT_VERSION_PATCH 0)
set(IS_RELEASE_VERSION FALSE)

if(IS_RELEASE_VERSION)
    set(PROJECT_VERSION_TWEAK 0)
    set(VERSION_REVISION "")
elseif (EXISTS "${CMAKE_SOURCE_DIR}/.git")
    find_package(Git)
    if(Git_FOUND)
        execute_process(
            COMMAND git rev-list --count HEAD ^tags/v${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            RESULT_VARIABLE TWEAK_RESULT
            OUTPUT_VARIABLE PROJECT_VERSION_TWEAK
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        execute_process(
            COMMAND git rev-parse --short --verify HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE VERSION_COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        execute_process(
            COMMAND git diff-index --quiet HEAD --
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            RESULT_VARIABLE VERSION_COMMIT_DIRTY
        )
        if(VERSION_COMMIT_DIRTY)
            set(DIRTY_POSTFIX "-dirty")
        else()
            set(DIRTY_POSTFIX "")
        endif()
        if(NOT TWEAK_RESULT EQUAL 0)
            # Happens on a shallow git clone, like Travis does. Append date to version ref.
            set(PROJECT_VERSION_TWEAK 0)
            string(TIMESTAMP TODAY "%Y%m%d")
            set(VERSION_REVISION "-${TODAY}-${VERSION_COMMIT_HASH}${DIRTY_POSTFIX}")
        else()
            set(VERSION_REVISION ".${PROJECT_VERSION_TWEAK}-${VERSION_COMMIT_HASH}${DIRTY_POSTFIX}")
        endif()
    else()
        set(PROJECT_VERSION_TWEAK 0)
        set(VERSION_REVISION " unknown development version")
    endif()
else()
    set(PROJECT_VERSION_TWEAK 0)
    set(VERSION_REVISION " unknown development version")
endif()

if(${TARGET_PLATFORM} STREQUAL "vita")
    string(LENGTH "${PROJECT_VERSION_MAJOR}" NUM_DIGITS)
    if(NUM_DIGITS EQUAL 1)
        set(VITA_VERSION_MAJOR "0${PROJECT_VERSION_MAJOR}")
    elseif(NUM_DIGITS EQUAL 2)
        set(VITA_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
    else()
        message(FATAL_ERROR "On Vita, major version number can only have one or two digits, found ${NUM_DIGITS} digits instead")
    endif()

    string(LENGTH "${PROJECT_VERSION_MINOR}" NUM_DIGITS)
    if(NUM_DIGITS EQUAL 1)
        set(VITA_VERSION_MINOR "0${PROJECT_VERSION_MINOR}")
    elseif(NUM_DIGITS EQUAL 2)
        set(VITA_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
    else()
        message(FATAL_ERROR "On Vita, minor version number can only have one or two digits, found ${NUM_DIGITS} digits instead")
    endif()

    set(VITA_VERSION "${VITA_VERSION_MAJOR}.${VITA_VERSION_MINOR}")
endif()

if (${TARGET_PLATFORM} STREQUAL "switch")
    string(SUBSTRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}${VERSION_REVISION}" 0 15 SWITCH_VERSION)
endif()

configure_file(${PROJECT_SOURCE_DIR}/gen/version.c.in ${PROJECT_SOURCE_DIR}/src/platform/version.c)
configure_file(${PROJECT_SOURCE_DIR}/gen/version.rc.in ${PROJECT_SOURCE_DIR}/res/version.rc)
configure_file(${PROJECT_SOURCE_DIR}/gen/version.txt.in ${PROJECT_SOURCE_DIR}/res/version.txt)

if(${TARGET_PLATFORM} STREQUAL "emscripten")
    configure_file(${PROJECT_SOURCE_DIR}/gen/shell.html.in ${PROJECT_SOURCE_DIR}/res/shell.html)
endif()

if(DRAW_FPS)
    add_definitions(-DDRAW_FPS)
endif()
if(DRAW_ROUTING)
    add_definitions(-DDRAW_ROUTING)
endif()
if(DRAW_HIGHWAY_TERRAIN)
    add_definitions(-DDRAW_HIGHWAY_TERRAIN)
endif()
if(DRAW_TILE_COORDS)
    add_definitions(-DDRAW_TILE_COORDS)
endif()
if(DRAW_ROAD_NETWORK_IDS)
    add_definitions(-DDRAW_ROAD_NETWORK_IDS)
endif()

set(ASSETS_DIR ${PROJECT_SOURCE_DIR}/res/assets)
if (EXISTS ${PROJECT_SOURCE_DIR}/res/packed_assets)
    set(ASSETS_DIR ${PROJECT_SOURCE_DIR}/res/packed_assets)
endif()

set(EXPAT_FILES
    ext/expat/xmlparse.c
    ext/expat/xmlrole.c
    ext/expat/xmltok.c
)

set(TINYFD_FILES
    ext/tinyfiledialogs/tinyfiledialogs.c
)

set(PNG_FILES
    ext/png/png.c
    ext/png/pngerror.c
    ext/png/pngget.c
    ext/png/pngmem.c
    ext/png/pngpread.c
    ext/png/pngread.c
    ext/png/pngrio.c
    ext/png/pngrtran.c
    ext/png/pngrutil.c
    ext/png/pngset.c
    ext/png/pngtrans.c
    ext/png/pngwio.c
    ext/png/pngwrite.c
    ext/png/pngwtran.c
    ext/png/pngwutil.c
)

set(ZLIB_FILES
    ext/zlib/adler32.c
    ext/zlib/crc32.c
    ext/zlib/deflate.c
    ext/zlib/inffast.c
    ext/zlib/inflate.c
    ext/zlib/inftrees.c
    ext/zlib/trees.c
    ext/zlib/zutil.c
)

set(PLATFORM_FILES
    ${PROJECT_SOURCE_DIR}/src/platform/arguments.c
    ${PROJECT_SOURCE_DIR}/src/platform/augustus.c
    ${PROJECT_SOURCE_DIR}/src/platform/crash_handler.c
    ${PROJECT_SOURCE_DIR}/src/platform/cursor.c
    ${PROJECT_SOURCE_DIR}/src/platform/file_manager.c
    ${PROJECT_SOURCE_DIR}/src/platform/file_manager_cache.c
    ${PROJECT_SOURCE_DIR}/src/platform/icon.c
    ${PROJECT_SOURCE_DIR}/src/platform/joystick.c
    ${PROJECT_SOURCE_DIR}/src/platform/keyboard_input.c
    ${PROJECT_SOURCE_DIR}/src/platform/log.c
    ${PROJECT_SOURCE_DIR}/src/platform/mouse.c
    ${PROJECT_SOURCE_DIR}/src/platform/platform.c
    ${PROJECT_SOURCE_DIR}/src/platform/prefs.c
    ${PROJECT_SOURCE_DIR}/src/platform/renderer.c
    ${PROJECT_SOURCE_DIR}/src/platform/screen.c
    ${PROJECT_SOURCE_DIR}/src/platform/sound_device.c
    ${PROJECT_SOURCE_DIR}/src/platform/touch.c
    ${PROJECT_SOURCE_DIR}/src/platform/version.c
    ${PROJECT_SOURCE_DIR}/src/platform/virtual_keyboard.c
)

if("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
    list(APPEND PLATFORM_FILES ${PROJECT_SOURCE_DIR}/src/platform/debug.c)
endif()
if (${TARGET_PLATFORM} STREQUAL "vita")
    set(PLATFORM_FILES
        ${PLATFORM_FILES}
        ${PROJECT_SOURCE_DIR}/src/platform/vita/vita.c
        ${PROJECT_SOURCE_DIR}/src/platform/vita/vita_keyboard.c
    )
elseif (${TARGET_PLATFORM} STREQUAL "switch")
    set(PLATFORM_FILES
        ${PLATFORM_FILES}
        ${PROJECT_SOURCE_DIR}/src/platform/switch/switch.c
    )
elseif (${TARGET_PLATFORM} STREQUAL "android")
    set(PLATFORM_FILES
        ${PLATFORM_FILES}
        ${PROJECT_SOURCE_DIR}/src/platform/android/android.c
        ${PROJECT_SOURCE_DIR}/src/platform/android/asset_handler.c
        ${PROJECT_SOURCE_DIR}/src/platform/android/jni.c
    )
endif()

set(CORE_FILES
    ${PROJECT_SOURCE_DIR}/src/core/array.c
    ${PROJECT_SOURCE_DIR}/src/core/buffer.c
    ${PROJECT_SOURCE_DIR}/src/core/calc.c
    ${PROJECT_SOURCE_DIR}/src/core/config.c
    ${PROJECT_SOURCE_DIR}/src/core/dir.c
    ${PROJECT_SOURCE_DIR}/src/core/encoding.c
    ${PROJECT_SOURCE_DIR}/src/core/encoding_japanese.c
    ${PROJECT_SOURCE_DIR}/src/core/encoding_korean.c
    ${PROJECT_SOURCE_DIR}/src/core/encoding_simp_chinese.c
    ${PROJECT_SOURCE_DIR}/src/core/encoding_trad_chinese.c
    ${PROJECT_SOURCE_DIR}/src/core/file.c
    ${PROJECT_SOURCE_DIR}/src/core/hotkey_config.c
    ${PROJECT_SOURCE_DIR}/src/core/image.c
    ${PROJECT_SOURCE_DIR}/src/core/image_packer.c
    ${PROJECT_SOURCE_DIR}/src/core/io.c
    ${PROJECT_SOURCE_DIR}/src/core/lang.c
    ${PROJECT_SOURCE_DIR}/src/core/locale.c
    ${PROJECT_SOURCE_DIR}/src/core/png_read.c
    ${PROJECT_SOURCE_DIR}/src/core/random.c
    ${PROJECT_SOURCE_DIR}/src/core/smacker.c
    ${PROJECT_SOURCE_DIR}/src/core/speed.c
    ${PROJECT_SOURCE_DIR}/src/core/string.c
    ${PROJECT_SOURCE_DIR}/src/core/time.c
    ${PROJECT_SOURCE_DIR}/src/core/xml_parser.c
    ${PROJECT_SOURCE_DIR}/src/core/zip.c
    ${PROJECT_SOURCE_DIR}/src/core/zlib_helper.c
)

set(BUILDING_FILES
    ${PROJECT_SOURCE_DIR}/src/building/animation.c
    ${PROJECT_SOURCE_DIR}/src/building/barracks.c
    ${PROJECT_SOURCE_DIR}/src/building/building.c
    ${PROJECT_SOURCE_DIR}/src/building/building_state.c
    ${PROJECT_SOURCE_DIR}/src/building/building_variant.c
    ${PROJECT_SOURCE_DIR}/src/building/caravanserai.c
    ${PROJECT_SOURCE_DIR}/src/building/clone.c
    ${PROJECT_SOURCE_DIR}/src/building/connectable.c
    ${PROJECT_SOURCE_DIR}/src/building/construction.c
    ${PROJECT_SOURCE_DIR}/src/building/construction_building.c
    ${PROJECT_SOURCE_DIR}/src/building/construction_clear.c
    ${PROJECT_SOURCE_DIR}/src/building/construction_routed.c
    ${PROJECT_SOURCE_DIR}/src/building/construction_warning.c
    ${PROJECT_SOURCE_DIR}/src/building/count.c
    ${PROJECT_SOURCE_DIR}/src/building/data_transfer.c
    ${PROJECT_SOURCE_DIR}/src/building/destruction.c
    ${PROJECT_SOURCE_DIR}/src/building/distribution.c
    ${PROJECT_SOURCE_DIR}/src/building/dock.c
    ${PROJECT_SOURCE_DIR}/src/building/figure.c
    ${PROJECT_SOURCE_DIR}/src/building/government.c
    ${PROJECT_SOURCE_DIR}/src/building/granary.c
    ${PROJECT_SOURCE_DIR}/src/building/house.c
    ${PROJECT_SOURCE_DIR}/src/building/house_evolution.c
    ${PROJECT_SOURCE_DIR}/src/building/house_population.c
    ${PROJECT_SOURCE_DIR}/src/building/house_service.c
    ${PROJECT_SOURCE_DIR}/src/building/image.c
    ${PROJECT_SOURCE_DIR}/src/building/industry.c
    ${PROJECT_SOURCE_DIR}/src/building/lighthouse.c
    ${PROJECT_SOURCE_DIR}/src/building/list.c
    ${PROJECT_SOURCE_DIR}/src/building/maintenance.c
    ${PROJECT_SOURCE_DIR}/src/building/market.c
    ${PROJECT_SOURCE_DIR}/src/building/menu.c
    ${PROJECT_SOURCE_DIR}/src/building/mess_hall.c
    ${PROJECT_SOURCE_DIR}/src/building/model.c
    ${PROJECT_SOURCE_DIR}/src/building/monument.c
    ${PROJECT_SOURCE_DIR}/src/building/properties.c
    ${PROJECT_SOURCE_DIR}/src/building/roadblock.c
    ${PROJECT_SOURCE_DIR}/src/building/rotation.c
    ${PROJECT_SOURCE_DIR}/src/building/storage.c
    ${PROJECT_SOURCE_DIR}/src/building/tavern.c
    ${PROJECT_SOURCE_DIR}/src/building/temple.c
    ${PROJECT_SOURCE_DIR}/src/building/warehouse.c
)
set(CITY_FILES
    ${PROJECT_SOURCE_DIR}/src/city/buildings.c
    ${PROJECT_SOURCE_DIR}/src/city/culture.c
    ${PROJECT_SOURCE_DIR}/src/city/data.c
    ${PROJECT_SOURCE_DIR}/src/city/data_private.c
    ${PROJECT_SOURCE_DIR}/src/city/emperor.c
    ${PROJECT_SOURCE_DIR}/src/city/entertainment.c
    ${PROJECT_SOURCE_DIR}/src/city/festival.c
    ${PROJECT_SOURCE_DIR}/src/city/figures.c
    ${PROJECT_SOURCE_DIR}/src/city/finance.c
    ${PROJECT_SOURCE_DIR}/src/city/games.c
    ${PROJECT_SOURCE_DIR}/src/city/gods.c
    ${PROJECT_SOURCE_DIR}/src/city/health.c
    ${PROJECT_SOURCE_DIR}/src/city/houses.c
    ${PROJECT_SOURCE_DIR}/src/city/labor.c
    ${PROJECT_SOURCE_DIR}/src/city/map.c
    ${PROJECT_SOURCE_DIR}/src/city/message.c
    ${PROJECT_SOURCE_DIR}/src/city/migration.c
    ${PROJECT_SOURCE_DIR}/src/city/military.c
    ${PROJECT_SOURCE_DIR}/src/city/mission.c
    ${PROJECT_SOURCE_DIR}/src/city/population.c
    ${PROJECT_SOURCE_DIR}/src/city/race_bet.c
    ${PROJECT_SOURCE_DIR}/src/city/ratings.c
    ${PROJECT_SOURCE_DIR}/src/city/request.c
    ${PROJECT_SOURCE_DIR}/src/city/resource.c
    ${PROJECT_SOURCE_DIR}/src/city/sentiment.c
    ${PROJECT_SOURCE_DIR}/src/city/sound.c
    ${PROJECT_SOURCE_DIR}/src/city/trade.c
    ${PROJECT_SOURCE_DIR}/src/city/trade_policy.c
    ${PROJECT_SOURCE_DIR}/src/city/victory.c
    ${PROJECT_SOURCE_DIR}/src/city/view.c
    ${PROJECT_SOURCE_DIR}/src/city/warning.c
)
set(EMPIRE_FILES
    ${PROJECT_SOURCE_DIR}/src/empire/city.c
    ${PROJECT_SOURCE_DIR}/src/empire/empire.c
    ${PROJECT_SOURCE_DIR}/src/empire/xml.c
    ${PROJECT_SOURCE_DIR}/src/empire/object.c
    ${PROJECT_SOURCE_DIR}/src/empire/trade_prices.c
    ${PROJECT_SOURCE_DIR}/src/empire/trade_route.c
)
set(FIGURE_FILES
    ${PROJECT_SOURCE_DIR}/src/figure/action.c
    ${PROJECT_SOURCE_DIR}/src/figure/combat.c
    ${PROJECT_SOURCE_DIR}/src/figure/enemy_army.c
    ${PROJECT_SOURCE_DIR}/src/figure/figure.c
    ${PROJECT_SOURCE_DIR}/src/figure/formation.c
    ${PROJECT_SOURCE_DIR}/src/figure/formation_enemy.c
    ${PROJECT_SOURCE_DIR}/src/figure/formation_herd.c
    ${PROJECT_SOURCE_DIR}/src/figure/formation_layout.c
    ${PROJECT_SOURCE_DIR}/src/figure/formation_legion.c
    ${PROJECT_SOURCE_DIR}/src/figure/image.c
    ${PROJECT_SOURCE_DIR}/src/figure/movement.c
    ${PROJECT_SOURCE_DIR}/src/figure/name.c
    ${PROJECT_SOURCE_DIR}/src/figure/phrase.c
    ${PROJECT_SOURCE_DIR}/src/figure/properties.c
    ${PROJECT_SOURCE_DIR}/src/figure/route.c
    ${PROJECT_SOURCE_DIR}/src/figure/service.c
    ${PROJECT_SOURCE_DIR}/src/figure/sound.c
    ${PROJECT_SOURCE_DIR}/src/figure/trader.c
)
set(FIGURETYPE_FILES
    ${PROJECT_SOURCE_DIR}/src/figuretype/animal.c
    ${PROJECT_SOURCE_DIR}/src/figuretype/cartpusher.c
    ${PROJECT_SOURCE_DIR}/src/figuretype/crime.c
    ${PROJECT_SOURCE_DIR}/src/figuretype/docker.c
    ${PROJECT_SOURCE_DIR}/src/figuretype/editor.c
    ${PROJECT_SOURCE_DIR}/src/figuretype/enemy.c
    ${PROJECT_SOURCE_DIR}/src/figuretype/entertainer.c
    ${PROJECT_SOURCE_DIR}/src/figuretype/maintenance.c
    ${PROJECT_SOURCE_DIR}/src/figuretype/migrant.c
    ${PROJECT_SOURCE_DIR}/src/figuretype/missile.c
    ${PROJECT_SOURCE_DIR}/src/figuretype/native.c
    ${PROJECT_SOURCE_DIR}/src/figuretype/service.c
    ${PROJECT_SOURCE_DIR}/src/figuretype/soldier.c
    ${PROJECT_SOURCE_DIR}/src/figuretype/supplier.c
    ${PROJECT_SOURCE_DIR}/src/figuretype/trader.c
    ${PROJECT_SOURCE_DIR}/src/figuretype/wall.c
    ${PROJECT_SOURCE_DIR}/src/figuretype/water.c
    ${PROJECT_SOURCE_DIR}/src/figuretype/workcamp.c
)
set(GAME_FILES
    ${PROJECT_SOURCE_DIR}/src/game/animation.c
    ${PROJECT_SOURCE_DIR}/src/game/cheats.c
    ${PROJECT_SOURCE_DIR}/src/game/difficulty.c
    ${PROJECT_SOURCE_DIR}/src/game/file.c
    ${PROJECT_SOURCE_DIR}/src/game/file_editor.c
    ${PROJECT_SOURCE_DIR}/src/game/file_io.c
    ${PROJECT_SOURCE_DIR}/src/game/game.c
    ${PROJECT_SOURCE_DIR}/src/game/mission.c
    ${PROJECT_SOURCE_DIR}/src/game/orientation.c
    ${PROJECT_SOURCE_DIR}/src/game/resource.c
    ${PROJECT_SOURCE_DIR}/src/game/settings.c
    ${PROJECT_SOURCE_DIR}/src/game/speed.c
    ${PROJECT_SOURCE_DIR}/src/game/state.c
    ${PROJECT_SOURCE_DIR}/src/game/tick.c
    ${PROJECT_SOURCE_DIR}/src/game/time.c
    ${PROJECT_SOURCE_DIR}/src/game/tutorial.c
    ${PROJECT_SOURCE_DIR}/src/game/undo.c
)
set(INPUT_FILES
    ${PROJECT_SOURCE_DIR}/src/input/cursor.c
    ${PROJECT_SOURCE_DIR}/src/input/hotkey.c
    ${PROJECT_SOURCE_DIR}/src/input/input.c
    ${PROJECT_SOURCE_DIR}/src/input/joystick.c
    ${PROJECT_SOURCE_DIR}/src/input/keyboard.c
    ${PROJECT_SOURCE_DIR}/src/input/keys.c
    ${PROJECT_SOURCE_DIR}/src/input/mouse.c
    ${PROJECT_SOURCE_DIR}/src/input/scroll.c
    ${PROJECT_SOURCE_DIR}/src/input/touch.c
    ${PROJECT_SOURCE_DIR}/src/input/zoom.c
)
set(MAP_FILES
    ${PROJECT_SOURCE_DIR}/src/map/aqueduct.c
    ${PROJECT_SOURCE_DIR}/src/map/bookmark.c
    ${PROJECT_SOURCE_DIR}/src/map/bridge.c
    ${PROJECT_SOURCE_DIR}/src/map/building.c
    ${PROJECT_SOURCE_DIR}/src/map/building_tiles.c
    ${PROJECT_SOURCE_DIR}/src/map/desirability.c
    ${PROJECT_SOURCE_DIR}/src/map/elevation.c
    ${PROJECT_SOURCE_DIR}/src/map/figure.c
    ${PROJECT_SOURCE_DIR}/src/map/grid.c
    ${PROJECT_SOURCE_DIR}/src/map/image.c
    ${PROJECT_SOURCE_DIR}/src/map/image_context.c
    ${PROJECT_SOURCE_DIR}/src/map/natives.c
    ${PROJECT_SOURCE_DIR}/src/map/orientation.c
    ${PROJECT_SOURCE_DIR}/src/map/point.c
    ${PROJECT_SOURCE_DIR}/src/map/property.c
    ${PROJECT_SOURCE_DIR}/src/map/random.c
    ${PROJECT_SOURCE_DIR}/src/map/ring.c
    ${PROJECT_SOURCE_DIR}/src/map/road_access.c
    ${PROJECT_SOURCE_DIR}/src/map/road_aqueduct.c
    ${PROJECT_SOURCE_DIR}/src/map/road_network.c
    ${PROJECT_SOURCE_DIR}/src/map/routing.c
    ${PROJECT_SOURCE_DIR}/src/map/routing_data.c
    ${PROJECT_SOURCE_DIR}/src/map/routing_path.c
    ${PROJECT_SOURCE_DIR}/src/map/routing_terrain.c
    ${PROJECT_SOURCE_DIR}/src/map/soldier_strength.c
    ${PROJECT_SOURCE_DIR}/src/map/sprite.c
    ${PROJECT_SOURCE_DIR}/src/map/terrain.c
    ${PROJECT_SOURCE_DIR}/src/map/tiles.c
    ${PROJECT_SOURCE_DIR}/src/map/water.c
    ${PROJECT_SOURCE_DIR}/src/map/water_supply.c
)
set(ASSETS_FILES
    ${PROJECT_SOURCE_DIR}/src/assets/group.c
    ${PROJECT_SOURCE_DIR}/src/assets/image.c
    ${PROJECT_SOURCE_DIR}/src/assets/layer.c
    ${PROJECT_SOURCE_DIR}/src/assets/assets.c
    ${PROJECT_SOURCE_DIR}/src/assets/xml.c
)
set(SCENARIO_FILES
    ${PROJECT_SOURCE_DIR}/src/scenario/building.c
    ${PROJECT_SOURCE_DIR}/src/scenario/criteria.c
    ${PROJECT_SOURCE_DIR}/src/scenario/demand_change.c
    ${PROJECT_SOURCE_DIR}/src/scenario/distant_battle.c
    ${PROJECT_SOURCE_DIR}/src/scenario/earthquake.c
    ${PROJECT_SOURCE_DIR}/src/scenario/editor.c
    ${PROJECT_SOURCE_DIR}/src/scenario/editor_events.c
    ${PROJECT_SOURCE_DIR}/src/scenario/editor_map.c
    ${PROJECT_SOURCE_DIR}/src/scenario/emperor_change.c
    ${PROJECT_SOURCE_DIR}/src/scenario/empire.c
    ${PROJECT_SOURCE_DIR}/src/scenario/gladiator_revolt.c
    ${PROJECT_SOURCE_DIR}/src/scenario/invasion.c
    ${PROJECT_SOURCE_DIR}/src/scenario/map.c
    ${PROJECT_SOURCE_DIR}/src/scenario/price_change.c
    ${PROJECT_SOURCE_DIR}/src/scenario/property.c
    ${PROJECT_SOURCE_DIR}/src/scenario/random_event.c
    ${PROJECT_SOURCE_DIR}/src/scenario/request.c
    ${PROJECT_SOURCE_DIR}/src/scenario/scenario.c
)
set(GRAPHICS_FILES
    ${PROJECT_SOURCE_DIR}/src/graphics/arrow_button.c
    ${PROJECT_SOURCE_DIR}/src/graphics/button.c
    ${PROJECT_SOURCE_DIR}/src/graphics/font.c
    ${PROJECT_SOURCE_DIR}/src/graphics/generic_button.c
    ${PROJECT_SOURCE_DIR}/src/graphics/graphics.c
    ${PROJECT_SOURCE_DIR}/src/graphics/image.c
    ${PROJECT_SOURCE_DIR}/src/graphics/image_button.c
    ${PROJECT_SOURCE_DIR}/src/graphics/lang_text.c
    ${PROJECT_SOURCE_DIR}/src/graphics/menu.c
    ${PROJECT_SOURCE_DIR}/src/graphics/panel.c
    ${PROJECT_SOURCE_DIR}/src/graphics/rich_text.c
    ${PROJECT_SOURCE_DIR}/src/graphics/renderer.c
    ${PROJECT_SOURCE_DIR}/src/graphics/screen.c
    ${PROJECT_SOURCE_DIR}/src/graphics/screenshot.c
    ${PROJECT_SOURCE_DIR}/src/graphics/scrollbar.c
    ${PROJECT_SOURCE_DIR}/src/graphics/text.c
    ${PROJECT_SOURCE_DIR}/src/graphics/tooltip.c
    ${PROJECT_SOURCE_DIR}/src/graphics/video.c
    ${PROJECT_SOURCE_DIR}/src/graphics/warning.c
    ${PROJECT_SOURCE_DIR}/src/graphics/window.c
)
set(SOUND_FILES
    ${PROJECT_SOURCE_DIR}/src/sound/city.c
    ${PROJECT_SOURCE_DIR}/src/sound/effect.c
    ${PROJECT_SOURCE_DIR}/src/sound/music.c
    ${PROJECT_SOURCE_DIR}/src/sound/speech.c
    ${PROJECT_SOURCE_DIR}/src/sound/system.c
)
set(WIDGET_FILES
    ${PROJECT_SOURCE_DIR}/src/widget/city.c
    ${PROJECT_SOURCE_DIR}/src/widget/city_bridge.c
    ${PROJECT_SOURCE_DIR}/src/widget/city_building_ghost.c
    ${PROJECT_SOURCE_DIR}/src/widget/city_figure.c
    ${PROJECT_SOURCE_DIR}/src/widget/city_draw_highway.c
    ${PROJECT_SOURCE_DIR}/src/widget/city_overlay_education.c
    ${PROJECT_SOURCE_DIR}/src/widget/city_overlay_entertainment.c
    ${PROJECT_SOURCE_DIR}/src/widget/city_overlay_health.c
    ${PROJECT_SOURCE_DIR}/src/widget/city_overlay_other.c
    ${PROJECT_SOURCE_DIR}/src/widget/city_overlay_risks.c
    ${PROJECT_SOURCE_DIR}/src/widget/city_pause_menu.c
    ${PROJECT_SOURCE_DIR}/src/widget/city_water_ghost.c
    ${PROJECT_SOURCE_DIR}/src/widget/city_with_overlay.c
    ${PROJECT_SOURCE_DIR}/src/widget/city_without_overlay.c
    ${PROJECT_SOURCE_DIR}/src/widget/input_box.c
    ${PROJECT_SOURCE_DIR}/src/widget/map_editor.c
    ${PROJECT_SOURCE_DIR}/src/widget/map_editor_tool.c
    ${PROJECT_SOURCE_DIR}/src/widget/map_editor_pause_menu.c
    ${PROJECT_SOURCE_DIR}/src/widget/minimap.c
    ${PROJECT_SOURCE_DIR}/src/widget/top_menu.c
    ${PROJECT_SOURCE_DIR}/src/widget/top_menu_editor.c
    ${PROJECT_SOURCE_DIR}/src/widget/sidebar/city.c
    ${PROJECT_SOURCE_DIR}/src/widget/sidebar/common.c
    ${PROJECT_SOURCE_DIR}/src/widget/sidebar/editor.c
    ${PROJECT_SOURCE_DIR}/src/widget/sidebar/extra.c
    ${PROJECT_SOURCE_DIR}/src/widget/sidebar/military.c
    ${PROJECT_SOURCE_DIR}/src/widget/sidebar/slide.c
)
set(WINDOW_FILES
    ${PROJECT_SOURCE_DIR}/src/window/advisors.c
    ${PROJECT_SOURCE_DIR}/src/window/advisor/chief.c
    ${PROJECT_SOURCE_DIR}/src/window/advisor/education.c
    ${PROJECT_SOURCE_DIR}/src/window/advisor/entertainment.c
    ${PROJECT_SOURCE_DIR}/src/window/advisor/financial.c
    ${PROJECT_SOURCE_DIR}/src/window/advisor/health.c
    ${PROJECT_SOURCE_DIR}/src/window/advisor/housing.c
    ${PROJECT_SOURCE_DIR}/src/window/advisor/imperial.c
    ${PROJECT_SOURCE_DIR}/src/window/advisor/labor.c
    ${PROJECT_SOURCE_DIR}/src/window/advisor/military.c
    ${PROJECT_SOURCE_DIR}/src/window/advisor/population.c
    ${PROJECT_SOURCE_DIR}/src/window/advisor/ratings.c
    ${PROJECT_SOURCE_DIR}/src/window/advisor/religion.c
    ${PROJECT_SOURCE_DIR}/src/window/advisor/trade.c
    ${PROJECT_SOURCE_DIR}/src/window/asset_previewer.c
    ${PROJECT_SOURCE_DIR}/src/window/build_menu.c
    ${PROJECT_SOURCE_DIR}/src/window/building_info.c
    ${PROJECT_SOURCE_DIR}/src/window/building/common.c
    ${PROJECT_SOURCE_DIR}/src/window/building/culture.c
    ${PROJECT_SOURCE_DIR}/src/window/building/distribution.c
    ${PROJECT_SOURCE_DIR}/src/window/building/figures.c
    ${PROJECT_SOURCE_DIR}/src/window/building/government.c
    ${PROJECT_SOURCE_DIR}/src/window/building/house.c
    ${PROJECT_SOURCE_DIR}/src/window/building/industry.c
    ${PROJECT_SOURCE_DIR}/src/window/building/military.c
    ${PROJECT_SOURCE_DIR}/src/window/building/terrain.c
    ${PROJECT_SOURCE_DIR}/src/window/building/utility.c
    ${PROJECT_SOURCE_DIR}/src/window/editor/allowed_buildings.c
    ${PROJECT_SOURCE_DIR}/src/window/editor/attributes.c
    ${PROJECT_SOURCE_DIR}/src/window/editor/build_menu.c
    ${PROJECT_SOURCE_DIR}/src/window/editor/demand_changes.c
    ${PROJECT_SOURCE_DIR}/src/window/editor/edit_demand_change.c
    ${PROJECT_SOURCE_DIR}/src/window/editor/edit_invasion.c
    ${PROJECT_SOURCE_DIR}/src/window/editor/edit_price_change.c
    ${PROJECT_SOURCE_DIR}/src/window/editor/edit_request.c
    ${PROJECT_SOURCE_DIR}/src/window/editor/empire.c
    ${PROJECT_SOURCE_DIR}/src/window/editor/invasions.c
    ${PROJECT_SOURCE_DIR}/src/window/editor/map.c
    ${PROJECT_SOURCE_DIR}/src/window/editor/price_changes.c
    ${PROJECT_SOURCE_DIR}/src/window/editor/requests.c
    ${PROJECT_SOURCE_DIR}/src/window/editor/special_events.c
    ${PROJECT_SOURCE_DIR}/src/window/editor/start_year.c
    ${PROJECT_SOURCE_DIR}/src/window/editor/starting_conditions.c
    ${PROJECT_SOURCE_DIR}/src/window/editor/win_criteria.c
    ${PROJECT_SOURCE_DIR}/src/window/cck_selection.c
    ${PROJECT_SOURCE_DIR}/src/window/city.c
    ${PROJECT_SOURCE_DIR}/src/window/config.c
    ${PROJECT_SOURCE_DIR}/src/window/console.c
    ${PROJECT_SOURCE_DIR}/src/window/donate_to_city.c
    ${PROJECT_SOURCE_DIR}/src/window/empire.c
    ${PROJECT_SOURCE_DIR}/src/window/file_dialog.c
    ${PROJECT_SOURCE_DIR}/src/window/gift_to_emperor.c
    ${PROJECT_SOURCE_DIR}/src/window/hold_games.c
    ${PROJECT_SOURCE_DIR}/src/window/hold_festival.c
    ${PROJECT_SOURCE_DIR}/src/window/hotkey_config.c
    ${PROJECT_SOURCE_DIR}/src/window/hotkey_editor.c
    ${PROJECT_SOURCE_DIR}/src/window/intermezzo.c
    ${PROJECT_SOURCE_DIR}/src/window/intro_video.c
    ${PROJECT_SOURCE_DIR}/src/window/labor_priority.c
    ${PROJECT_SOURCE_DIR}/src/window/logo.c
    ${PROJECT_SOURCE_DIR}/src/window/main_menu.c
    ${PROJECT_SOURCE_DIR}/src/window/message_dialog.c
    ${PROJECT_SOURCE_DIR}/src/window/message_list.c
    ${PROJECT_SOURCE_DIR}/src/window/military_menu.c
    ${PROJECT_SOURCE_DIR}/src/window/mission_briefing.c
    ${PROJECT_SOURCE_DIR}/src/window/mission_end.c
    ${PROJECT_SOURCE_DIR}/src/window/mission_selection.c
    ${PROJECT_SOURCE_DIR}/src/window/new_career.c
    ${PROJECT_SOURCE_DIR}/src/window/numeric_input.c
    ${PROJECT_SOURCE_DIR}/src/window/option_popup.c
    ${PROJECT_SOURCE_DIR}/src/window/overlay_menu.c
    ${PROJECT_SOURCE_DIR}/src/window/plain_message_dialog.c
    ${PROJECT_SOURCE_DIR}/src/window/popup_dialog.c
    ${PROJECT_SOURCE_DIR}/src/window/race_bet.c
    ${PROJECT_SOURCE_DIR}/src/window/resource_settings.c
    ${PROJECT_SOURCE_DIR}/src/window/select_list.c
    ${PROJECT_SOURCE_DIR}/src/window/set_salary.c
    ${PROJECT_SOURCE_DIR}/src/window/trade_opened.c
    ${PROJECT_SOURCE_DIR}/src/window/trade_prices.c
    ${PROJECT_SOURCE_DIR}/src/window/victory_dialog.c
    ${PROJECT_SOURCE_DIR}/src/window/victory_video.c
)
set(EDITOR_FILES
    ${PROJECT_SOURCE_DIR}/src/editor/editor.c
    ${PROJECT_SOURCE_DIR}/src/editor/tool.c
    ${PROJECT_SOURCE_DIR}/src/editor/tool_restriction.c
)
set(TRANSLATION_FILES
    ${PROJECT_SOURCE_DIR}/src/translation/english.c
    ${PROJECT_SOURCE_DIR}/src/translation/french.c
    ${PROJECT_SOURCE_DIR}/src/translation/german.c
    ${PROJECT_SOURCE_DIR}/src/translation/italian.c
    ${PROJECT_SOURCE_DIR}/src/translation/japanese.c
    ${PROJECT_SOURCE_DIR}/src/translation/korean.c
    ${PROJECT_SOURCE_DIR}/src/translation/polish.c
    ${PROJECT_SOURCE_DIR}/src/translation/portuguese.c
    ${PROJECT_SOURCE_DIR}/src/translation/russian.c
    ${PROJECT_SOURCE_DIR}/src/translation/simplified_chinese.c
    ${PROJECT_SOURCE_DIR}/src/translation/spanish.c
    ${PROJECT_SOURCE_DIR}/src/translation/swedish.c
    ${PROJECT_SOURCE_DIR}/src/translation/traditional_chinese.c
    ${PROJECT_SOURCE_DIR}/src/translation/translation.c
)

set(MACOSX_FILES "")
if(APPLE)
    function(resource VAR SOURCE_PATH DESTINATION PATTERN)
    file(GLOB_RECURSE _LIST CONFIGURE_DEPENDS ${SOURCE_PATH}/${PATTERN})
    foreach (RESOURCE ${_LIST})
        get_filename_component(_PARENT ${RESOURCE} DIRECTORY)
        if (${_PARENT} STREQUAL ${SOURCE_PATH})
            set(_DESTINATION ${DESTINATION})
        else ()
            file(RELATIVE_PATH _DESTINATION ${SOURCE_PATH} ${_PARENT})
            set(_DESTINATION ${DESTINATION}/${_DESTINATION})
        endif ()
        set_property(SOURCE ${RESOURCE} PROPERTY MACOSX_PACKAGE_LOCATION ${_DESTINATION})
    endforeach (RESOURCE)
    set(${VAR} ${_LIST} PARENT_SCOPE)
    endfunction()
    resource(ASSET_SOURCES ${ASSETS_DIR} Resources/assets *)
    set(MACOSX_FILES
        ${PROJECT_SOURCE_DIR}/res/augustus.icns
        ${ASSET_SOURCES}
    )
endif()

set(EMSCRIPTEN_FILES "")
if(${TARGET_PLATFORM} STREQUAL "emscripten")
    set(EMSCRIPTEN_FILES ${PROJECT_SOURCE_DIR}/res/shell.html)
endif()

set(SOURCE_FILES
    ${PLATFORM_FILES}
    ${CORE_FILES}
    ${BUILDING_FILES}
    ${CITY_FILES}
    ${EMPIRE_FILES}
    ${FIGURE_FILES}
    ${FIGURETYPE_FILES}
    ${GAME_FILES}
    ${INPUT_FILES}
    ${MAP_FILES}
    ${ASSETS_FILES}
    ${SCENARIO_FILES}
    ${GRAPHICS_FILES}
    ${SOUND_FILES}
    ${WIDGET_FILES}
    ${WINDOW_FILES}
    ${EDITOR_FILES}
    ${TRANSLATION_FILES}
    ${TINYFD_FILES}
    ${PROJECT_SOURCE_DIR}/res/augustus.rc
    ${MACOSX_FILES}
)

if(${TARGET_PLATFORM} STREQUAL "emscripten" AND EMSCRIPTEN_LOAD_SDL_PORTS)
    set(USE_FLAGS "-s USE_SDL=2 -s USE_SDL_MIXER=2 -s SDL2_MIXER_FORMATS=[\"mp3\"] -s USE_MPG123=1")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")
else()
    function(GET_SDL_EXT_DIR result module)
        if(NOT module STREQUAL "")
            set(module "_${module}")
        endif()
        set(SDL_LOCATION ${PROJECT_SOURCE_DIR}/ext/SDL2)
        file(GLOB children
            RELATIVE ${SDL_LOCATION}
            CONFIGURE_DEPENDS
            ${SDL_LOCATION}/SDL${module}
            ${SDL_LOCATION}/SDL2${module}
            ${SDL_LOCATION}/SDL${module}-*
            ${SDL_LOCATION}/SDL2${module}-*
        )
        foreach(child ${children})
            if(IS_DIRECTORY "${SDL_LOCATION}/${child}")
                set(${result} "${SDL_LOCATION}/${child}" PARENT_SCOPE)
                break()
            endif()
        endforeach()
    endfunction()

    find_package(SDL2 REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    if(${TARGET_PLATFORM} STREQUAL "emscripten")
        set(LINK_MPG123 true)
    endif()
endif()

if(LINK_MPG123)
    find_package(MPG123 REQUIRED)
endif()

if(${TARGET_PLATFORM} STREQUAL "android")
    add_library(${SHORT_NAME} SHARED ${SDL2_ANDROID_HOOK} ${SOURCE_FILES})
else()
    add_executable(${SHORT_NAME} WIN32 ${SOURCE_FILES})
endif()

if(${TARGET_PLATFORM} STREQUAL "emscripten")
    set(CMAKE_EXECUTABLE_SUFFIX .html)
    set_target_properties(${SHORT_NAME} PROPERTIES LINK_FLAGS "-lidbfs.js -s SINGLE_FILE=1 -s DEMANGLE_SUPPORT=1 -s DYNCALLS=1 --shell-file ${PROJECT_SOURCE_DIR}/res/shell.html --embed-file ${ASSETS_DIR}@/assets -s INITIAL_MEMORY=268435456 -s ALLOW_MEMORY_GROWTH=1 -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0 -s SAFE_HEAP=1 -s INVOKE_RUN=0 -s EXPORTED_FUNCTIONS=[\"_main\"] -s EXPORTED_RUNTIME_METHODS=[\"callMain\",\"FS\"] -s DEFAULT_LIBRARY_FUNCS_TO_INCLUDE=[\"$autoResumeAudioContext\"] --bind")
endif()

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

if(CMAKE_INSTALL_DATADIR)
    add_compile_definitions(CUSTOM_ASSETS_DIR="${CMAKE_INSTALL_DATADIR}")
endif()

if(APPLE)
    # generating a macOS icns file (see https://stackoverflow.com/a/20703594)
    add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/res/augustus.icns
        COMMAND mkdir -p augustus.iconset
        COMMAND sips -z 16 16    augustus_256.png --out augustus.iconset/icon_16x16.png
        COMMAND sips -z 32 32    augustus_256.png --out augustus.iconset/icon_16x16@2x.png
        COMMAND sips -z 32 32    augustus_256.png --out augustus.iconset/icon_32x32.png
        COMMAND sips -z 64 64    augustus_256.png --out augustus.iconset/icon_32x32@2x.png
        COMMAND sips -z 128 128  augustus_256.png --out augustus.iconset/icon_128x128.png
        COMMAND sips -z 256 256  augustus_256.png --out augustus.iconset/icon_128x128@2x.png
        COMMAND sips -z 256 256  augustus_256.png --out augustus.iconset/icon_256x256.png
        COMMAND sips -z 512 512  augustus_512.png --out augustus.iconset/icon_256x256@2x.png
        COMMAND iconutil -c icns augustus.iconset
        COMMAND rm -R augustus.iconset
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/res)
    set_source_files_properties(${PROJECT_SOURCE_DIR}/res/augustus.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources")

    # setting variables that will populate Info.plist
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "pl.Keriew.augustus")
    set(MACOSX_BUNDLE_BUNDLE_NAME ${USER_FRIENDLY_NAME})
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10" CACHE STRING "Minimum OS X deployment version" FORCE)
    set(MACOSX_BUNDLE_ICON_FILE "augustus.icns")
    set(MACOSX_BUNDLE_BUNDLE_VERSION
        "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}${VERSION_REVISION}")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${MACOSX_BUNDLE_BUNDLE_VERSION})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${MACOSX_BUNDLE_BUNDLE_VERSION})

    set_target_properties(${SHORT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/res/Info.plist")

    set(DIRS "")
    set(LIBS "")

    # if SDL2 library is a framework, we need to indicate to CMake
    # the path to its dependencies. SDL2_LIBRARY contains two parts.
    # Example: /Library/Frameworks/SDL2.framework;-framework Cocoa
    # So strip out everything after the ';'
    string(REGEX REPLACE ";.*$" "" SDL2_LIB_DIR "${SDL2_LIBRARY}")
    if(EXISTS "${SDL2_LIB_DIR}/Versions/A/Frameworks")
        list(APPEND DIRS "${SDL2_LIB_DIR}/Versions/A/Frameworks")
    endif()

    # if SDL2_mixer library is a framework, we need to indicate to CMake
    # the path to its dependencies (Ogg.framework etc):
    if(EXISTS "${SDL2_MIXER_LIBRARY}/Versions/A/Frameworks")
        list(APPEND DIRS "${SDL2_MIXER_LIBRARY}/Versions/A/Frameworks")
    endif()

    # when installing, "fixup" automatically copies libraries inside the
    # bundle and links the binary against them
    install(CODE "
        include(BundleUtilities)
        fixup_bundle(${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app \"${LIBS}\" \"${DIRS}\")
    " BUNDLE DESTINATION ${CMAKE_BINARY_DIR})
endif()

if(SDL2_INCLUDE_DIR)
    include_directories(${SDL2_INCLUDE_DIR})
endif()
if(SDL2_MIXER_INCLUDE_DIR)
    include_directories(${SDL2_MIXER_INCLUDE_DIR})
endif()

if(SYSTEM_LIBS)
    find_package(ZLIB)
    find_package(PNG)
    find_package(EXPAT)
endif()

if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    target_link_libraries(${SHORT_NAME} ${ZLIB_LIBRARIES})
else()
    include_directories("ext/zlib")
    target_sources(${SHORT_NAME} PRIVATE "${ZLIB_FILES}")
endif()

if(PNG_FOUND)
    include_directories(${PNG_INCLUDE_DIRS})
    target_link_libraries(${SHORT_NAME} ${PNG_LIBRARIES})
else()
    if(SYSTEM_LIBS)
        message(STATUS "PNG was not found but that's ok: falling back to internal version")
    endif()
    include_directories("ext/png")
    target_sources(${SHORT_NAME} PRIVATE "${PNG_FILES}")
endif()

if(EXPAT_FOUND)
    include_directories(${EXPAT_INCLUDE_DIRS})
    target_link_libraries(${SHORT_NAME} ${EXPAT_LIBRARIES})
else()
    include_directories("ext/expat")
    target_sources(${SHORT_NAME} PRIVATE "${EXPAT_FILES}")
    if(NOT WIN32)
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DXML_DEV_URANDOM")
    endif()
endif()

if(${TARGET_PLATFORM} STREQUAL "vita")
    include_directories(
        $ENV{VITASDK}/arm-vita-eabi/include
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
elseif (${TARGET_PLATFORM} STREQUAL "switch")
    include_directories(
        ${DEVKITPRO}/portlibs/switch/include/SDL2
        ${DEVKITPRO}/libnx/include
        ${DEVKITPRO}/portlibs/switch/include
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

include_directories(ext)
include_directories(src)
if(MSVC)
    include_directories(ext/dirent)
endif()

if(${TARGET_PLATFORM} STREQUAL "vita")
    target_link_libraries (${SHORT_NAME}
        ${SDL2_MIXER_LIBRARY}
        ${SDL2_LIBRARY}
        vorbisfile
        vorbis
        mikmod
        mpg123
        FLAC
        ogg
        m
        SceAppUtil_stub
        SceAudio_stub
        SceAudioIn_stub
        SceCommonDialog_stub
        SceCtrl_stub
        SceDisplay_stub
        SceGxm_stub
        SceHid_stub
        SceMotion_stub
        SceIofilemgr_stub
        SceMotion_stub
        SceSysmodule_stub
        SceTouch_stub
        ScePower_stub
        ScePgf_stub
        SceAppMgr_stub
    )
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__vita__ -DPNG_ARM_NEON_OPT=0")
    # this setting enables larger heap memory sizes on Vita, up to ~330 MB
    # useful for pre-loading videos into memory
    set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d ATTRIBUTE2=12")
    vita_create_self(${SHORT_NAME}.self ${SHORT_NAME} UNSAFE UNCOMPRESSED)
    vita_create_vpk(${SHORT_NAME}.vpk ${VITA_TITLEID} ${SHORT_NAME}.self
        VERSION ${VITA_VERSION}
        NAME ${SHORT_NAME}
        FILE res/vita/icon0.png sce_sys/icon0.png
        FILE res/vita/bg.png sce_sys/livearea/contents/bg.png
        FILE res/vita/startup.png sce_sys/livearea/contents/startup.png
        FILE res/vita/template.xml sce_sys/livearea/contents/template.xml
        FILE ${ASSETS_DIR} assets
    )
elseif (${TARGET_PLATFORM} STREQUAL "switch")
    target_link_libraries (${SHORT_NAME}
        SDL2_mixer
        SDL2
        EGL
        GLESv2
        glapi
        drm_nouveau
        vorbisidec
        modplug
        mpg123
        FLAC
        opusfile
        opus
        ogg
        m
        z
        nx
        stdc++
    )

    set(FLAGS
         -DPNG_ARM_NEON_OPT=0
        -fPIE
        -L${DEVKITPRO}/portlibs/switch/lib
        -L${DEVKITPRO}/libnx/lib
    )

    foreach(flag ${FLAGS})
        set(FLAGS_GENERAL "${FLAGS_GENERAL} ${flag}")
    endforeach(flag ${FLAGS})

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FLAGS_GENERAL}")

    target_compile_options(${SHORT_NAME} PRIVATE ${FLAGS})
    add_custom_command(TARGET ${SHORT_NAME}
        POST_BUILD
        COMMAND ${DEVKITPRO}/tools/bin/nacptool --create "${SHORT_NAME}" "rsn8887" ${SWITCH_VERSION} ${SHORT_NAME}.nacp
        COMMAND ${DEVKITPRO}/tools/bin/elf2nro ${SHORT_NAME} ${SHORT_NAME}.nro --icon=${CMAKE_SOURCE_DIR}/res/switch/icon0.jpg --nacp=${SHORT_NAME}.nacp --romfsdir="${ASSETS_DIR}"
    )
else()
    if (UNIX AND NOT APPLE AND (CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang"))
        target_link_libraries(${SHORT_NAME} m)
    endif()
    if(${TARGET_PLATFORM} STREQUAL "android")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPNG_ARM_NEON_OPT=0 -D_BSD_SOURCE")
        find_library(log-lib log)
        find_library(android-lib android)
        target_link_libraries(${SHORT_NAME} ${log-lib} ${android-lib})
    endif()
    if(LINK_MPG123)
        target_link_libraries(${SHORT_NAME} ${MPG123_LIBRARY})
    endif()
    target_link_libraries (${SHORT_NAME} ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY})
    if(NOT APPLE AND NOT ${TARGET_PLATFORM} STREQUAL "android")
        install(TARGETS ${SHORT_NAME} RUNTIME DESTINATION bin)
    endif()

    if(WIN32)
       target_link_libraries(${SHORT_NAME} dbghelp shlwapi)
    endif()

    if(UNIX AND NOT APPLE)
        install(FILES "res/augustus.desktop" DESTINATION "share/applications" RENAME "com.github.keriew.augustus.desktop")
        install(FILES "res/augustus.metainfo.xml" DESTINATION "share/metainfo" RENAME "com.github.keriew.augustus.metainfo.xml")
        install(FILES "res/augustus_32.png" DESTINATION "share/icons/hicolor/32x32/apps" RENAME "com.github.keriew.augustus.png")
        install(FILES "res/augustus_48.png" DESTINATION "share/icons/hicolor/48x48/apps" RENAME "com.github.keriew.augustus.png")
        install(FILES "res/augustus_256.png" DESTINATION "share/icons/hicolor/256x256/apps" RENAME "com.github.keriew.augustus.png")
        install(FILES "res/augustus_512.png" DESTINATION "share/icons/hicolor/512x512/apps" RENAME "com.github.keriew.augustus.png")
        install(DIRECTORY "${ASSETS_DIR}/" DESTINATION "share/augustus-game/assets")
    endif()

endif()
