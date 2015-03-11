#include "Natives.h"

#include "Building.h"
#include "Calc.h"
#include "Grid.h"
#include "Terrain.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Graphics.h"
#include "Data/Grid.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"

static void determineMeetingCenter();

void Natives_init()
{
	int nativeGraphic = GraphicId(ID_Graphic_NativeBuilding);
	int gridOffset = Data_Settings_Map.gridStartOffset;
	for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize) {
		for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {
			if (!(Data_Grid_terrain[gridOffset] & Terrain_Building) || Data_Grid_buildingIds[gridOffset]) {
				continue;
			}
			
			int randomBit = Data_Grid_random[gridOffset] & 1;
			int buildingType;
			if (Data_Grid_graphicIds[gridOffset] == Data_Scenario.nativeGraphics.hut) {
				buildingType = Building_NativeHut;
				Data_Grid_graphicIds[gridOffset] = nativeGraphic;
			} else if (Data_Grid_graphicIds[gridOffset] == Data_Scenario.nativeGraphics.hut + 1) {
				buildingType = Building_NativeHut;
				Data_Grid_graphicIds[gridOffset] = nativeGraphic + 1;
			} else if (Data_Grid_graphicIds[gridOffset] == Data_Scenario.nativeGraphics.meetingCenter) {
				buildingType = Building_NativeMeeting;
				Data_Grid_graphicIds[gridOffset] = nativeGraphic + 2;
			} else if (Data_Grid_graphicIds[gridOffset] == Data_Scenario.nativeGraphics.crops) {
				buildingType = Building_NativeCrops;
				Data_Grid_graphicIds[gridOffset] = GraphicId(ID_Graphic_FarmCrops) + randomBit;
			} else { //unknown building
				Terrain_removeBuildingFromGrids(0, x, y);
				continue;
			}
			int buildingId = Building_create(buildingType, x, y);
			Data_Grid_buildingIds[gridOffset] = buildingId;
			struct Data_Building *b = &Data_Buildings[buildingId];
			b->state = BuildingState_InUse;
			switch (buildingType) {
				case Building_NativeCrops:
					b->data.industry.progress = randomBit;
					break;
				case Building_NativeMeeting:
					b->sentiment.nativeAnger = 100;
					Data_Grid_buildingIds[gridOffset + 1] = buildingId;
					Data_Grid_buildingIds[gridOffset + 162] = buildingId;
					Data_Grid_buildingIds[gridOffset + 163] = buildingId;
					Terrain_markNativeLand(b->x, b->y, 2, 6);
					if (!Data_CityInfo.nativeMainMeetingCenterX) {
						Data_CityInfo.nativeMainMeetingCenterX = b->x;
						Data_CityInfo.nativeMainMeetingCenterY = b->y;
					}
					break;
				case Building_NativeHut:
					b->sentiment.nativeAnger = 100;
					b->walkerSpawnDelay = randomBit;
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
	Data_BuildingList.small.size = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (BuildingIsInUse(i) && Data_Buildings[i].type == Building_NativeMeeting) {
			DATA_BUILDINGLIST_SMALL_ENQUEUE(i);
		}
	}
	if (Data_BuildingList.small.size <= 0) {
		return;
	}
	// determine closest meeting center for hut
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (BuildingIsInUse(i) && Data_Buildings[i].type == Building_NativeHut) {
			int minDist = 1000;
			int minMeetingId = 0;
			for (int n = 0; n < Data_BuildingList.small.size; n++) {
				int meetingId = Data_BuildingList.small.items[n];
				int dist = Calc_distanceMaximum(Data_Buildings[i].x, Data_Buildings[i].y,
					Data_Buildings[meetingId].x, Data_Buildings[meetingId].y);
				if (dist < minDist) {
					minDist = dist;
					minMeetingId = meetingId;
				}
			}
			Data_Buildings[i].subtype.nativeMeetingCenterId = minMeetingId;
		}
	}
}

void Natives_checkLand()
{
	Grid_andByteGrid(Data_Grid_edge, Edge_NoNativeLand);
	if (Data_CityInfo.nativeAttackDuration) {
		Data_CityInfo.nativeAttackDuration--;
	}
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (!BuildingIsInUse(i)) {
			continue;
		}
		struct Data_Building *b = &Data_Buildings[i];
		int size, radius;
		if (b->type == Building_NativeHut) {
			size = 1;
			radius = 3;
		} else if (b->type == Building_NativeMeeting) {
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
