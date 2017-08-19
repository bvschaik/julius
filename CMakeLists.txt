cmake_minimum_required(VERSION 2.6)
project(julius)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/")

message("${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
set(SMK_FILES src/Video/smacker.c src/Video/smk_bitstream.c src/Video/smk_hufftree.c)
set(CORE_FILES src/core/calc.c)
add_executable(julius
	linux/main2.cpp
	linux/SDLSoundDevice.cpp
	src/Animation.cpp
	src/Building.cpp
	src/BuildingHouse.cpp
	src/BuildingPlacement.cpp
	src/CityInfo.cpp
	src/CityInfo_Culture.cpp
	src/CityInfo_Finance.cpp
	src/CityInfo_Gods.cpp
	src/CityInfo_Labor.cpp
	src/CityInfo_Population.cpp
	src/CityInfo_Ratings.cpp
	src/CityInfo_Resource.cpp
	src/CityInfo_Tick.cpp
	src/CityInfo_Victory.cpp
	src/CityView.cpp
	src/Cursor.cpp
	src/Desirability.cpp
	src/Empire.cpp
	src/Event.cpp
	src/Event_Invasion.cpp
	src/Figure.cpp
	src/FigureAction.cpp
	src/FigureAction_Animal.cpp
	src/FigureAction_Cartpusher.cpp
	src/FigureAction_Combat.cpp
	src/FigureAction_Common.cpp
	src/FigureAction_Culture.cpp
	src/FigureAction_Docker.cpp
	src/FigureAction_Enemy.cpp
	src/FigureAction_Entertainer.cpp
	src/FigureAction_Market.cpp
	src/FigureAction_Migrant.cpp
	src/FigureAction_Missile.cpp
	src/FigureAction_Native.cpp
	src/FigureAction_Rioter.cpp
	src/FigureAction_Service.cpp
	src/FigureAction_Soldier.cpp
	src/FigureAction_Trader.cpp
	src/FigureAction_Wall.cpp
	src/FigureAction_Water.cpp
	src/FigureGeneration.cpp
	src/FigureMovement.cpp
	src/FigureNames.cpp
	src/FigureRoute.cpp
	src/Figure_Phrase.cpp
	src/Figure_Service.cpp
	src/FileSystem.cpp
	src/Formation.cpp
	src/FormationUpdate.cpp
	src/Game.cpp
	src/GameFile.cpp
	src/GameTick.cpp
	src/Graphics.cpp
	src/Graphics_Footprint.cpp
	src/Grid.cpp
	src/HouseEvolution.cpp
	src/HousePopulation.cpp
	src/KeyboardHotkey.cpp
	src/KeyboardInput.cpp
	src/Language.cpp
	src/Loader.cpp
	src/Loader_Graphics.cpp
	src/Loader_Model.cpp
	src/Natives.cpp
	src/PlayerMessage.cpp
	src/Random.cpp
	src/Resource.cpp
	src/Resource_Granary.cpp
	src/Resource_Warehouse.cpp
	src/Routing.cpp
	src/Runner.cpp
	src/Scenario.cpp
	src/Screen.cpp
	src/Scroll.cpp
	src/Security.cpp
	src/Settings.cpp
	src/SidebarMenu.cpp
	src/Sound.cpp
	src/Sound_City.cpp
	src/Sound_Music.cpp
	src/String.cpp
	src/Terrain.cpp
	src/TerrainBridge.cpp
	src/TerrainGraphics.cpp
	src/TerrainGraphics_Context.cpp
	src/TerrainWater.cpp
	src/Time.cpp
	src/Trader.cpp
	src/Tutorial.cpp
	src/Undo.cpp
	src/UtilityManagement.cpp
	src/Video.cpp
	src/Widget_Button.cpp
	src/Widget_Menu.cpp
	src/Widget_Panel.cpp
	src/Widget_Text.cpp
	src/Zip.cpp
	src/Data/Model.cpp
	src/UI/Advisor_Chief.cpp
	src/UI/Advisor_Education.cpp
	src/UI/Advisor_Entertainment.cpp
	src/UI/Advisor_Financial.cpp
	src/UI/Advisor_Health.cpp
	src/UI/Advisor_Imperial.cpp
	src/UI/Advisor_Labor.cpp
	src/UI/Advisor_Military.cpp
	src/UI/Advisor_Population.cpp
	src/UI/Advisor_Ratings.cpp
	src/UI/Advisor_Religion.cpp
	src/UI/Advisor_Trade.cpp
	src/UI/Advisors.cpp
	src/UI/BuildingInfo.cpp
	src/UI/BuildingInfo_Culture.cpp
	src/UI/BuildingInfo_Decoration.cpp
	src/UI/BuildingInfo_Distribution.cpp
	src/UI/BuildingInfo_Engineering.cpp
	src/UI/BuildingInfo_Entertainment.cpp
	src/UI/BuildingInfo_Figures.cpp
	src/UI/BuildingInfo_Government.cpp
	src/UI/BuildingInfo_House.cpp
	src/UI/BuildingInfo_Industry.cpp
	src/UI/BuildingInfo_Military.cpp
	src/UI/BuildingInfo_Native.cpp
	src/UI/BuildingInfo_Terrain.cpp
	src/UI/BuildingInfo_Water.cpp
	src/UI/BuildingMenu.cpp
	src/UI/CCKSelection.cpp
	src/UI/City.cpp
	src/UI/CityBuildings.cpp
	src/UI/CityBuildings_Figures.cpp
	src/UI/CityBuildings_Ghost.cpp
	src/UI/CityBuildings_Overlay.cpp
	src/UI/CityBuildings_Scroll.cpp
	src/UI/DifficultyOptions.cpp
	src/UI/DisplayOptions.cpp
	src/UI/DonateToCityDialog.cpp
	src/UI/EmpireMap.cpp
	src/UI/FileDialog.cpp
	src/UI/HoldFestivalDialog.cpp
	src/UI/Intermezzo.cpp
	src/UI/Logo.cpp
	src/UI/MainMenu.cpp
	src/UI/MessageDialog.cpp
	src/UI/Minimap.cpp
	src/UI/MissionEnd.cpp
	src/UI/MissionStart.cpp
	src/UI/NewCareerDialog.cpp
	src/UI/OverlayMenu.cpp
	src/UI/PlayerMessageList.cpp
	src/UI/PopupDialog.cpp
	src/UI/SendGiftToCaesarDialog.cpp
	src/UI/SetSalaryDialog.cpp
	src/UI/Sidebar.cpp
	src/UI/SoundOptions.cpp
	src/UI/SpeedOptions.cpp
	src/UI/Tooltip.cpp
	src/UI/TopMenu.cpp
	src/UI/VideoIntermezzo.cpp
	src/UI/Warning.cpp
	src/UI/Window.cpp
	${CORE_FILES}
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

