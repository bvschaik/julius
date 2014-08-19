#include "WalkerAction_private.h"

#include "Calc.h"
#include "Routing.h"
#include "Security.h"
#include "Sound.h"
#include "Terrain.h"
#include "Walker.h"

#include "Data/CityInfo.h"
#include "Data/Formation.h"
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

static int prefectGetNearestEnemy(int x, int y, int *distance)
{
	int minEnemyId = 0;
	int minDist = 10000;
	for (int i = 1; i < MAX_WALKERS; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state != WalkerState_Alive || w->targetedByWalkerId) {
			continue;
		}
		int dist;
		if (w->type == Walker_Rioter) {
			dist = Calc_distanceMaximum(x, y, w->x, w->y);
		} else if (w->type == Walker_Enemy54_Gladiator) {
			dist = Calc_distanceMaximum(x, y, w->x, w->y);
		} else if (w->type == Walker_IndigenousNative && w->actionState == WalkerActionState_159_AttackingNative) {
			dist = Calc_distanceMaximum(x, y, w->x, w->y);
		} else if (WalkerIsEnemy(w->type)) {
			dist = 3 * Calc_distanceMaximum(x, y, w->x, w->y);
		} else if (w->type == Walker_Wolf) {
			dist = 4 * Calc_distanceMaximum(x, y, w->x, w->y);
		} else {
			continue;
		}
		if (dist < minDist) {
			minDist = dist;
			minEnemyId = i;
		}
	}
	*distance = minDist;
	return minEnemyId;
}

static int prefectGoFightEnemy(int walkerId, struct Data_Walker *w)
{
	if (Data_CityInfo.riotersOrAttackingNativesInCity <= 0 && Data_Formation_Extra.numEnemyFormations <= 0) {
		return 0;
	}
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
		case WalkerActionState_149_Corpse:
		case WalkerActionState_70_PrefectCreated:
		case WalkerActionState_71_PrefectEnteringExiting:
		case WalkerActionState_74_PrefectGoingToFire:
		case WalkerActionState_75_PrefectAtFire:
		case WalkerActionState_76_PrefectGoingToEnemy:
		case WalkerActionState_77_PrefectAtEnemy:
			return 0;
	}
	w->waitTicksNextTarget++;
	if (w->waitTicksNextTarget < 10) {
		return 0;
	}
	w->waitTicksNextTarget = 0;
	int distance;
	int enemyId = prefectGetNearestEnemy(w->x, w->y, &distance);
	if (enemyId > 0 && distance <= 30) {
		w->waitTicksNextTarget = 0;
		w->actionState = WalkerActionState_76_PrefectGoingToEnemy;
		w->destinationX = Data_Walkers[enemyId].x;
		w->destinationY = Data_Walkers[enemyId].y;
		w->targetWalkerId = enemyId;
		Data_Walkers[enemyId].targetedByWalkerId = walkerId;
		w->targetWalkerCreatedSequence = Data_Walkers[enemyId].createdSequence;
		WalkerRoute_remove(walkerId);
		return 1;
	}
	return 0;
}

static int prefectGoFightFire(int walkerId, struct Data_Walker *w)
{
	if (Data_BuildingList.burning.size <= 0) {
		return 0;
	}
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
		case WalkerActionState_149_Corpse:
		case WalkerActionState_70_PrefectCreated:
		case WalkerActionState_71_PrefectEnteringExiting:
		case WalkerActionState_74_PrefectGoingToFire:
		case WalkerActionState_75_PrefectAtFire:
		case WalkerActionState_76_PrefectGoingToEnemy:
		case WalkerActionState_77_PrefectAtEnemy:
			return 0;
	}
	w->waitTicksMissile++;
	if (w->waitTicksMissile < 20) {
		return 0;
	}
	int distance;
	int ruinId = Security_Fire_getClosestBurningRuin(w->x, w->y, &distance);
	if (ruinId > 0 && distance <= 25) {
		w->waitTicksMissile = 0;
		w->actionState = WalkerActionState_74_PrefectGoingToFire;
		w->destinationX = Data_Buildings[ruinId].roadAccessX;
		w->destinationY = Data_Buildings[ruinId].roadAccessY;
		w->destinationBuildingId = ruinId;
		WalkerRoute_remove(walkerId);
		Data_Buildings[ruinId].walkerId4 = walkerId;
		return 1;
	}
	return 0;
}

static void prefectExtinguishFire(int walkerId, struct Data_Walker *w)
{
	struct Data_Building *b = &Data_Buildings[w->destinationBuildingId];
	int distance = Calc_distanceMaximum(w->x, w->y, b->x, b->y);
	if (b->inUse == 1 && b->type == Building_BurningRuin && distance < 2) {
		b->fireDuration = 32;
		Sound_Effects_playChannel(SoundChannel_FireSplash);
	} else {
		w->waitTicks = 1;
	}
	w->attackDirection = Routing_getGeneralDirection(w->x, w->y, b->x, b->y);
	if (w->attackDirection >= 8) {
		w->attackDirection = 0;
	}
	w->waitTicks--;
	if (w->waitTicks <= 0) {
		w->waitTicksMissile = 20;
		if (!prefectGoFightFire(walkerId, w)) {
			int xRoad, yRoad;
			if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
				w->actionState = WalkerActionState_73_PrefectReturning;
				w->destinationX = xRoad;
				w->destinationY = yRoad;
				WalkerRoute_remove(walkerId);
			} else {
				w->state = WalkerState_Dead;
			}
		}
	}
}

static int prefectTargetIsAlive(struct Data_Walker *w)
{
	if (w->targetWalkerId <= 0) {
		return 0;
	}
	struct Data_Walker *t = &Data_Walkers[w->targetWalkerId];
	if (t->state == WalkerState_Alive &&
		t->actionState != WalkerActionState_149_Corpse &&
		t->createdSequence == w->targetWalkerCreatedSequence) {
		return 1;
	}
	return 0;
}

void WalkerAction_prefect(int walkerId)
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
	
	// special actions
	if (!prefectGoFightEnemy(walkerId, w)) {
		prefectGoFightFire(walkerId, w);
	}
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_70_PrefectCreated:
			w->isGhost = 1;
			w->graphicOffset = 0;
			w->waitTicks--;
			if (w->waitTicks <= 0) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					w->actionState = WalkerActionState_71_PrefectEnteringExiting;
					WalkerAction_Common_setCrossCountryDestination(walkerId, w, xRoad, yRoad);
					w->roamLength = 0;
				} else {
					w->state = WalkerState_Dead;
				}
			}
			break;
		case WalkerActionState_71_PrefectEnteringExiting:
			w->useCrossCountry = 1;
			w->isGhost = 1;
			if (WalkerMovement_crossCountryWalkTicks(walkerId, 1) == 1) {
				if (Data_Grid_buildingIds[w->gridOffset] == w->buildingId) {
					// returned to own building
					w->state = WalkerState_Dead;
				} else {
					w->actionState = WalkerActionState_72_PrefectRoaming;
					WalkerMovement_initRoaming(walkerId);
					w->roamLength = 0;
				}
			}
			break;
		case WalkerActionState_72_PrefectRoaming:
			w->isGhost = 0;
			w->roamLength++;
			if (w->roamLength >= w->maxRoamLength) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					w->actionState = WalkerActionState_73_PrefectReturning;
					w->destinationX = xRoad;
					w->destinationY = yRoad;
					WalkerRoute_remove(walkerId);
				} else {
					w->state = WalkerState_Dead;
				}
			}
			WalkerMovement_roamTicks(walkerId, 1);
			break;
		case WalkerActionState_73_PrefectReturning:
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == 8) {
				w->actionState = WalkerActionState_71_PrefectEnteringExiting;
				WalkerAction_Common_setCrossCountryDestination(walkerId, w, b->x, b->y);
				w->roamLength = 0;
			} else if (w->direction == 9 || w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_74_PrefectGoingToFire:
			w->terrainUsage = 0;
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == 8) {
				w->actionState = WalkerActionState_75_PrefectAtFire;
				WalkerRoute_remove(walkerId);
				w->roamLength = 0;
				w->waitTicks = 50;
			} else if (w->direction == 9 || w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_75_PrefectAtFire:
			prefectExtinguishFire(walkerId, w);
			break;
		case WalkerActionState_76_PrefectGoingToEnemy:
			w->terrainUsage = 0;
			if (!prefectTargetIsAlive(w)) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					w->actionState = WalkerActionState_73_PrefectReturning;
					w->destinationX = xRoad;
					w->destinationY = yRoad;
					WalkerRoute_remove(walkerId);
					w->roamLength = 0;
				} else {
					w->state = WalkerState_Dead;
				}
			}
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == 8) {
				w->destinationX = Data_Walkers[w->targetWalkerId].x;
				w->destinationY = Data_Walkers[w->targetWalkerId].y;
				WalkerRoute_remove(walkerId);
			} else if (w->direction == 9 || w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
	}
	// graphic id
	int dir;
	if (w->actionState == WalkerActionState_75_PrefectAtFire ||
		w->actionState == WalkerActionState_150_Attack) {
		dir = w->attackDirection;
	} else if (w->direction < 8) {
		dir = w->direction;
	} else {
		dir = w->previousTileDirection;
	}
	dir = (8 + dir - Data_Settings_Map.orientation) % 8;
	switch (w->actionState) {
		case WalkerActionState_74_PrefectGoingToFire:
			w->graphicId = GraphicId(ID_Graphic_Walker_PrefectWithBucket) +
				dir + 8 * w->graphicOffset;
			break;
		case WalkerActionState_75_PrefectAtFire:
			w->graphicId = GraphicId(ID_Graphic_Walker_PrefectWithBucket) +
				dir + 96 + 8 * (w->graphicOffset) / 2;
			break;
		case WalkerActionState_150_Attack:
			if (w->attackGraphicOffset >= 12) {
				w->graphicId = GraphicId(ID_Graphic_Walker_Prefect) +
					104 + dir + 8 * ((w->attackGraphicOffset - 12) / 2);
			} else {
				w->graphicId = GraphicId(ID_Graphic_Walker_Prefect) + 104 + dir;
			}
			break;
		case WalkerActionState_149_Corpse:
			w->graphicId = GraphicId(ID_Graphic_Walker_Prefect) +
				96 + WalkerActionCorpseGraphicOffset(w);
			break;
		default:
			w->graphicId = GraphicId(ID_Graphic_Walker_Prefect) +
				dir + 8 * w->graphicOffset;
			break;
	}
}

void WalkerAction_worker(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = 1;
	w->useCrossCountry = 0;
	w->maxRoamLength = 384;
	if (Data_Buildings[w->buildingId].inUse != 1 ||
		Data_Buildings[w->buildingId].walkerId != walkerId) {
		w->state = WalkerState_Dead;
	}
}
