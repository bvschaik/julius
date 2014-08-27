#include "WalkerAction_private.h"

#include "Calc.h"
#include "PlayerMessage.h"
#include "Terrain.h"
#include "Walker.h"

#include "Data/Message.h"
#include "Data/Model.h"
#include "Data/Scenario.h"

void WalkerAction_fishingBoat(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	if (b->inUse != 1) {
		w->state = WalkerState_Dead;
	}
	if (w->actionState != WalkerActionState_190_FishingBoatCreated && b->data.other.boatWalkerId != walkerId) {
		int xTile, yTile;
		int buildingId = Terrain_Water_getWharfTileForNewFishingBoat(walkerId, &xTile, &yTile);
		if (buildingId) {
			w->buildingId = buildingId;
			b = &Data_Buildings[buildingId];
			b->data.other.boatWalkerId = walkerId;
			w->actionState = WalkerActionState_193_FishingBoatSailingToWharf;
			w->destinationX = xTile;
			w->destinationY = yTile;
			w->sourceX = xTile;
			w->sourceY = yTile;
			WalkerRoute_remove(walkerId);
		} else {
			w->state = WalkerState_Dead;
		}
	}
	w->isGhost = 0;
	w->isBoat = 1;
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	switch (w->actionState) {
		case WalkerActionState_190_FishingBoatCreated:
			w->waitTicks++;
			if (w->waitTicks >= 50) {
				w->waitTicks = 0;
				int xTile, yTile;
				int buildingId = Terrain_Water_getWharfTileForNewFishingBoat(walkerId, &xTile, &yTile);
				if (buildingId) {
					b->walkerId = 0;
					w->buildingId = buildingId;
					Data_Buildings[buildingId].data.other.boatWalkerId = walkerId;
					w->actionState = WalkerActionState_193_FishingBoatSailingToWharf;
					w->destinationX = xTile;
					w->destinationY = yTile;
					w->sourceX = xTile;
					w->sourceY = yTile;
					WalkerRoute_remove(walkerId);
				}
			}
			break;
		case WalkerActionState_191_FishingBoatGoingToFish:
			WalkerMovement_walkTicks(walkerId, 1);
			w->heightFromGround = 0;
			if (w->direction == 8) {
				int xTile, yTile;
				if (Terrain_Water_findAlternativeTileForFishingBoat(walkerId, &xTile, &yTile)) {
					WalkerRoute_remove(walkerId);
					w->destinationX = xTile;
					w->destinationY = yTile;
					w->direction = w->previousTileDirection;
				} else {
					w->actionState = WalkerActionState_192_FishingBoatFishing;
					w->waitTicks = 0;
				}
			} else if (w->direction == 9 || w->direction == 10) {
				w->actionState = WalkerActionState_194_FishingBoatAtWharf;
				w->destinationX = w->sourceX;
				w->destinationY = w->sourceY;
			}
			break;
		case WalkerActionState_192_FishingBoatFishing:
			w->waitTicks++;
			if (w->waitTicks >= 200) {
				w->waitTicks = 0;
				w->actionState = WalkerActionState_195_FishingBoatReturningWithFish;
				w->destinationX = w->sourceX;
				w->destinationY = w->sourceY;
				WalkerRoute_remove(walkerId);
			}
			break;
		case WalkerActionState_193_FishingBoatSailingToWharf:
			WalkerMovement_walkTicks(walkerId, 1);
			w->heightFromGround = 0;
			if (w->direction == 8) {
				w->actionState = WalkerActionState_194_FishingBoatAtWharf;
				w->waitTicks = 0;
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == 10) {
				// cannot reach grounds
				if (Data_Message.messageCategoryCount[MessageDelay_FishingBlocked] > 0) {
					Data_Message.messageCategoryCount[MessageDelay_FishingBlocked]--;
				} else {
					PlayerMessage_post(1, 118, 0, 0);
					Data_Message.messageCategoryCount[MessageDelay_FishingBlocked] = 12;
				}
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_194_FishingBoatAtWharf:
			{
			int pctWorkers = Calc_getPercentage(b->numWorkers, Data_Model_Buildings[b->type].laborers);
			int maxWaitTicks = 5 * (102 - pctWorkers);
			if (b->data.other.fishingBoatHasFish > 0) {
				pctWorkers = 0;
			}
			if (pctWorkers > 0) {
				w->waitTicks++;
				if (w->waitTicks >= maxWaitTicks) {
					w->waitTicks = 0;
					int xTile, yTile;
					if (Terrain_Water_getNearestFishTile(walkerId, &xTile, &yTile)) {
						w->actionState = WalkerActionState_191_FishingBoatGoingToFish;
						w->destinationX = xTile;
						w->destinationY = yTile;
						WalkerRoute_remove(walkerId);
					}
				}
			}
			}
			break;
		case WalkerActionState_195_FishingBoatReturningWithFish:
			WalkerMovement_walkTicks(walkerId, 1);
			w->heightFromGround = 0;
			if (w->direction == 8) {
				w->actionState = WalkerActionState_194_FishingBoatAtWharf;
				w->waitTicks = 0;
				b->walkerSpawnDelay = 1;
				b->data.other.fishingBoatHasFish++;
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
	}
	int dir = (w->direction < 8) ? w->direction : w->previousTileDirection;
	dir = (8 + dir - Data_Settings_Map.orientation) % 8;
	
	if (w->actionState == WalkerActionState_192_FishingBoatFishing) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Ship) + dir + 16;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_Ship) + dir + 8;
	}
}

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
