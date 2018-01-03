#ifndef DATA_BUILDING_H
#define DATA_BUILDING_H

#include "building/type.h"

#define BuildingIsInUse(b) ((b)->state == BUILDING_STATE_IN_USE)

extern struct _Data_Buildings_Extra {
	int highestBuildingIdInUse;
	int highestBuildingIdEver;
	int createdSequence;
	int barracksTowerSentryRequested;

    int maxConnectsEver; // only used in saved game
    int incorrectHousePositions;
    int unfixableHousePositions;
} Data_Buildings_Extra;

#endif
