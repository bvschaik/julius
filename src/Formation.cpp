#include "Formation.h"
#include "Data/CityInfo.h"
#include "Data/Formation.h"
#include "Data/Walker.h"

#include <string.h>

void Formation_clearList()
{
	for (int i = 0; i < MAX_FORMATIONS; i++) {
		memset(&Data_Formations[i], sizeof(struct Data_Formation), 0);
	}
	Data_Formation_Extra.idLastInUse = 0;
	Data_Formation_Extra.idLastLegion = 0;
	Data_Formation_Extra.numLegions = 0;
}

void Formation_deleteFortAndBanner(int formationId)
{
	// TODO
}

void Formation_setMaxSoldierPerLegion()
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1 && Data_Formations[i].isLegion) {
			Data_Formations[i].maxWalkers = 16;
		}
	}
}

int Formation_getNumLegions()
{
	int numLegions = 0;
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1 && Data_Formations[i].isLegion) {
			numLegions++;
		}
	}
	return numLegions;
}

int Formation_getLegionFormationId(int legionIndex)
{
	int index = 1;
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1 && Data_Formations[i].isLegion) {
			if (index == legionIndex) {
				return i;
			}
			index++;
		}
	}
	return 0;
}

void Formation_legionReturnHome(int formationId)
{
	// TODO
}

void Formation_calculateLegionTotals()
{
	Data_Formation_Extra.idLastLegion = 0;
	Data_Formation_Extra.numLegions = 0;
	Data_CityInfo.militaryLegionaryLegions = 0;
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1) {
			if (Data_Formations[i].isLegion) {
				Data_Formation_Extra.idLastLegion = i;
				Data_Formation_Extra.numLegions++;
				if (Data_Formations[i].walkerType == Walker_FortLegionary) {
					Data_CityInfo.militaryLegionaryLegions++;
				}
			}
			if (Data_Formations[i].__unknown56 <= 0 &&
				Data_Formations[i].walkerIds[0]) {
				int walkerId = Data_Formations[i].walkerIds[0];
				if (Data_Walkers[walkerId].state == WalkerState_1) {
					Data_Formations[i].xFort = Data_Walkers[walkerId].x;
					Data_Formations[i].yFort = Data_Walkers[walkerId].y;
				}
			}
		}
	}
}

