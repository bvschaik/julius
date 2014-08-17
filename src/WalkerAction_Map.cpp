#include "WalkerAction_private.h"

#include "Terrain.h"
#include "Walker.h"

#include "Data/Scenario.h"

void WalkerAction_flotsam(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->isBoat = 2;
	if (Data_Scenario.riverExitPoint.x == -1 || Data_Scenario.riverExitPoint.y == -1) {
		return;
	}
	w->isGhost = 0;
	w->cartGraphicId = 0;
	w->terrainUsage = 0;
	// TODO
}

void WalkerAction_shipwreck(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->isGhost = 0;
	w->heightFromGround = 0;
	w->isBoat = 1;
	WalkerActionIncreaseGraphicOffset(w, 128);
	if (w->waitTicks < 1000) {
		Walker_removeFromTileList(walkerId);
		int xTile, yTile;
		if (Terrain_Water_findOpenWaterForShipwreck(walkerId, &xTile, &yTile)) {
			w->x = xTile;
			w->y = yTile;
			w->gridOffset = GridOffset(w->x, w->y);
			w->crossCountryX = 15 * w->x + 7;
			w->crossCountryY = 15 * w->y + 7;
		}
		Walker_addToTileList(walkerId);
		w->waitTicks = 1000;
	}
	w->waitTicks++;
	if (w->waitTicks > 2000) {
		w->state = WalkerState_Dead;
	}
	w->graphicId = GraphicId(ID_Graphic_Walker_Shipwreck) + w->graphicOffset / 16;
}
