#ifndef DATA_BUILDING_H
#define DATA_BUILDING_H

#include "building/type.h"

#define MAX_HOUSE_LEVELS 20

enum {
	BuildingState_Unused = 0,
	BuildingState_InUse = 1,
	BuildingState_Undo = 2,
	BuildingState_Created = 3,
	BuildingState_Rubble = 4,
	BuildingState_DeletedByGame = 5, // used for earthquakes, fires, house mergers
	BuildingState_DeletedByPlayer = 6
};

#define BuildingIsHouse(type) ((type) >= BUILDING_HOUSE_VACANT_LOT && (type) <= BUILDING_HOUSE_LUXURY_PALACE)

#define BuildingIsInUse(b) ((b)->state == BuildingState_InUse)

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
