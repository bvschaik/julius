cmake_minimum_required(VERSION 2.6)
project(julius)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/")

if (CMAKE_COMPILER_IS_GNUCC)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror=implicit-function-declaration")

    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER 4.9.0)
        set (CMAKE_C_FLAGS "-Werror=incompatible-pointer-types -Werror=int-conversion ${CMAKE_C_FLAGS}")
    else ()
        set (CMAKE_C_STANDARD 99) # -std=gnu99
        message(WARNING "WARNING: Old version GCC. Continue...")
    endif ()
endif ()

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
set(SMK_FILES src/Video/smacker.c src/Video/smk_bitstream.c src/Video/smk_hufftree.c)
set(CORE_FILES
    ${PROJECT_SOURCE_DIR}/src/core/buffer.c
    ${PROJECT_SOURCE_DIR}/src/core/calc.c
    ${PROJECT_SOURCE_DIR}/src/core/debug.c
    ${PROJECT_SOURCE_DIR}/src/core/dir.c
    ${PROJECT_SOURCE_DIR}/src/core/file.c
    ${PROJECT_SOURCE_DIR}/src/core/io.c
    ${PROJECT_SOURCE_DIR}/src/core/lang.c
    ${PROJECT_SOURCE_DIR}/src/core/random.c
    ${PROJECT_SOURCE_DIR}/src/core/string.c
    ${PROJECT_SOURCE_DIR}/src/core/time.c
    ${PROJECT_SOURCE_DIR}/src/core/zip.c
)
set(BUILDING_FILES
    ${PROJECT_SOURCE_DIR}/src/building/building.c
    ${PROJECT_SOURCE_DIR}/src/building/construction_warning.c
    ${PROJECT_SOURCE_DIR}/src/building/count.c
    ${PROJECT_SOURCE_DIR}/src/building/figure.c
    ${PROJECT_SOURCE_DIR}/src/building/house.c
    ${PROJECT_SOURCE_DIR}/src/building/industry.c
    ${PROJECT_SOURCE_DIR}/src/building/list.c
    ${PROJECT_SOURCE_DIR}/src/building/maintenance.c
    ${PROJECT_SOURCE_DIR}/src/building/market.c
    ${PROJECT_SOURCE_DIR}/src/building/model.c
    ${PROJECT_SOURCE_DIR}/src/building/properties.c
    ${PROJECT_SOURCE_DIR}/src/building/storage.c
)
set(CITY_FILES
    ${PROJECT_SOURCE_DIR}/src/city/culture.c
    ${PROJECT_SOURCE_DIR}/src/city/finance.c
    ${PROJECT_SOURCE_DIR}/src/city/message.c
    ${PROJECT_SOURCE_DIR}/src/city/warning.c
)
set(EMPIRE_FILES
    ${PROJECT_SOURCE_DIR}/src/empire/city.c
    ${PROJECT_SOURCE_DIR}/src/empire/empire.c
    ${PROJECT_SOURCE_DIR}/src/empire/object.c
    ${PROJECT_SOURCE_DIR}/src/empire/trade_prices.c
    ${PROJECT_SOURCE_DIR}/src/empire/trade_route.c
)
set(FIGURE_FILES
    ${PROJECT_SOURCE_DIR}/src/figure/enemy_army.c
    ${PROJECT_SOURCE_DIR}/src/figure/figure.c
    ${PROJECT_SOURCE_DIR}/src/figure/formation.c
    ${PROJECT_SOURCE_DIR}/src/figure/name.c
    ${PROJECT_SOURCE_DIR}/src/figure/phrase.c
    ${PROJECT_SOURCE_DIR}/src/figure/properties.c
    ${PROJECT_SOURCE_DIR}/src/figure/route.c
    ${PROJECT_SOURCE_DIR}/src/figure/sound.c
    ${PROJECT_SOURCE_DIR}/src/figure/trader.c
)
set(GAME_FILES
    ${PROJECT_SOURCE_DIR}/src/game/difficulty.c
    ${PROJECT_SOURCE_DIR}/src/game/settings.c
    ${PROJECT_SOURCE_DIR}/src/game/time.c
    ${PROJECT_SOURCE_DIR}/src/game/tutorial.c
)
set(INPUT_FILES
    ${PROJECT_SOURCE_DIR}/src/input/keyboard.c
    ${PROJECT_SOURCE_DIR}/src/input/mouse.c
    ${PROJECT_SOURCE_DIR}/src/input/scroll.c
)
set(MAP_FILES
    ${PROJECT_SOURCE_DIR}/src/map/aqueduct.c
    ${PROJECT_SOURCE_DIR}/src/map/bookmark.c
    ${PROJECT_SOURCE_DIR}/src/map/bridge.c
    ${PROJECT_SOURCE_DIR}/src/map/building.c
    ${PROJECT_SOURCE_DIR}/src/map/desirability.c
    ${PROJECT_SOURCE_DIR}/src/map/elevation.c
    ${PROJECT_SOURCE_DIR}/src/map/figure.c
    ${PROJECT_SOURCE_DIR}/src/map/grid.c
    ${PROJECT_SOURCE_DIR}/src/map/image.c
    ${PROJECT_SOURCE_DIR}/src/map/property.c
    ${PROJECT_SOURCE_DIR}/src/map/random.c
    ${PROJECT_SOURCE_DIR}/src/map/ring.c
    ${PROJECT_SOURCE_DIR}/src/map/road_aqueduct.c
    ${PROJECT_SOURCE_DIR}/src/map/road_network.c
    ${PROJECT_SOURCE_DIR}/src/map/routing.c
    ${PROJECT_SOURCE_DIR}/src/map/routing_data.c
    ${PROJECT_SOURCE_DIR}/src/map/routing_path.c
    ${PROJECT_SOURCE_DIR}/src/map/routing_terrain.c
    ${PROJECT_SOURCE_DIR}/src/map/soldier_strength.c
    ${PROJECT_SOURCE_DIR}/src/map/sprite.c
    ${PROJECT_SOURCE_DIR}/src/map/terrain.c
    ${PROJECT_SOURCE_DIR}/src/map/water_supply.c
)
set(SCENARIO_FILES
    ${PROJECT_SOURCE_DIR}/src/scenario/building.c
    ${PROJECT_SOURCE_DIR}/src/scenario/criteria.c
    ${PROJECT_SOURCE_DIR}/src/scenario/demand_change.c
    ${PROJECT_SOURCE_DIR}/src/scenario/distant_battle.c
    ${PROJECT_SOURCE_DIR}/src/scenario/earthquake.c
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
    ${PROJECT_SOURCE_DIR}/src/graphics/image.c
)
set(SOUND_FILES
    ${PROJECT_SOURCE_DIR}/src/sound/city.c
    ${PROJECT_SOURCE_DIR}/src/sound/effect.c
    ${PROJECT_SOURCE_DIR}/src/sound/music.c
    ${PROJECT_SOURCE_DIR}/src/sound/speech.c
    ${PROJECT_SOURCE_DIR}/src/sound/system.c
)
add_executable(julius
	${PROJECT_SOURCE_DIR}/linux/main2.c
	${PROJECT_SOURCE_DIR}/linux/SDLSoundDevice.c
	${PROJECT_SOURCE_DIR}/src/Animation.c
	${PROJECT_SOURCE_DIR}/src/Building.c
	${PROJECT_SOURCE_DIR}/src/BuildingPlacement.c
	${PROJECT_SOURCE_DIR}/src/CityInfo.c
	${PROJECT_SOURCE_DIR}/src/CityInfo_Culture.c
	${PROJECT_SOURCE_DIR}/src/CityInfo_Finance.c
	${PROJECT_SOURCE_DIR}/src/CityInfo_Gods.c
	${PROJECT_SOURCE_DIR}/src/CityInfo_Labor.c
	${PROJECT_SOURCE_DIR}/src/CityInfo_Population.c
	${PROJECT_SOURCE_DIR}/src/CityInfo_Ratings.c
	${PROJECT_SOURCE_DIR}/src/CityInfo_Resource.c
	${PROJECT_SOURCE_DIR}/src/CityInfo_Tick.c
	${PROJECT_SOURCE_DIR}/src/CityInfo_Victory.c
	${PROJECT_SOURCE_DIR}/src/CityView.c
	${PROJECT_SOURCE_DIR}/src/Cursor.c
	${PROJECT_SOURCE_DIR}/src/Figure.c
	${PROJECT_SOURCE_DIR}/src/FigureAction.c
	${PROJECT_SOURCE_DIR}/src/FigureAction_Animal.c
	${PROJECT_SOURCE_DIR}/src/FigureAction_Cartpusher.c
	${PROJECT_SOURCE_DIR}/src/FigureAction_Combat.c
	${PROJECT_SOURCE_DIR}/src/FigureAction_Common.c
	${PROJECT_SOURCE_DIR}/src/FigureAction_Culture.c
	${PROJECT_SOURCE_DIR}/src/FigureAction_Docker.c
	${PROJECT_SOURCE_DIR}/src/FigureAction_Enemy.c
	${PROJECT_SOURCE_DIR}/src/FigureAction_Entertainer.c
	${PROJECT_SOURCE_DIR}/src/FigureAction_Market.c
	${PROJECT_SOURCE_DIR}/src/FigureAction_Migrant.c
	${PROJECT_SOURCE_DIR}/src/FigureAction_Missile.c
	${PROJECT_SOURCE_DIR}/src/FigureAction_Native.c
	${PROJECT_SOURCE_DIR}/src/FigureAction_Rioter.c
	${PROJECT_SOURCE_DIR}/src/FigureAction_Service.c
	${PROJECT_SOURCE_DIR}/src/FigureAction_Soldier.c
	${PROJECT_SOURCE_DIR}/src/FigureAction_Trader.c
	${PROJECT_SOURCE_DIR}/src/FigureAction_Wall.c
	${PROJECT_SOURCE_DIR}/src/FigureAction_Water.c
	${PROJECT_SOURCE_DIR}/src/FigureMovement.c
	${PROJECT_SOURCE_DIR}/src/Figure_Service.c
	${PROJECT_SOURCE_DIR}/src/Formation.c
	${PROJECT_SOURCE_DIR}/src/FormationUpdate.c
	${PROJECT_SOURCE_DIR}/src/Game.c
	${PROJECT_SOURCE_DIR}/src/GameFile.c
	${PROJECT_SOURCE_DIR}/src/GameTick.c
	${PROJECT_SOURCE_DIR}/src/Graphics.c
	${PROJECT_SOURCE_DIR}/src/Graphics_Footprint.c
	${PROJECT_SOURCE_DIR}/src/HouseEvolution.c
	${PROJECT_SOURCE_DIR}/src/HousePopulation.c
	${PROJECT_SOURCE_DIR}/src/KeyboardHotkey.c
	${PROJECT_SOURCE_DIR}/src/Loader.c
	${PROJECT_SOURCE_DIR}/src/Natives.c
	${PROJECT_SOURCE_DIR}/src/Resource.c
	${PROJECT_SOURCE_DIR}/src/Resource_Granary.c
	${PROJECT_SOURCE_DIR}/src/Resource_Warehouse.c
	${PROJECT_SOURCE_DIR}/src/Runner.c
	${PROJECT_SOURCE_DIR}/src/Scenario.c
	${PROJECT_SOURCE_DIR}/src/Screen.c
	${PROJECT_SOURCE_DIR}/src/Security.c
	${PROJECT_SOURCE_DIR}/src/SidebarMenu.c
	${PROJECT_SOURCE_DIR}/src/Terrain.c
	${PROJECT_SOURCE_DIR}/src/TerrainGraphics.c
	${PROJECT_SOURCE_DIR}/src/TerrainGraphics_Context.c
	${PROJECT_SOURCE_DIR}/src/TerrainWater.c
	${PROJECT_SOURCE_DIR}/src/Trader.c
	${PROJECT_SOURCE_DIR}/src/Undo.c
	${PROJECT_SOURCE_DIR}/src/Video.c
	${PROJECT_SOURCE_DIR}/src/Widget_Button.c
	${PROJECT_SOURCE_DIR}/src/Widget_Menu.c
	${PROJECT_SOURCE_DIR}/src/Widget_Panel.c
	${PROJECT_SOURCE_DIR}/src/Widget_Text.c
	${PROJECT_SOURCE_DIR}/src/Data/Model.c
	${PROJECT_SOURCE_DIR}/src/UI/Advisor_Chief.c
	${PROJECT_SOURCE_DIR}/src/UI/Advisor_Education.c
	${PROJECT_SOURCE_DIR}/src/UI/Advisor_Entertainment.c
	${PROJECT_SOURCE_DIR}/src/UI/Advisor_Financial.c
	${PROJECT_SOURCE_DIR}/src/UI/Advisor_Health.c
	${PROJECT_SOURCE_DIR}/src/UI/Advisor_Imperial.c
	${PROJECT_SOURCE_DIR}/src/UI/Advisor_Labor.c
	${PROJECT_SOURCE_DIR}/src/UI/Advisor_Military.c
	${PROJECT_SOURCE_DIR}/src/UI/Advisor_Population.c
	${PROJECT_SOURCE_DIR}/src/UI/Advisor_Ratings.c
	${PROJECT_SOURCE_DIR}/src/UI/Advisor_Religion.c
	${PROJECT_SOURCE_DIR}/src/UI/Advisor_Trade.c
	${PROJECT_SOURCE_DIR}/src/UI/Advisors.c
	${PROJECT_SOURCE_DIR}/src/UI/BuildingInfo.c
	${PROJECT_SOURCE_DIR}/src/UI/BuildingInfo_Culture.c
	${PROJECT_SOURCE_DIR}/src/UI/BuildingInfo_Decoration.c
	${PROJECT_SOURCE_DIR}/src/UI/BuildingInfo_Distribution.c
	${PROJECT_SOURCE_DIR}/src/UI/BuildingInfo_Engineering.c
	${PROJECT_SOURCE_DIR}/src/UI/BuildingInfo_Entertainment.c
	${PROJECT_SOURCE_DIR}/src/UI/BuildingInfo_Figures.c
	${PROJECT_SOURCE_DIR}/src/UI/BuildingInfo_Government.c
	${PROJECT_SOURCE_DIR}/src/UI/BuildingInfo_House.c
	${PROJECT_SOURCE_DIR}/src/UI/BuildingInfo_Industry.c
	${PROJECT_SOURCE_DIR}/src/UI/BuildingInfo_Military.c
	${PROJECT_SOURCE_DIR}/src/UI/BuildingInfo_Native.c
	${PROJECT_SOURCE_DIR}/src/UI/BuildingInfo_Terrain.c
	${PROJECT_SOURCE_DIR}/src/UI/BuildingInfo_Water.c
	${PROJECT_SOURCE_DIR}/src/UI/BuildingMenu.c
	${PROJECT_SOURCE_DIR}/src/UI/CCKSelection.c
	${PROJECT_SOURCE_DIR}/src/UI/City.c
	${PROJECT_SOURCE_DIR}/src/UI/CityBuildings.c
	${PROJECT_SOURCE_DIR}/src/UI/CityBuildings_Figures.c
	${PROJECT_SOURCE_DIR}/src/UI/CityBuildings_Ghost.c
	${PROJECT_SOURCE_DIR}/src/UI/CityBuildings_Overlay.c
	${PROJECT_SOURCE_DIR}/src/UI/CityBuildings_Scroll.c
	${PROJECT_SOURCE_DIR}/src/UI/DifficultyOptions.c
	${PROJECT_SOURCE_DIR}/src/UI/DisplayOptions.c
	${PROJECT_SOURCE_DIR}/src/UI/DonateToCityDialog.c
	${PROJECT_SOURCE_DIR}/src/UI/EmpireMap.c
	${PROJECT_SOURCE_DIR}/src/UI/FileDialog.c
	${PROJECT_SOURCE_DIR}/src/UI/HoldFestivalDialog.c
	${PROJECT_SOURCE_DIR}/src/UI/Intermezzo.c
	${PROJECT_SOURCE_DIR}/src/UI/Logo.c
	${PROJECT_SOURCE_DIR}/src/UI/MainMenu.c
	${PROJECT_SOURCE_DIR}/src/UI/MessageDialog.c
	${PROJECT_SOURCE_DIR}/src/UI/Minimap.c
	${PROJECT_SOURCE_DIR}/src/UI/MissionEnd.c
	${PROJECT_SOURCE_DIR}/src/UI/MissionStart.c
	${PROJECT_SOURCE_DIR}/src/UI/NewCareerDialog.c
	${PROJECT_SOURCE_DIR}/src/UI/OverlayMenu.c
	${PROJECT_SOURCE_DIR}/src/UI/PlayerMessageList.c
	${PROJECT_SOURCE_DIR}/src/UI/PopupDialog.c
	${PROJECT_SOURCE_DIR}/src/UI/SendGiftToCaesarDialog.c
	${PROJECT_SOURCE_DIR}/src/UI/SetSalaryDialog.c
	${PROJECT_SOURCE_DIR}/src/UI/Sidebar.c
	${PROJECT_SOURCE_DIR}/src/UI/SoundOptions.c
	${PROJECT_SOURCE_DIR}/src/UI/SpeedOptions.c
	${PROJECT_SOURCE_DIR}/src/UI/Tooltip.c
	${PROJECT_SOURCE_DIR}/src/UI/TopMenu.c
	${PROJECT_SOURCE_DIR}/src/UI/VideoIntermezzo.c
	${PROJECT_SOURCE_DIR}/src/UI/Warning.c
	${PROJECT_SOURCE_DIR}/src/UI/Window.c
	${CORE_FILES}
	${BUILDING_FILES}
	${CITY_FILES}
	${EMPIRE_FILES}
	${FIGURE_FILES}
	${GAME_FILES}
	${INPUT_FILES}
	${MAP_FILES}
	${SCENARIO_FILES}
	${GRAPHICS_FILES}
	${SOUND_FILES}
	${SMK_FILES}
)

#include(FindPkgConfig)
#pkg_search_module(SDL2 REQUIRED sdl2)
#pkg_search_module(SDL2_MIXER REQUIRED SDL2_mixer)

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)

include_directories(${SDL2_INCLUDE_DIR})
include_directories(${SDL2_MIXER_INCLUDE_DIR})

#set(LIBS ${LIBS} ${SDL_LIBRARY})
#link_libraries(${LIBS})
target_link_libraries (julius ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY})

include_directories(src)

install(TARGETS julius RUNTIME DESTINATION bin)

# Unit tests
enable_testing()
add_subdirectory(test)

