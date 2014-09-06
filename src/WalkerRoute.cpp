#include "Walker.h"

#include "Routing.h"

#include "Data/Building.h"
#include "Data/Routes.h"
#include "Data/Walker.h"

void WalkerRoute_clearList()
{
	for (int i = 0; i < MAX_ROUTES; i++) {
		Data_Routes.walkerIds[i] = 0;
		for (int j = 0; j < MAX_ROUTEPATH_LENGTH; i++) {
			Data_Routes.directionPaths[i][j] = 0;
		}
	}
}

void WalkerRoute_clean()
{
	for (int i = 0; i < MAX_ROUTES; i++) {
		int walkerId = Data_Routes.walkerIds[i];
		if (walkerId > 0 && walkerId < MAX_WALKERS) {
			if (Data_Walkers[walkerId].state != WalkerState_Alive || Data_Walkers[walkerId].routingPathId != i) {
				Data_Routes.walkerIds[i] = 0;
			}
		}
	}
}

int WalkerRoute_getNumAvailable()
{
	int available = 0;
	for (int i = 0; i < MAX_ROUTES; i++) {
		if (!Data_Routes.walkerIds[i]) {
			available++;
		}
	}
	return available;
}

static int getFirstAvailable()
{
	for (int i = 1; i < 600; i++) {
		if (Data_Routes.walkerIds[i] == 0) {
			return i;
		}
	}
	return 0;
}
#include <cstdio>
void WalkerRoute_add(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->routingPathId = 0;
	w->routingPathCurrentTile = 0;
	w->routingPathLength = 0;
	int pathId = getFirstAvailable();
	if (!pathId) {
		return;
	}
	int pathLength;
	if (w->isBoat) {
		if (w->isBoat == 2) { // flotsam
			Routing_getDistanceWaterFlotsam(w->x, w->y);
			pathLength = Routing_getPathOnWater(pathId, w->x, w->y,
				w->destinationX, w->destinationY, 1);
		} else {
			Routing_getDistanceWaterBoat(w->x, w->y);
			pathLength = Routing_getPathOnWater(pathId, w->x, w->y,
				w->destinationX, w->destinationY, 0);
		}
	} else {
		// land walker
		int canTravel;
		printf("   Determining travelability from %d %d to %d %d\n",
			w->x, w->y, w->destinationX, w->destinationY);
		switch (w->terrainUsage) {
			case WalkerTerrainUsage_Enemy:
				canTravel = Routing_canTravelOverLandNonCitizen(w->x, w->y,
					w->destinationX, w->destinationY, w->destinationBuildingId, 5000);
				if (!canTravel) {
					canTravel = Routing_canTravelOverLandNonCitizen(w->x, w->y,
						w->destinationX, w->destinationY, w->destinationBuildingId, 25000);
					if (!canTravel) {
						canTravel = Routing_canTravelThroughEverythingNonCitizen(
							w->x, w->y, w->destinationX, w->destinationY);
					}
				}
				break;
			case WalkerTerrainUsage_Walls:
				canTravel = Routing_canTravelOverWalls(w->x, w->y,
					w->destinationX, w->destinationY);
				break;
			case WalkerTerrainUsage_Animal:
				canTravel = Routing_canTravelOverLandNonCitizen(w->x, w->y,
					w->destinationX, w->destinationY, MAX_BUILDINGS, 5000);
				break;
			case WalkerTerrainUsage_AnyLand:
				canTravel = Routing_canTravelOverRoadGardenCitizen(w->x, w->y,
					w->destinationX, w->destinationY);
				if (!canTravel) {
					canTravel = Routing_canTravelOverLandCitizen(w->x, w->y,
						w->destinationX, w->destinationY);
				}
				break;
			case WalkerTerrainUsage_Roads:
				canTravel = Routing_canTravelOverRoadGardenCitizen(w->x, w->y,
					w->destinationX, w->destinationY);
				break;
			default:
				canTravel = Routing_canTravelOverLandCitizen(w->x, w->y,
					w->destinationX, w->destinationY);
				break;
		}
		if (canTravel) {
			if (w->terrainUsage == WalkerTerrainUsage_Walls) {
				pathLength = Routing_getPath(4, pathId, w->x, w->y,
					w->destinationX, w->destinationY);
				if (pathLength <= 0) {
					pathLength = Routing_getPath(8, pathId, w->x, w->y,
						w->destinationX, w->destinationY);
				}
			} else {
				pathLength = Routing_getPath(8, pathId, w->x, w->y,
					w->destinationX, w->destinationY);
			}
		} else { // cannot travel
			pathLength = 0;
		}
		printf("   Done - can travel ? %d path length %d\n", canTravel, pathLength);
	}
	if (pathLength) {
		Data_Routes.walkerIds[pathId] = walkerId;
		w->routingPathId = pathId;
		w->routingPathLength = pathLength;
	}
}

void WalkerRoute_remove(int walkerId)
{
	int path = Data_Walkers[walkerId].routingPathId;
	if (path > 0) {
		if (Data_Routes.walkerIds[path] == walkerId) {
			Data_Routes.walkerIds[path] = 0;
		}
		Data_Walkers[walkerId].routingPathId = 0;
	}
}
