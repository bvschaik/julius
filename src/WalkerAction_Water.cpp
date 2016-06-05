#include "WalkerAction_private.h"

#include "Calc.h"
#include "PlayerMessage.h"
#include "Terrain.h"
#include "Walker.h"

#include "Data/CityInfo.h"
#include "Data/Message.h"
#include "Data/Model.h"
#include "Data/Random.h"
#include "Data/Scenario.h"

static const int flotsamType0[] = {0, 1, 2, 3, 4, 4, 4, 3, 2, 1, 0, 0};
static const int flotsamType12[] = {
	0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 3, 2, 1, 0, 0, 1, 1, 2, 2, 1, 1, 0, 0, 0
};
static const int flotsamType3[] = {
	0, 0, 1, 1, 2, 2, 3, 3, 4, 4, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

void WalkerAction_fishingBoat(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	if (!BuildingIsInUse(w->buildingId)) {
		w->state = FigureState_Dead;
	}
	if (w->actionState != FigureActionState_190_FishingBoatCreated && b->data.other.boatWalkerId != walkerId) {
		int xTile, yTile;
		int buildingId = Terrain_Water_getWharfTileForNewFishingBoat(walkerId, &xTile, &yTile);
		b = &Data_Buildings[buildingId];
		if (buildingId) {
			w->buildingId = buildingId;
			b->data.other.boatWalkerId = walkerId;
			w->actionState = FigureActionState_193_FishingBoatSailingToWharf;
			w->destinationX = xTile;
			w->destinationY = yTile;
			w->sourceX = xTile;
			w->sourceY = yTile;
			FigureRoute_remove(walkerId);
		} else {
			w->state = FigureState_Dead;
		}
	}
	w->isGhost = 0;
	w->isBoat = 1;
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	switch (w->actionState) {
		case FigureActionState_190_FishingBoatCreated:
			w->waitTicks++;
			if (w->waitTicks >= 50) {
				w->waitTicks = 0;
				int xTile, yTile;
				int buildingId = Terrain_Water_getWharfTileForNewFishingBoat(walkerId, &xTile, &yTile);
				if (buildingId) {
					b->walkerId = 0; // remove from original building
					w->buildingId = buildingId;
					Data_Buildings[buildingId].data.other.boatWalkerId = walkerId;
					w->actionState = FigureActionState_193_FishingBoatSailingToWharf;
					w->destinationX = xTile;
					w->destinationY = yTile;
					w->sourceX = xTile;
					w->sourceY = yTile;
					FigureRoute_remove(walkerId);
				}
			}
			break;
		case FigureActionState_191_FishingBoatGoingToFish:
			WalkerMovement_walkTicks(walkerId, 1);
			w->heightAdjustedTicks = 0;
			if (w->direction == DirFigure_8_AtDestination) {
				int xTile, yTile;
				if (Terrain_Water_findAlternativeTileForFishingBoat(walkerId, &xTile, &yTile)) {
					FigureRoute_remove(walkerId);
					w->destinationX = xTile;
					w->destinationY = yTile;
					w->direction = w->previousTileDirection;
				} else {
					w->actionState = FigureActionState_192_FishingBoatFishing;
					w->waitTicks = 0;
				}
			} else if (w->direction == DirFigure_9_Reroute || w->direction == DirFigure_10_Lost) {
				w->actionState = FigureActionState_194_FishingBoatAtWharf;
				w->destinationX = w->sourceX;
				w->destinationY = w->sourceY;
			}
			break;
		case FigureActionState_192_FishingBoatFishing:
			w->waitTicks++;
			if (w->waitTicks >= 200) {
				w->waitTicks = 0;
				w->actionState = FigureActionState_195_FishingBoatReturningWithFish;
				w->destinationX = w->sourceX;
				w->destinationY = w->sourceY;
				FigureRoute_remove(walkerId);
			}
			break;
		case FigureActionState_193_FishingBoatSailingToWharf:
			WalkerMovement_walkTicks(walkerId, 1);
			w->heightAdjustedTicks = 0;
			if (w->direction == DirFigure_8_AtDestination) {
				w->actionState = FigureActionState_194_FishingBoatAtWharf;
				w->waitTicks = 0;
			} else if (w->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
			} else if (w->direction == DirFigure_10_Lost) {
				// cannot reach grounds
				if (Data_Message.messageCategoryCount[MessageDelay_FishingBlocked] > 0) {
					Data_Message.messageCategoryCount[MessageDelay_FishingBlocked]--;
				} else {
					PlayerMessage_post(1, Message_118_FishingBoatBlocked, 0, 0);
					Data_Message.messageCategoryCount[MessageDelay_FishingBlocked] = 12;
				}
				w->state = FigureState_Dead;
			}
			break;
		case FigureActionState_194_FishingBoatAtWharf:
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
						w->actionState = FigureActionState_191_FishingBoatGoingToFish;
						w->destinationX = xTile;
						w->destinationY = yTile;
						FigureRoute_remove(walkerId);
					}
				}
			}
			}
			break;
		case FigureActionState_195_FishingBoatReturningWithFish:
			WalkerMovement_walkTicks(walkerId, 1);
			w->heightAdjustedTicks = 0;
			if (w->direction == DirFigure_8_AtDestination) {
				w->actionState = FigureActionState_194_FishingBoatAtWharf;
				w->waitTicks = 0;
				b->walkerSpawnDelay = 1;
				b->data.other.fishingBoatHasFish++;
			} else if (w->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
			} else if (w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			}
			break;
	}
	int dir = (w->direction < 8) ? w->direction : w->previousTileDirection;
	WalkerActionNormalizeDirection(dir);
	
	if (w->actionState == FigureActionState_192_FishingBoatFishing) {
		w->graphicId = GraphicId(ID_Graphic_Figure_Ship) + dir + 16;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Figure_Ship) + dir + 8;
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
	w->terrainUsage = FigureTerrainUsage_Any;
	switch (w->actionState) {
		case FigureActionState_128_FlotsamCreated:
			w->isGhost = 1;
			w->waitTicks--;
			if (w->waitTicks <= 0) {
				w->actionState = FigureActionState_129_FlotsamFloating;
				w->waitTicks = 0;
				if (Data_CityInfo.godCurseNeptuneSankShips && !w->resourceId) {
					w->minMaxSeen = 1;
					Data_CityInfo.godCurseNeptuneSankShips = 0;
				}
				w->destinationX = Data_Scenario.riverExitPoint.x;
				w->destinationY = Data_Scenario.riverExitPoint.y;
			}
			break;
		case FigureActionState_129_FlotsamFloating:
			if (w->flotsamVisible) {
				w->flotsamVisible = 0;
			} else {
				w->flotsamVisible = 1;
				w->waitTicks++;
				WalkerMovement_walkTicks(walkerId, 1);
				w->isGhost = 0;
				w->heightAdjustedTicks = 0;
				if (w->direction == DirFigure_8_AtDestination ||
					w->direction == DirFigure_9_Reroute || w->direction == DirFigure_10_Lost) {
					w->actionState = FigureActionState_130_FlotsamLeftMap;
				}
			}
			break;
		case FigureActionState_130_FlotsamLeftMap:
			w->isGhost = 1;
			w->minMaxSeen = 0;
			w->actionState = FigureActionState_128_FlotsamCreated;
			if (w->waitTicks >= 400) {
				w->waitTicks = Data_Random.random1_7bit & 7;
			} else if (w->waitTicks >= 200) {
				w->waitTicks = 50 + (Data_Random.random1_7bit & 0xf);
			} else if (w->waitTicks >= 100) {
				w->waitTicks = 100 + (Data_Random.random1_7bit & 0x1f);
			} else if (w->waitTicks >= 50) {
				w->waitTicks = 200 + (Data_Random.random1_7bit & 0x3f);
			} else {
				w->waitTicks = 300 + Data_Random.random1_7bit;
			}
			Figure_removeFromTileList(walkerId);
			w->x = Data_Scenario.riverEntryPoint.x;
			w->y = Data_Scenario.riverEntryPoint.y;
			w->gridOffset = GridOffset(w->x, w->y);
			w->crossCountryX = 15 * w->x;
			w->crossCountryY = 15 * w->y;
			break;
	}
	if (w->resourceId == 0) {
		WalkerActionIncreaseGraphicOffset(w, 12);
		if (w->minMaxSeen) {
			w->graphicId = GraphicId(ID_Graphic_Figure_FlotsamSheep) +
				flotsamType0[w->graphicOffset];
		} else {
			w->graphicId = GraphicId(ID_Graphic_Figure_Flotsam0) +
				flotsamType0[w->graphicOffset];
		}
	} else if (w->resourceId == 1) {
		WalkerActionIncreaseGraphicOffset(w, 24);
		w->graphicId = GraphicId(ID_Graphic_Figure_Flotsam1) +
			flotsamType12[w->graphicOffset];
	} else if (w->resourceId == 2) {
		WalkerActionIncreaseGraphicOffset(w, 24);
		w->graphicId = GraphicId(ID_Graphic_Figure_Flotsam2) +
			flotsamType12[w->graphicOffset];
	} else if (w->resourceId == 3) {
		WalkerActionIncreaseGraphicOffset(w, 24);
		if (flotsamType3[w->graphicOffset] == -1) {
			w->graphicId = 0;
		} else {
			w->graphicId = GraphicId(ID_Graphic_Figure_Flotsam3) +
				flotsamType3[w->graphicOffset];
		}
	}
}

void WalkerAction_shipwreck(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->isGhost = 0;
	w->heightAdjustedTicks = 0;
	w->isBoat = 1;
	WalkerActionIncreaseGraphicOffset(w, 128);
	if (w->waitTicks < 1000) {
		Figure_removeFromTileList(walkerId);
		int xTile, yTile;
		if (Terrain_Water_findOpenWaterForShipwreck(walkerId, &xTile, &yTile)) {
			w->x = xTile;
			w->y = yTile;
			w->gridOffset = GridOffset(w->x, w->y);
			w->crossCountryX = 15 * w->x + 7;
			w->crossCountryY = 15 * w->y + 7;
		}
		Figure_addToTileList(walkerId);
		w->waitTicks = 1000;
	}
	w->waitTicks++;
	if (w->waitTicks > 2000) {
		w->state = FigureState_Dead;
	}
	w->graphicId = GraphicId(ID_Graphic_Figure_Shipwreck) + w->graphicOffset / 16;
}
