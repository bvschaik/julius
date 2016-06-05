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
	if (w->actionState == FigureActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Figure_Migrant) +
			WalkerActionCorpseGraphicOffset(w) + 96;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Figure_Migrant) +
			dir + 8 * w->graphicOffset;
	}
	if (w->actionState == FigureActionState_2_ImmigrantArriving) {
		w->cartGraphicId = GraphicId(ID_Graphic_Figure_MigrantCart) + dir;
		int cartDir = (dir + 4) % 8;
		WalkerAction_Common_setCartOffset(walkerId, cartDir);
	}
}

void WalkerAction_immigrant(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	int buildingId = w->immigrantBuildingId;
	struct Data_Building *b = &Data_Buildings[buildingId];
	
	w->terrainUsage = FigureTerrainUsage_Any;
	w->cartGraphicId = 0;
	if (!BuildingIsInUse(buildingId) || b->immigrantWalkerId != walkerId || !b->houseSize) {
		w->state = FigureState_Dead;
		return;
	}
	
	WalkerActionIncreaseGraphicOffset(w, 12);
	
	switch (w->actionState) {
		case FigureActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_1_ImmigrantCreated:
			w->isGhost = 1;
			w->graphicOffset = 0;
			w->waitTicks--;
			if (w->waitTicks <= 0) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					w->actionState = FigureActionState_2_ImmigrantArriving;
					w->destinationX = xRoad;
					w->destinationY = yRoad;
					w->roamLength = 0;
				} else {
					w->state = FigureState_Dead;
				}
			}
			break;
		case FigureActionState_2_ImmigrantArriving:
			w->isGhost = 0;
			WalkerMovement_walkTicks(walkerId, 1);
			switch (w->direction) {
				case DirFigure_8_AtDestination:
					w->actionState = FigureActionState_3_ImmigrantEnteringHouse;
					WalkerAction_Common_setCrossCountryDestination(walkerId, w, b->x, b->y);
					w->roamLength = 0;
					break;
				case DirFigure_9_Reroute:
					FigureRoute_remove(walkerId);
					break;
				case DirFigure_10_Lost:
					b->immigrantWalkerId = 0;
					b->distanceFromEntry = 0;
					w->state = FigureState_Dead;
					break;
			}
			break;
		case FigureActionState_3_ImmigrantEnteringHouse:
			w->useCrossCountry = 1;
			w->isGhost = 1;
			if (WalkerMovement_crossCountryWalkTicks(walkerId, 1) == 1) {
				w->state = FigureState_Dead;
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
	
	w->terrainUsage = FigureTerrainUsage_Any;
	w->cartGraphicId = 0;
	
	WalkerActionIncreaseGraphicOffset(w, 12);
	
	switch (w->actionState) {
		case FigureActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_4_EmigrantCreated:
			w->isGhost = 1;
			w->graphicOffset = 0;
			w->waitTicks++;
			if (w->waitTicks >= 5) {
				int xRoad, yRoad;
				if (!Terrain_getClosestRoadWithinRadius(w->x, w->y, 1, 5, &xRoad, &yRoad)) {
					w->state = FigureState_Dead;
				}
				w->actionState = FigureActionState_5_EmigrantExitingHouse;
				WalkerAction_Common_setCrossCountryDestination(walkerId, w, xRoad, yRoad);
				w->roamLength = 0;
			}
			break;
		case FigureActionState_5_EmigrantExitingHouse:
			w->useCrossCountry = 1;
			w->isGhost = 1;
			if (WalkerMovement_crossCountryWalkTicks(walkerId, 1) == 1) {
				w->actionState = FigureActionState_6_EmigrantLeaving;
				w->destinationX = Data_CityInfo.entryPointX;
				w->destinationY = Data_CityInfo.entryPointY;
				w->roamLength = 0;
				w->progressOnTile = 15;
			}
			w->isGhost = w->inBuildingWaitTicks ? 1 : 0;
			break;
		case FigureActionState_6_EmigrantLeaving:
			w->useCrossCountry = 0;
			w->isGhost = 0;
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirFigure_8_AtDestination ||
				w->direction == DirFigure_9_Reroute ||
				w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			}
			break;
	}
	updateDirectionAndGraphic(walkerId, w);
}

void WalkerAction_homeless(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->terrainUsage = FigureTerrainUsage_PreferRoads;
	
	switch (w->actionState) {
		case FigureActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_7_HomelessCreated:
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
						w->actionState = FigureActionState_8_HomelessGoingToHouse;
						w->destinationX = xRoad;
						w->destinationY = yRoad;
						w->roamLength = 0;
					} else {
						w->state = FigureState_Dead;
					}
				} else {
					w->actionState = FigureActionState_10_HomelessLeaving;
					w->destinationX = Data_CityInfo.exitPointX;
					w->destinationY = Data_CityInfo.exitPointY;
					w->roamLength = 0;
					w->waitTicks = 0;
				}
			}
			break;
		case FigureActionState_8_HomelessGoingToHouse:
			w->isGhost = 0;
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirFigure_9_Reroute || w->direction == DirFigure_10_Lost) {
				Data_Buildings[w->immigrantBuildingId].immigrantWalkerId = 0;
				w->state = FigureState_Dead;
			} else if (w->direction == DirFigure_8_AtDestination) {
				w->actionState = FigureActionState_9_HomelessEnteringHouse;
				WalkerAction_Common_setCrossCountryDestination(walkerId, w,
					Data_Buildings[w->immigrantBuildingId].x,
					Data_Buildings[w->immigrantBuildingId].y);
				w->roamLength = 0;
			}
			break;
		case FigureActionState_9_HomelessEnteringHouse:
			w->useCrossCountry = 1;
			w->isGhost = 1;
			if (WalkerMovement_crossCountryWalkTicks(walkerId, 1) == 1) {
				w->state = FigureState_Dead;
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
		case FigureActionState_10_HomelessLeaving:
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirFigure_8_AtDestination || w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			} else if (w->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
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
						w->actionState = FigureActionState_8_HomelessGoingToHouse;
						w->destinationX = xRoad;
						w->destinationY = yRoad;
						w->roamLength = 0;
						FigureRoute_remove(walkerId);
					}
				}
			}
			break;
	}
	if (w->actionState == FigureActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Figure_Homeless) +
			WalkerActionCorpseGraphicOffset(w) + 96;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Figure_Homeless) +
			WalkerActionDirection(w) + 8 * w->graphicOffset;
	}
}
