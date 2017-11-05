#include "Figure.h"

#include "Routing.h"

#include "data/building.hpp"
#include "data/routes.hpp"
#include "data/figure.hpp"

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
		int figureId = Data_Routes.figureIds[i];
		if (figureId > 0 && figureId < MAX_FIGURES) {
			if (Data_Figures[figureId].state != FigureState_Alive || Data_Figures[figureId].routingPathId != i) {
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

void FigureRoute_add(int figureId)
{
	struct Data_Figure *f = &Data_Figures[figureId];
	f->routingPathId = 0;
	f->routingPathCurrentTile = 0;
	f->routingPathLength = 0;
	int pathId = getFirstAvailable();
	if (!pathId) {
		return;
	}
	int pathLength;
	if (f->isBoat) {
		if (f->isBoat == 2) { // flotsam
			Routing_getDistanceWaterFlotsam(f->x, f->y);
			pathLength = Routing_getPathOnWater(pathId, f->x, f->y,
				f->destinationX, f->destinationY, 1);
		} else {
			Routing_getDistanceWaterBoat(f->x, f->y);
			pathLength = Routing_getPathOnWater(pathId, f->x, f->y,
				f->destinationX, f->destinationY, 0);
		}
	} else {
		// land figure
		int canTravel;
		switch (f->terrainUsage) {
			case FigureTerrainUsage_Enemy:
				canTravel = Routing_canTravelOverLandNonCitizen(f->x, f->y,
					f->destinationX, f->destinationY, f->destinationBuildingId, 5000);
				if (!canTravel) {
					canTravel = Routing_canTravelOverLandNonCitizen(f->x, f->y,
						f->destinationX, f->destinationY, 0, 25000);
					if (!canTravel) {
						canTravel = Routing_canTravelThroughEverythingNonCitizen(
							f->x, f->y, f->destinationX, f->destinationY);
					}
				}
				break;
			case FigureTerrainUsage_Walls:
				canTravel = Routing_canTravelOverWalls(f->x, f->y,
					f->destinationX, f->destinationY);
				break;
			case FigureTerrainUsage_Animal:
				canTravel = Routing_canTravelOverLandNonCitizen(f->x, f->y,
					f->destinationX, f->destinationY, MAX_BUILDINGS, 5000);
				break;
			case FigureTerrainUsage_PreferRoads:
				canTravel = Routing_canTravelOverRoadGardenCitizen(f->x, f->y,
					f->destinationX, f->destinationY);
				if (!canTravel) {
					canTravel = Routing_canTravelOverLandCitizen(f->x, f->y,
						f->destinationX, f->destinationY);
				}
				break;
			case FigureTerrainUsage_Roads:
				canTravel = Routing_canTravelOverRoadGardenCitizen(f->x, f->y,
					f->destinationX, f->destinationY);
				break;
			default:
				canTravel = Routing_canTravelOverLandCitizen(f->x, f->y,
					f->destinationX, f->destinationY);
				break;
		}
		if (canTravel) {
			if (f->terrainUsage == FigureTerrainUsage_Walls) {
				pathLength = Routing_getPath(4, pathId, f->x, f->y,
					f->destinationX, f->destinationY);
				if (pathLength <= 0) {
					pathLength = Routing_getPath(8, pathId, f->x, f->y,
						f->destinationX, f->destinationY);
				}
			} else {
				pathLength = Routing_getPath(8, pathId, f->x, f->y,
					f->destinationX, f->destinationY);
			}
		} else { // cannot travel
			pathLength = 0;
		}
	}
	if (pathLength) {
		Data_Routes.figureIds[pathId] = figureId;
		f->routingPathId = pathId;
		f->routingPathLength = pathLength;
	}
}

void FigureRoute_remove(int figureId)
{
	int path = Data_Figures[figureId].routingPathId;
	if (path > 0) {
		if (Data_Routes.figureIds[path] == figureId) {
			Data_Routes.figureIds[path] = 0;
		}
		Data_Figures[figureId].routingPathId = 0;
	}
}
