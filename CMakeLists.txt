cmake_minimum_required(VERSION 2.6)
project(julius)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
set(SMK_FILES src/Video/smacker.c src/Video/smk_bitstream.c src/Video/smk_hufftree.c)
set(CORE_FILES
    src/core/buffer.c
    src/core/calc.c
    src/core/debug.c
    src/core/dir.c
    src/core/file.c
    src/core/io.c
    src/core/lang.c
    src/core/random.c
    src/core/string.c
    src/core/time.c
    src/core/zip.c
)
set (BUILDING_FILES
    src/building/model.c
)
set (FIGURE_FILES
    src/figure/name.c
)
add_executable(julius
	linux/main2.c
	linux/SDLSoundDevice.c
	src/Animation.c
	src/Building.c
	src/BuildingHouse.c
	src/BuildingPlacement.c
	src/CityInfo.c
	src/CityInfo_Culture.c
	src/CityInfo_Finance.c
	src/CityInfo_Gods.c
	src/CityInfo_Labor.c
	src/CityInfo_Population.c
	src/CityInfo_Ratings.c
	src/CityInfo_Resource.c
	src/CityInfo_Tick.c
	src/CityInfo_Victory.c
	src/CityView.c
	src/Cursor.c
	src/Desirability.c
	src/Empire.c
	src/Event.c
	src/Event_Invasion.c
	src/Figure.c
	src/FigureAction.c
	src/FigureAction_Animal.c
	src/FigureAction_Cartpusher.c
	src/FigureAction_Combat.c
	src/FigureAction_Common.c
	src/FigureAction_Culture.c
	src/FigureAction_Docker.c
	src/FigureAction_Enemy.c
	src/FigureAction_Entertainer.c
	src/FigureAction_Market.c
	src/FigureAction_Migrant.c
	src/FigureAction_Missile.c
	src/FigureAction_Native.c
	src/FigureAction_Rioter.c
	src/FigureAction_Service.c
	src/FigureAction_Soldier.c
	src/FigureAction_Trader.c
	src/FigureAction_Wall.c
	src/FigureAction_Water.c
	src/FigureGeneration.c
	src/FigureMovement.c
	src/FigureRoute.c
	src/Figure_Phrase.c
	src/Figure_Service.c
	src/Formation.c
	src/FormationUpdate.c
	src/Game.c
	src/GameFile.c
	src/GameTick.c
	src/Graphics.c
	src/Graphics_Footprint.c
	src/Grid.c
	src/HouseEvolution.c
	src/HousePopulation.c
	src/KeyboardHotkey.c
	src/KeyboardInput.c
	src/Loader.c
	src/Loader_Graphics.c
	src/Natives.c
	src/PlayerMessage.c
	src/Resource.c
	src/Resource_Granary.c
	src/Resource_Warehouse.c
	src/Routing.c
	src/Runner.c
	src/Scenario.c
	src/Screen.c
	src/Scroll.c
	src/Security.c
	src/Settings.c
	src/SidebarMenu.c
	src/Sound.c
	src/Sound_City.c
	src/Sound_Music.c
	src/Terrain.c
	src/TerrainBridge.c
	src/TerrainGraphics.c
	src/TerrainGraphics_Context.c
	src/TerrainWater.c
	src/Trader.c
	src/Tutorial.c
	src/Undo.c
	src/UtilityManagement.c
	src/Video.c
	src/Widget_Button.c
	src/Widget_Menu.c
	src/Widget_Panel.c
	src/Widget_Text.c
	src/Data/Model.c
	src/UI/Advisor_Chief.c
	src/UI/Advisor_Education.c
	src/UI/Advisor_Entertainment.c
	src/UI/Advisor_Financial.c
	src/UI/Advisor_Health.c
	src/UI/Advisor_Imperial.c
	src/UI/Advisor_Labor.c
	src/UI/Advisor_Military.c
	src/UI/Advisor_Population.c
	src/UI/Advisor_Ratings.c
	src/UI/Advisor_Religion.c
	src/UI/Advisor_Trade.c
	src/UI/Advisors.c
	src/UI/BuildingInfo.c
	src/UI/BuildingInfo_Culture.c
	src/UI/BuildingInfo_Decoration.c
	src/UI/BuildingInfo_Distribution.c
	src/UI/BuildingInfo_Engineering.c
	src/UI/BuildingInfo_Entertainment.c
	src/UI/BuildingInfo_Figures.c
	src/UI/BuildingInfo_Government.c
	src/UI/BuildingInfo_House.c
	src/UI/BuildingInfo_Industry.c
	src/UI/BuildingInfo_Military.c
	src/UI/BuildingInfo_Native.c
	src/UI/BuildingInfo_Terrain.c
	src/UI/BuildingInfo_Water.c
	src/UI/BuildingMenu.c
	src/UI/CCKSelection.c
	src/UI/City.c
	src/UI/CityBuildings.c
	src/UI/CityBuildings_Figures.c
	src/UI/CityBuildings_Ghost.c
	src/UI/CityBuildings_Overlay.c
	src/UI/CityBuildings_Scroll.c
	src/UI/DifficultyOptions.c
	src/UI/DisplayOptions.c
	src/UI/DonateToCityDialog.c
	src/UI/EmpireMap.c
	src/UI/FileDialog.c
	src/UI/HoldFestivalDialog.c
	src/UI/Intermezzo.c
	src/UI/Logo.c
	src/UI/MainMenu.c
	src/UI/MessageDialog.c
	src/UI/Minimap.c
	src/UI/MissionEnd.c
	src/UI/MissionStart.c
	src/UI/NewCareerDialog.c
	src/UI/OverlayMenu.c
	src/UI/PlayerMessageList.c
	src/UI/PopupDialog.c
	src/UI/SendGiftToCaesarDialog.c
	src/UI/SetSalaryDialog.c
	src/UI/Sidebar.c
	src/UI/SoundOptions.c
	src/UI/SpeedOptions.c
	src/UI/Tooltip.c
	src/UI/TopMenu.c
	src/UI/VideoIntermezzo.c
	src/UI/Warning.c
	src/UI/Window.c
	${CORE_FILES}
	${BUILDING_FILES}
	${FIGURE_FILES}
	${SMK_FILES})

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

