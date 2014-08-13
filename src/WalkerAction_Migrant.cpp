#include "WalkerAction_private.h"

#include "BuildingHouse.h"
#include "CityInfo.h"
#include "HousePopulation.h"
#include "Terrain.h"
#include "Walker.h"

#include "Data/CityInfo.h"
#include "Data/Model.h"

static void updateDirectionAndGraphic(int walkerId, struct Data_Walker *w)
{
	int dir = WalkerActionDirection(w);
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Migrant) +
			WalkerActionCorpseGraphicOffset(w) + 96;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_Migrant) +
			dir + 8 * w->graphicOffset;
	}
	if (w->actionState == WalkerActionState_2_ImmigrantArriving) {
		w->cartGraphicId = GraphicId(ID_Graphic_Walker_MigrantCart) + dir;
		int cartDir = (dir + 4) % 8;
		WalkerAction_Common_setCartOffset(walkerId, cartDir);
	}
}

void WalkerAction_immigrant(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	int buildingId = w->immigrantBuildingId;
	struct Data_Building *b = &Data_Buildings[buildingId];
	
	w->terrainUsage = 0;
	w->cartGraphicId = 0;
	if (b->inUse != 1 || b->immigrantWalkerId != walkerId || !b->houseSize) {
		w->state = WalkerState_Dead;
		return;
	}
	
	WalkerActionIncreaseGraphicOffset(w, 12);
	
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_1_ImmigrantCreated:
			w->isGhost = 1;
			w->graphicOffset = 0;
			w->waitTicks--;
			if (w->waitTicks <= 0) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					w->actionState = WalkerActionState_2_ImmigrantArriving;
					w->destinationX = xRoad;
					w->destinationY = yRoad;
					w->roamLength = 0;
				} else {
					w->state = WalkerState_Dead;
				}
			}
			break;
		case WalkerActionState_2_ImmigrantArriving:
			w->isGhost = 0;
			WalkerMovement_walkTicks(walkerId, 1);
			switch (w->direction) {
				case 9:
					WalkerRoute_remove(walkerId);
					break;
				case 10:
					b->immigrantWalkerId = 0;
					b->distanceFromEntry = 0;
					w->state = WalkerState_Dead;
					break;
				case Direction_None:
					w->actionState = WalkerActionState_3_ImmigrantEnteringHouse;
					w->destinationX = b->x;
					w->destinationY = b->y;
					WalkerMovement_crossCountrySetDirection(walkerId,
						w->crossCountryX, w->crossCountryY,
						15 * w->destinationX, 15 * w->destinationY, 0);
					w->roamLength = 0;
					break;
			}
			break;
		case WalkerActionState_3_ImmigrantEnteringHouse:
			w->useCrossCountry = 1;
			w->isGhost = 1;
			if (WalkerMovement_crossCountryWalkTicks(walkerId, 1) == 1) { // WalkerMovement_moveTickDirectly?
				w->state = WalkerState_Dead;
				int maxPeople = Data_Model_Houses[b->subtype.houseLevel].maxPeople;
				if (b->houseIsMerged) {
					maxPeople *= 4;
				}
				int room = maxPeople - b->housePopulation;
				if (room < 0) {
					room = 0;
				}
				if (room < w->migrantNumPeople) {
					w->migrantNumPeople = room;
				}
				if (!b->housePopulation) {
					BuildingHouse_changeTo(buildingId, Building_HouseSmallTent);
				}
				b->housePopulation += w->migrantNumPeople;
				b->housePopulationRoom = maxPeople - b->housePopulation;
				CityInfo_Population_addPeople(w->migrantNumPeople);
				b->immigrantWalkerId = 0;
			}
			w->isGhost = w->inBuildingWaitTicks ? 1 : 0;
			break;
	}
	
	updateDirectionAndGraphic(walkerId, w);
}

void WalkerAction_emigrant(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	
	w->terrainUsage = 0;
	w->cartGraphicId = 0;
	
	WalkerActionIncreaseGraphicOffset(w, 12);
	
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_4_EmigrantCreated:
			w->isGhost = 1;
			w->graphicOffset = 0;
			w->waitTicks++;
			if (w->waitTicks >= 5) {
				int xRoad, yRoad;
				if (!Terrain_getClosestRoadWithinRadius(w->x, w->y, 1, 5, &xRoad, &yRoad)) {
					w->state = WalkerState_Dead;
				}
				w->actionState = WalkerActionState_5_EmigrantExitingHouse;
				w->destinationX = xRoad;
				w->destinationY = yRoad;
				WalkerMovement_crossCountrySetDirection(walkerId,
					w->crossCountryX, w->crossCountryY,
					15 * w->destinationX, 15 * w->destinationY, 0);
				w->roamLength = 0;
			}
			break;
		case WalkerActionState_5_EmigrantExitingHouse:
			w->useCrossCountry = 1;
			w->isGhost = 1;
			if (WalkerMovement_crossCountryWalkTicks(walkerId, 1) == 1) {
				w->actionState = WalkerActionState_6_EmigrantLeaving;
				w->destinationX = Data_CityInfo.entryPointX;
				w->destinationY = Data_CityInfo.entryPointY;
				w->roamLength = 0;
				w->progressOnTile = 15;
			}
			w->isGhost = w->inBuildingWaitTicks ? 1 : 0;
			break;
		case WalkerActionState_6_EmigrantLeaving:
			w->useCrossCountry = 0;
			w->isGhost = 0;
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == 8 || w->direction == 9 || w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
	}
	updateDirectionAndGraphic(walkerId, w);
}

void WalkerAction_homeless(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->terrainUsage = 3;
	
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_7_HomelessCreated:
			w->graphicOffset = 0;
			w->waitTicks++;
			if (w->waitTicks > 51) {
				int buildingId = HousePopulation_getClosestHouseWithRoom(w->x, w->y);
				if (buildingId) {
					struct Data_Building *b = &Data_Buildings[buildingId];
					int xRoad, yRoad;
					if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
						b->immigrantWalkerId = walkerId;
						w->immigrantBuildingId = buildingId;
						w->actionState = WalkerActionState_8_HomelessGoingToHouse;
						w->destinationX = xRoad;
						w->destinationY = yRoad;
						w->roamLength = 0;
					} else {
						w->state = WalkerState_Dead;
					}
				} else {
					w->actionState = WalkerActionState_10_HomelessLeaving;
					w->destinationX = Data_CityInfo.exitPointX;
					w->destinationY = Data_CityInfo.exitPointY;
					w->roamLength = 0;
					w->waitTicks = 0;
				}
			}
			break;
		case WalkerActionState_8_HomelessGoingToHouse:
			w->isGhost = 0;
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == 9 || w->direction == 10) {
				Data_Buildings[w->immigrantBuildingId].immigrantWalkerId = 0;
				w->state = WalkerState_Dead;
			} else if (w->direction == Direction_None) {
				w->actionState = WalkerActionState_9_HomelessEnteringHouse;
				w->destinationX = Data_Buildings[w->immigrantBuildingId].x;
				w->destinationY = Data_Buildings[w->immigrantBuildingId].y;
				WalkerMovement_crossCountrySetDirection(walkerId,
					w->crossCountryX, w->crossCountryY,
					15 * w->destinationX, 15 * w->destinationY, 0);
				w->roamLength = 0;
			}
			break;
		case WalkerActionState_9_HomelessEnteringHouse:
			w->useCrossCountry = 1;
			w->isGhost = 1;
			if (WalkerMovement_crossCountryWalkTicks(walkerId, 1) == 1) {
				w->state = WalkerState_Dead;
				struct Data_Building *b = &Data_Buildings[w->immigrantBuildingId];
				if (w->immigrantBuildingId && BuildingIsHouse(b->type)) {
					int maxPeople = Data_Model_Houses[b->subtype.houseLevel].maxPeople;
					if (b->houseIsMerged) {
						maxPeople *= 4;
					}
					int room = maxPeople - b->housePopulation;
					if (room < 0) {
						room = 0;
					}
					if (room < w->migrantNumPeople) {
						w->migrantNumPeople = room;
					}
					if (!b->housePopulation) {
						BuildingHouse_changeTo(w->immigrantBuildingId, Building_HouseSmallTent);
					}
					b->housePopulation += w->migrantNumPeople;
					b->housePopulationRoom = maxPeople - b->housePopulation;
					CityInfo_Population_addPeopleHomeless(w->migrantNumPeople);
					b->immigrantWalkerId = 0;
				}
			}
			break;
		case WalkerActionState_10_HomelessLeaving:
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == 8 || w->direction == 10) {
				w->state = WalkerState_Dead;
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			}
			w->waitTicks++;
			if (w->waitTicks > 30) {
				w->waitTicks = 0;
				int buildingId = HousePopulation_getClosestHouseWithRoom(w->x, w->y);
				if (buildingId > 0) {
					struct Data_Building *b = &Data_Buildings[buildingId];
					int xRoad, yRoad;
					if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
						b->immigrantWalkerId = walkerId;
						w->immigrantBuildingId = buildingId;
						w->actionState = WalkerActionState_8_HomelessGoingToHouse;
						w->destinationX = xRoad;
						w->destinationY = yRoad;
						w->roamLength = 0;
						WalkerRoute_remove(walkerId);
					}
				}
			}
			break;
	}
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Homeless) +
			WalkerActionCorpseGraphicOffset(w) + 96;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_Homeless) +
			WalkerActionDirection(w) + 8 * w->graphicOffset;
	}
}
