#include "WalkerAction_private.h"

#include "Terrain.h"
#include "Data/Grid.h"

void WalkerAction_taxCollector(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	
	w->terrainUsage = 1;
	w->useCrossCountry = 0;
	w->maxRoamLength = 512;
	if (b->inUse != 1 || b->walkerId != walkerId) {
		w->state = WalkerState_Dead;
	}
	WalkerActionIncreaseGraphicOffset(w, 12);
	
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_40_TaxCollectorCreated:
			w->isGhost = 1;
			w->graphicOffset = 0;
			w->waitTicks--;
			if (w->waitTicks <= 0) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					w->actionState = WalkerActionState_41_TaxCollectorEnteringExiting;
					WalkerAction_Common_setCrossCountryDestination(walkerId, w, xRoad, yRoad);
					w->roamLength = 0;
				} else {
					w->state = WalkerState_Dead;
				}
			}
			break;
		case WalkerActionState_41_TaxCollectorEnteringExiting:
			w->useCrossCountry = 1;
			w->isGhost = 1;
			if (WalkerMovement_crossCountryWalkTicks(walkerId, 1) == 1) {
				if (Data_Grid_buildingIds[w->gridOffset] == w->buildingId) {
					// returned to own building
					w->state = WalkerState_Dead;
				} else {
					w->actionState = WalkerActionState_42_TaxCollectorRoaming;
					WalkerMovement_initRoaming(walkerId);
					w->roamLength = 0;
				}
			}
			break;
		case WalkerActionState_42_TaxCollectorRoaming:
			w->isGhost = 0;
			w->roamLength++;
			if (w->roamLength >= w->maxRoamLength) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					w->actionState = WalkerActionState_43_TaxCollectorReturning;
					w->destinationX = xRoad;
					w->destinationY = yRoad;
				} else {
					w->state = WalkerState_Dead;
				}
			}
			WalkerMovement_roamTicks(walkerId, 1);
			break;
		case WalkerActionState_43_TaxCollectorReturning:
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == 8) {
				w->actionState = WalkerActionState_41_TaxCollectorEnteringExiting;
				WalkerAction_Common_setCrossCountryDestination(walkerId, w, b->x, b->y);
				w->roamLength = 0;
			} else if (w->direction == 9 || w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
	}
	WalkerActionUpdateGraphic(w, GraphicId(ID_Graphic_Walker_TaxCollector));
}

void WalkerAction_engineer(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	
	w->terrainUsage = 1;
	w->useCrossCountry = 0;
	w->maxRoamLength = 640;
	if (b->inUse != 1 || b->walkerId != walkerId) {
		w->state = WalkerState_Dead;
	}
	WalkerActionIncreaseGraphicOffset(w, 12);
	
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_60_EngineerCreated:
			w->isGhost = 1;
			w->graphicOffset = 0;
			w->waitTicks--;
			if (w->waitTicks <= 0) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					w->actionState = WalkerActionState_61_EngineerEnteringExiting;
					WalkerAction_Common_setCrossCountryDestination(walkerId, w, xRoad, yRoad);
					w->roamLength = 0;
				} else {
					w->state = WalkerState_Dead;
				}
			}
			break;
		case WalkerActionState_61_EngineerEnteringExiting:
			w->useCrossCountry = 1;
			w->isGhost = 1;
			if (WalkerMovement_crossCountryWalkTicks(walkerId, 1) == 1) {
				if (Data_Grid_buildingIds[w->gridOffset] == w->buildingId) {
					// returned to own building
					w->state = WalkerState_Dead;
				} else {
					w->actionState = WalkerActionState_62_EngineerRoaming;
					WalkerMovement_initRoaming(walkerId);
					w->roamLength = 0;
				}
			}
			break;
		case WalkerActionState_62_EngineerRoaming:
			w->isGhost = 0;
			w->roamLength++;
			if (w->roamLength >= w->maxRoamLength) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					w->actionState = WalkerActionState_63_EngineerReturning;
					w->destinationX = xRoad;
					w->destinationY = yRoad;
				} else {
					w->state = WalkerState_Dead;
				}
			}
			WalkerMovement_roamTicks(walkerId, 1);
			break;
		case WalkerActionState_63_EngineerReturning:
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == 8) {
				w->actionState = WalkerActionState_61_EngineerEnteringExiting;
				WalkerAction_Common_setCrossCountryDestination(walkerId, w, b->x, b->y);
				w->roamLength = 0;
			} else if (w->direction == 9 || w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
	}
	WalkerActionUpdateGraphic(w, GraphicId(ID_Graphic_Walker_Engineer));
}
