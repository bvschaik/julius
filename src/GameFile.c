#include "GameFile.h"

#include "Building.h"
#include "CityView.h"

#include "Data/CityInfo.h"
#include "Data/State.h"
#include "UI/AllWindows.h" // TODO: try to eliminate this

#include "building/granary.h"
#include "building/menu.h"
#include "building/storage.h"
#include "city/message.h"
#include "core/file.h"
#include "core/io.h"
#include "empire/empire.h"
#include "empire/trade_prices.h"
#include "figure/route.h"
#include "game/file_io.h"
#include "game/state.h"
#include "game/undo.h"
#include "graphics/image.h"
#include "map/orientation.h"
#include "map/road_network.h"
#include "map/routing_terrain.h"
#include "map/tiles.h"
#include "scenario/distant_battle.h"
#include "scenario/empire.h"
#include "scenario/map.h"
#include "scenario/property.h"
#include "sound/city.h"
#include "sound/music.h"

static const char missionPackFile[] = "mission1.pak";

static const char missionSavedGames[][32] = {
	"Citizen.sav",
	"Clerk.sav",
	"Engineer.sav",
	"Architect.sav",
	"Quaestor.sav",
	"Procurator.sav",
	"Aedile.sav",
	"Praetor.sav",
	"Consul.sav",
	"Proconsul.sav",
	"Caesar.sav",
	"Caesar2.sav"
};

static void setupFromSavedGame();

int GameFile_loadSavedGame(const char *filename)
{
    if (!game_file_io_read_saved_game(filename, 0)) {
        return 0;
    }
	sound_music_stop();
	
	setupFromSavedGame();
	building_storage_reset_building_ids();
	return 1;
}

int GameFile_loadSavedGameFromMissionPack(int missionId)
{
	int offset;
    // TODO use buffer
	if (!io_read_file_part_into_buffer(missionPackFile, &offset, 4, 4 * missionId)) {
		return 0;
	}
	if (offset <= 0) {
		return 0;
	}
    if (!game_file_io_read_saved_game(missionPackFile, offset)) {
        return 0;
    }

    setupFromSavedGame();
	return 1;
}

static void debug()
{/*
	printf("TIME: y %d m %d d %d t %d\n", game_time_year(), game_time_month(), game_time_day(), game_time_tick());
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		building *b = building_get(i);
		if (b->state != BuildingState_Unused || b->type) {
			printf("Building %d type %d inUse %d x %d y %d emp %d w %d ls %d hc %d\n",
				i, b->type, b->state, b->x, b->y, b->numWorkers, b->figureId, b->figureId2, b->housesCovered);
		}
	}
	for (int i = 1; i < MAX_FIGURES; i++) {
		figure *f = &Data_Figures[i];
		if (f->state == FigureState_Alive) {
			printf("Figure %d type %d as %d wt %d mt %d\n",
				i, f->type, f->actionState, f->waitTicks, f->waitTicksMissile);
		}
	}*/
}

static void load_empire_data(int is_custom_scenario, int empire_id)
{
    empire_load(is_custom_scenario, empire_id);
    scenario_distant_battle_set_roman_travel_months();
    scenario_distant_battle_set_enemy_travel_months();
}

static void setupFromSavedGame()
{
	debug();
	load_empire_data(scenario_is_custom(), scenario_empire_id());

	scenario_map_init();

	if (Data_State.map.orientation >= 0 && Data_State.map.orientation <= 6) {
		// ensure even number
		Data_State.map.orientation = 2 * (Data_State.map.orientation / 2);
	} else {
		Data_State.map.orientation = 0;
	}

	CityView_calculateLookup();
	CityView_checkCameraBoundaries();

	map_routing_update_all();

	map_orientation_update_buildings();
	figure_route_clean();
	map_road_network_update();
	Building_GameTick_checkAccessToRome();
	building_granaries_calculate_stocks();
	building_menu_update();
	city_message_init_problem_areas();

	sound_city_init();
	sound_music_reset();

	game_undo_disable();
	game_state_reset_overlay();
	Data_State.missionBriefingShown = 1;

	Data_CityInfo.tutorial1FireMessageShown = 1;
	Data_CityInfo.tutorial3DiseaseMessageShown = 1;

    image_load_climate(scenario_property_climate());
    image_load_enemy(scenario_property_enemy());
	empire_determine_distant_battle_city();
	map_tiles_determine_gardens();

    UI_PlayerMessageList_resetScroll();

	game_state_unpause();
}

void GameFile_writeMissionSavedGameIfNeeded()
{
	int rank = scenario_campaign_rank();
	if (rank < 0) {
		rank = 0;
	} else if (rank > 11) {
		rank = 11;
	}
	if (!Data_CityInfo.missionSavedGameWritten) {
		Data_CityInfo.missionSavedGameWritten = 1;
		if (!file_exists(missionSavedGames[rank])) {
			game_file_io_write_saved_game(missionSavedGames[rank]);
		}
	}
}

int GameFile_writeSavedGame(const char *filename)
{
    return game_file_io_write_saved_game(filename);
}

int GameFile_deleteSavedGame(const char *filename)
{
    return game_file_io_delete_saved_game(filename);
}

int GameFile_loadScenario(const char *filename)
{
    if (!game_file_io_read_scenario(filename)) {
        return 0;
    }
    
    trade_prices_reset();
    load_empire_data(1, scenario_empire_id());
    return 1;
}
