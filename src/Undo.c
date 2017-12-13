#include "Undo.h"

#include "Resource.h"
#include "Terrain.h"
#include "TerrainGraphics.h"
#include "UI/Window.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/State.h"

#include "building/properties.h"
#include "city/finance.h"
#include "game/resource.h"
#include "graphics/image.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/property.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "scenario/earthquake.h"

#include <string.h>

#define MAX_UNDO_BUILDINGS 50

static struct {
	int timeout;
	int buildingCost;
	int numBuildings;
	int buildingType;
	struct Data_Building buildings[MAX_UNDO_BUILDINGS];
	short buildingIndex[MAX_UNDO_BUILDINGS];
} data;

static void clearBuildingList()
{
	data.numBuildings = 0;
	memset(data.buildings, 0, MAX_UNDO_BUILDINGS * sizeof(struct Data_Building));
	memset(data.buildingIndex, 0, MAX_UNDO_BUILDINGS * sizeof(short));
}

int Undo_recordBeforeBuild()
{
	Data_State.undoReady = 0;
	Data_State.undoAvailable = 1;
	data.timeout = 0;
	data.buildingCost = 0;
	data.buildingType = Data_State.selectedBuilding.type;
	clearBuildingList();
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].state == BuildingState_Undo) {
			Data_State.undoAvailable = 0;
			return 0;
		}
		if (Data_Buildings[i].state == BuildingState_DeletedByPlayer) {
			Data_State.undoAvailable = 0;
		}
	}

	map_image_backup();
	map_terrain_backup();
	map_grid_copy_u8(Data_Grid_aqueducts, Data_Grid_Undo_aqueducts);
	map_property_backup();
	map_grid_copy_u8(Data_Grid_spriteOffsets, Data_Grid_Undo_spriteOffsets);

	return 1;
}

void Undo_restoreBuildings()
{
	for (int i = 0; i < data.numBuildings; i++) {
		if (data.buildingIndex[i]) {
			struct Data_Building *b = &Data_Buildings[data.buildingIndex[i]];
			if (b->state == BuildingState_DeletedByPlayer) {
				b->state = BuildingState_InUse;
			}
			b->isDeleted = 0;
		}
	}
	clearBuildingList();
}

void Undo_addBuildingToList(int buildingId)
{
	if (buildingId <= 0) {
		return;
	}
	data.numBuildings = 0;
	int isOnList = 0;
	for (int i = 0; i < MAX_UNDO_BUILDINGS; i++) {
		if (data.buildingIndex[i]) {
			data.numBuildings++;
		}
		if (data.buildingIndex[i] == buildingId) {
			isOnList = 1;
		}
	}
	if (!isOnList) {
		for (int i = 0; i < MAX_UNDO_BUILDINGS; i++) {
			if (!data.buildingIndex[i]) {
				data.numBuildings++;
				memcpy(&data.buildings[i], &Data_Buildings[buildingId], sizeof(struct Data_Building));
				data.buildingIndex[i] = buildingId;
				return;
			}
		}
		Data_State.undoAvailable = 0;
	}
}

int Undo_isBuildingInList(int buildingId)
{
	if (buildingId <= 0 || Data_State.undoReady != 1 || Data_State.undoAvailable != 1) {
		return 0;
	}
	if (data.numBuildings <= 0) {
		return 0;
	}
	for (int i = 0; i < MAX_UNDO_BUILDINGS; i++) {
		if (data.buildingIndex[i] == buildingId) {
			return 1;
		}
	}
	return 0;
}

void Undo_recordBuild(int cost)
{
	Data_State.undoReady = 1;
	data.timeout = 500;
	data.buildingCost = cost;
	UI_Window_requestRefresh();
}

void Undo_restoreTerrainGraphics()
{
	for (int y = 0; y < Data_State.map.height; y++) {
		for (int x = 0; x < Data_State.map.width; x++) {
			int gridOffset = map_grid_offset(x, y);
			if (!map_building_at(gridOffset)) {
				map_image_restore_at(gridOffset);
			}
		}
	}
}

static void placeBuildingOnTerrain(int buildingId)
{
	if (buildingId <= 0) {
		return;
	}
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (BuildingIsFarm(b->type)) {
		int graphicOffset;
		switch (b->type) {
			default:
			case BUILDING_WHEAT_FARM: graphicOffset = 0; break;
			case BUILDING_VEGETABLE_FARM: graphicOffset = 5; break;
			case BUILDING_FRUIT_FARM: graphicOffset = 10; break;
			case BUILDING_OLIVE_FARM: graphicOffset = 15; break;
			case BUILDING_VINES_FARM: graphicOffset = 20; break;
			case BUILDING_PIG_FARM: graphicOffset = 25; break;
		}
		TerrainGraphics_setBuildingFarm(buildingId, b->x, b->y,
			image_group(GROUP_BUILDING_FARM_CROPS) + graphicOffset, 0);
	} else {
		int size = building_properties_for_type(b->type)->size;
		Terrain_addBuildingToGrids(buildingId, b->x, b->y, size, 0, 0);
		if (b->type == BUILDING_WHARF) {
			b->data.other.boatFigureId = 0;
		}
	}
	b->state = BuildingState_InUse;
}

void Undo_perform()
{
	if (!Data_State.undoReady || !Data_State.undoAvailable) {
		return;
	}
	Data_State.undoAvailable = 0;
	city_finance_process_construction(-data.buildingCost);
	if (data.buildingType == BUILDING_CLEAR_LAND) {
		for (int i = 0; i < data.numBuildings; i++) {
			if (data.buildingIndex[i]) {
				int buildingId = data.buildingIndex[i];
				memcpy(&Data_Buildings[buildingId], &data.buildings[i],
					sizeof(struct Data_Building));
				placeBuildingOnTerrain(buildingId);
			}
		}
		map_terrain_restore();
		map_grid_copy_u8(Data_Grid_Undo_aqueducts, Data_Grid_aqueducts);
		map_grid_copy_u8(Data_Grid_Undo_spriteOffsets, Data_Grid_spriteOffsets);
		map_image_restore();
		map_property_restore();
		map_property_clear_constructing_and_deleted();
	} else if (data.buildingType == BUILDING_AQUEDUCT || data.buildingType == BUILDING_ROAD ||
			data.buildingType == BUILDING_WALL) {
		map_terrain_restore();
		map_grid_copy_u8(Data_Grid_Undo_aqueducts, Data_Grid_aqueducts);
		Undo_restoreTerrainGraphics();
	} else if (data.buildingType == BUILDING_LOW_BRIDGE || data.buildingType == BUILDING_SHIP_BRIDGE) {
		map_terrain_restore();
		map_grid_copy_u8(Data_Grid_Undo_spriteOffsets, Data_Grid_spriteOffsets);
		Undo_restoreTerrainGraphics();
	} else if (data.buildingType == BUILDING_PLAZA || data.buildingType == BUILDING_GARDENS) {
		map_terrain_restore();
		map_grid_copy_u8(Data_Grid_Undo_aqueducts, Data_Grid_aqueducts);
		map_property_restore();
		Undo_restoreTerrainGraphics();
	} else if (data.numBuildings) {
		if (data.buildingType == BUILDING_DRAGGABLE_RESERVOIR) {
			map_terrain_restore();
			map_grid_copy_u8(Data_Grid_Undo_aqueducts, Data_Grid_aqueducts);
			Undo_restoreTerrainGraphics();
		}
		for (int i = 0; i < data.numBuildings; i++) {
			if (data.buildingIndex[i]) {
				struct Data_Building *b = &Data_Buildings[data.buildingIndex[i]];
				if (b->type == BUILDING_ORACLE || (b->type >= BUILDING_LARGE_TEMPLE_CERES && b->type <= BUILDING_LARGE_TEMPLE_VENUS)) {
					Resource_addToCityWarehouses(RESOURCE_MARBLE, 2);
				}
				b->state = BuildingState_Undo;
			}
		}
	}
	map_routing_update_land();
	map_routing_update_walls();
	data.numBuildings = 0;
}

void Undo_updateAvailable()
{
	if (!Data_State.undoAvailable || !Data_State.undoReady) {
		return;
	}
	if (data.timeout <= 0 || scenario_earthquake_is_in_progress()) {
		Data_State.undoAvailable = 0;
		clearBuildingList();
		UI_Window_requestRefresh();
		return;
	}
	data.timeout--;
	switch (data.buildingType) {
		case BUILDING_CLEAR_LAND:
		case BUILDING_AQUEDUCT:
		case BUILDING_ROAD:
		case BUILDING_WALL:
		case BUILDING_LOW_BRIDGE:
		case BUILDING_SHIP_BRIDGE:
		case BUILDING_PLAZA:
		case BUILDING_GARDENS:
			return;
	}
	if (data.numBuildings <= 0) {
		Data_State.undoAvailable = 0;
		UI_Window_requestRefresh();
		return;
	}
	if (data.buildingType == BUILDING_HOUSE_VACANT_LOT) {
		for (int i = 0; i < data.numBuildings; i++) {
			if (data.buildingIndex[i] && Data_Buildings[data.buildingIndex[i]].housePopulation) {
				// no undo on a new house where people moved in
				Data_State.undoAvailable = 0;
				UI_Window_requestRefresh();
				return;
			}
		}
	}
	for (int i = 0; i < data.numBuildings; i++) {
		if (data.buildingIndex[i]) {
			struct Data_Building *b = &Data_Buildings[data.buildingIndex[i]];
			if (b->state == BuildingState_Undo ||
				b->state == BuildingState_Rubble ||
				b->state == BuildingState_DeletedByGame) {
				Data_State.undoAvailable = 0;
				UI_Window_requestRefresh();
				return;
			}
			if (b->type != data.buildings[i].type || b->gridOffset != data.buildings[i].gridOffset) {
				Data_State.undoAvailable = 0;
				UI_Window_requestRefresh();
				return;
			}
		}
	}
}
