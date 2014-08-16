#include "WalkerMovement.h"

#include "Terrain.h"

#include "Data/Building.h"
#include "Data/Settings.h"
#include "Data/Walker.h"

void WalkerMovement_initRoaming(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	w->progressOnTile = 15;
	w->roamNoDestination = 0;
	w->roamTicksUntilNextTurn = -1;
	w->roamTurnDirection = 2;
	int roamDir = b->walkerRoamDirection;
	b->walkerRoamDirection += 2;
	if (b->walkerRoamDirection > 6) {
		b->walkerRoamDirection = 0;
	}
	int x = b->x;
	int y = b->y;
	switch (roamDir) {
		case 0: y -= 8; break;
		case 2: x += 8; break;
		case 4: y += 8; break;
		case 6: x -= 8; break;
	}
	BoundToMap(x, y);
	int xRoad, yRoad;
	if (Terrain_getClosestRoadWithinRadius(x, y, 1, 6, &xRoad, &yRoad)) {
		w->destinationX = xRoad;
		w->destinationY = yRoad;
	} else {
		w->roamNoDestination = 1;
	}
}

void WalkerMovement_roamTicks(int walkerId, int numTicks)
{
	// TODO
}

void WalkerMovement_walkTicks(int walkerId, int numTicks)
{
	// TODO
}

void WalkerMovement_followTicks(int walkerId, int leaderWalkerId, int numTicks)
{
	// TODO
}

void WalkerMovement_crossCountrySetDirection(int walkerId, int xSrc, int ySrc, int xDst, int yDst, int isProjectile)
{
	// TODO
}

int WalkerMovement_crossCountryWalkTicks(int walkerId, int numTicks)
{
	// TODO
	return 0;
}

