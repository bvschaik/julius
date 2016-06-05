#include "Walker.h"

#include "Routing.h"

#include "Data/Building.h"
#include "Data/Routes.h"
#include "Data/Figure.h"

void FigureRoute_clearList()
{
	for (int i = 0; i < MAX_ROUTES; i++) {
		Data_Routes.figureIds[i] = 0;
		for (int j = 0; j < MAX_ROUTEPATH_LENGTH; j++) {
			Data_Routes.directionPaths[i][j] = 0;
		}
	}
}

void FigureRoute_clean()
{
	for (int i = 0; i < MAX_ROUTES; i++) {
		int walkerId = Data_Routes.figureIds[i];
		if (walkerId > 0 && walkerId < MAX_FIGURES) {
			if (Data_Walkers[walkerId].state != FigureState_Alive || Data_Walkers[walkerId].routingPathId != i) {
				Data_Routes.figureIds[i] = 0;
			}
		}
	}
}

int FigureRoute_getNumAvailable()
{
	int available = 0;
	for (int i = 1; i < MAX_ROUTES; i++) {
		if (!Data_Routes.figureIds[i]) {
			available++;
		}
	}
	return available;
}

static int getFirstAvailable()
{
	for (int i = 1; i < MAX_ROUTES; i++) {
		if (Data_Routes.figureIds[i] == 0) {
			return i;
		}
	}
	return 0;
}

void FigureRoute_add(int walkerId)
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
		switch (w->terrainUsage) {
			case FigureTerrainUsage_Enemy:
				canTravel = Routing_canTravelOverLandNonCitizen(w->x, w->y,
					w->destinationX, w->destinationY, w->destinationBuildingId, 5000);
				if (!canTravel) {
					canTravel = Routing_canTravelOverLandNonCitizen(w->x, w->y,
						w->destinationX, w->destinationY, 0, 25000);
					if (!canTravel) {
						canTravel = Routing_canTravelThroughEverythingNonCitizen(
							w->x, w->y, w->destinationX, w->destinationY);
					}
				}
				break;
			case FigureTerrainUsage_Walls:
				canTravel = Routing_canTravelOverWalls(w->x, w->y,
					w->destinationX, w->destinationY);
				break;
			case FigureTerrainUsage_Animal:
				canTravel = Routing_canTravelOverLandNonCitizen(w->x, w->y,
					w->destinationX, w->destinationY, MAX_BUILDINGS, 5000);
				break;
			case FigureTerrainUsage_PreferRoads:
				canTravel = Routing_canTravelOverRoadGardenCitizen(w->x, w->y,
					w->destinationX, w->destinationY);
				if (!canTravel) {
					canTravel = Routing_canTravelOverLandCitizen(w->x, w->y,
						w->destinationX, w->destinationY);
				}
				break;
			case FigureTerrainUsage_Roads:
				canTravel = Routing_canTravelOverRoadGardenCitizen(w->x, w->y,
					w->destinationX, w->destinationY);
				break;
			default:
				canTravel = Routing_canTravelOverLandCitizen(w->x, w->y,
					w->destinationX, w->destinationY);
				break;
		}
		if (canTravel) {
			if (w->terrainUsage == FigureTerrainUsage_Walls) {
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
	}
	if (pathLength) {
		Data_Routes.figureIds[pathId] = walkerId;
		w->routingPathId = pathId;
		w->routingPathLength = pathLength;
	}
}

void FigureRoute_remove(int walkerId)
{
	int path = Data_Walkers[walkerId].routingPathId;
	if (path > 0) {
		if (Data_Routes.figureIds[path] == walkerId) {
			Data_Routes.figureIds[path] = 0;
		}
		Data_Walkers[walkerId].routingPathId = 0;
	}
}
