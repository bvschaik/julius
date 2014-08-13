
#include "Calc.h"

#include "Data/Building.h"
#include "Data/Grid.h"
#include "Data/Settings.h"
#include "Data/Walker.h"

static int determineDestination(int x, int y, int btype1, int btype2)
{
	int roadNetwork = Data_Grid_roadNetworks[GridOffset(x,y)];

	Data_BuildingList.small.size = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (b->inUse != 1) {
			continue;
		}
		if (b->type != btype1 && b->type != btype2) {
			continue;
		}
		if (b->distanceFromEntry && b->roadNetworkId == roadNetwork) {
			if (b->type == Building_Hippodrome && b->prevPartBuildingId) {
				continue;
			}
			DATA_BUILDINGLIST_SMALL_ENQUEUE(i);
		}
	}
	if (Data_BuildingList.small.size <= 0) {
		return 0;
	}
	int minBuildingId = 0;
	int minDistance = 10000;
	for (int i = 0; i < Data_BuildingList.small.size; i++) {
		struct Data_Building *b = &Data_Buildings[Data_BuildingList.small.items[i]];
		int daysLeft;
		if (b->type == btype1) {
			daysLeft = b->data.entertainment.days1;
		} else if (b->type == btype2) {
			daysLeft = b->data.entertainment.days2;
		} else {
			daysLeft = 0;
		}
		int dist = daysLeft + Calc_distanceMaximum(x, y, b->x, b->y);
		if (dist < minDistance) {
			minDistance = dist;
			minBuildingId = Data_BuildingList.small.items[i];
		}
	}
	if (minBuildingId) {
		return minBuildingId;
	}
	return 0;
}

static void updateShowsAtDestination(int walkerId)
{
	struct Data_Building *b = &Data_Buildings[Data_Walkers[walkerId].destinationBuildingId];
	switch (Data_Walkers[walkerId].type) {
		case Walker_Actor:
			b->data.entertainment.play++;
			if (b->data.entertainment.play >= 5) {
				b->data.entertainment.play = 0;
			}
			if (b->type == Building_Theater) {
				b->data.entertainment.days1 = 32;
			} else {
				b->data.entertainment.days2 = 32;
			}
			break;
		case Walker_Gladiator:
			if (b->type == Building_Amphitheater) {
				b->data.entertainment.days1 = 32;
			} else {
				b->data.entertainment.days2 = 32;
			}
			break;
		case Walker_LionTamer:
		case Walker_Charioteer:
			b->data.entertainment.days1 = 32;
			break;
	}
}
