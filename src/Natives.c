#include "Natives.h"

#include "Building.h"
#include "Terrain.h"

#include "Data/CityInfo.h"
#include "Data/State.h"

#include "building/building.h"
#include "building/list.h"
#include "core/calc.h"
#include "graphics/image.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/property.h"
#include "map/random.h"
#include "map/terrain.h"
#include "scenario/building.h"

static void determineMeetingCenter();

void Natives_init()
{
    int image_hut = scenario_building_image_native_hut();
    int image_meeting = scenario_building_image_native_meeting();
    int image_crops = scenario_building_image_native_crops();
	int nativeGraphic = image_group(GROUP_BUILDING_NATIVE);
	int gridOffset = Data_State.map.gridStartOffset;
	for (int y = 0; y < Data_State.map.height; y++, gridOffset += Data_State.map.gridBorderSize) {
		for (int x = 0; x < Data_State.map.width; x++, gridOffset++) {
			if (!map_terrain_is(gridOffset, TERRAIN_BUILDING) || map_building_at(gridOffset)) {
				continue;
			}
			
			int randomBit = map_random_get(gridOffset) & 1;
			int buildingType;
            int image_id = map_image_at(gridOffset);
			if (image_id == image_hut) {
				buildingType = BUILDING_NATIVE_HUT;
				map_image_set(gridOffset, nativeGraphic);
			} else if (image_id == image_hut + 1) {
				buildingType = BUILDING_NATIVE_HUT;
				map_image_set(gridOffset, nativeGraphic + 1);
			} else if (image_id == image_meeting) {
				buildingType = BUILDING_NATIVE_MEETING;
				map_image_set(gridOffset, nativeGraphic + 2);
			} else if (image_id == image_crops) {
				buildingType = BUILDING_NATIVE_CROPS;
				map_image_set(gridOffset, image_group(GROUP_BUILDING_FARM_CROPS) + randomBit);
			} else { //unknown building
				Terrain_removeBuildingFromGrids(0, x, y);
				continue;
			}
			int buildingId = Building_create(buildingType, x, y);
			map_building_set(gridOffset, buildingId);
			building *b = building_get(buildingId);
			b->state = BuildingState_InUse;
			switch (buildingType) {
				case BUILDING_NATIVE_CROPS:
					b->data.industry.progress = randomBit;
					break;
				case BUILDING_NATIVE_MEETING:
					b->sentiment.nativeAnger = 100;
					map_building_set(gridOffset + map_grid_delta(1, 0), buildingId);
					map_building_set(gridOffset + map_grid_delta(0, 1), buildingId);
					map_building_set(gridOffset + map_grid_delta(1, 1), buildingId);
					Terrain_markNativeLand(b->x, b->y, 2, 6);
					if (!Data_CityInfo.nativeMainMeetingCenterX) {
						Data_CityInfo.nativeMainMeetingCenterX = b->x;
						Data_CityInfo.nativeMainMeetingCenterY = b->y;
					}
					break;
				case BUILDING_NATIVE_HUT:
					b->sentiment.nativeAnger = 100;
					b->figureSpawnDelay = randomBit;
					Terrain_markNativeLand(b->x, b->y, 1, 3);
					break;
			}
		}
	}
	
	determineMeetingCenter();
}

static void determineMeetingCenter()
{
	// gather list of meeting centers
	building_list_small_clear();
	for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
		if (BuildingIsInUse(b) && b->type == BUILDING_NATIVE_MEETING) {
			building_list_small_add(i);
		}
	}
	int total_meetings = building_list_small_size();
	if (total_meetings <= 0) {
		return;
	}
	const int *meetings = building_list_small_items();
	// determine closest meeting center for hut
	for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
		if (BuildingIsInUse(b) && b->type == BUILDING_NATIVE_HUT) {
			int minDist = 1000;
			int minMeetingId = 0;
			for (int n = 0; n < total_meetings; n++) {
				building *meeting = building_get(meetings[n]);
				int dist = calc_maximum_distance(b->x, b->y, meeting->x, meeting->y);
				if (dist < minDist) {
					minDist = dist;
					minMeetingId = meetings[n];
				}
			}
			b->subtype.nativeMeetingCenterId = minMeetingId;
		}
	}
}

void Natives_checkLand()
{
	map_property_clear_all_native_land();
	if (Data_CityInfo.nativeAttackDuration) {
		Data_CityInfo.nativeAttackDuration--;
	}
	for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
		if (!BuildingIsInUse(b)) {
			continue;
		}
		int size, radius;
		if (b->type == BUILDING_NATIVE_HUT) {
			size = 1;
			radius = 3;
		} else if (b->type == BUILDING_NATIVE_MEETING) {
			size = 2;
			radius = 6;
		} else {
			continue;
		}
		if (b->sentiment.nativeAnger >= 100) {
			Terrain_markNativeLand(b->x, b->y, size, radius);
			if (Terrain_hasBuildingOnNativeLand(b->x, b->y, size, radius)) {
				Data_CityInfo.nativeAttackDuration = 2;
			}
		} else {
			b->sentiment.nativeAnger++;
		}
	}
}
