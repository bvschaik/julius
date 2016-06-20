#include "FigureAction_private.h"

#include "Calc.h"
#include "Figure.h"
#include "Routing.h"
#include "Security.h"
#include "Sound.h"
#include "Terrain.h"

#include "Data/CityInfo.h"
#include "Data/Formation.h"
#include "Data/Grid.h"

void FigureAction_taxCollector(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	
	w->terrainUsage = FigureTerrainUsage_Roads;
	w->useCrossCountry = 0;
	w->maxRoamLength = 512;
	if (!BuildingIsInUse(w->buildingId) || b->walkerId != walkerId) {
		w->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(w, 12);
	
	switch (w->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_40_TaxCollectorCreated:
			w->isGhost = 1;
			w->graphicOffset = 0;
			w->waitTicks--;
			if (w->waitTicks <= 0) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					w->actionState = FigureActionState_41_TaxCollectorEnteringExiting;
					FigureAction_Common_setCrossCountryDestination(walkerId, w, xRoad, yRoad);
					w->roamLength = 0;
				} else {
					w->state = FigureState_Dead;
				}
			}
			break;
		case FigureActionState_41_TaxCollectorEnteringExiting:
			w->useCrossCountry = 1;
			w->isGhost = 1;
			if (FigureMovement_crossCountryWalkTicks(walkerId, 1) == 1) {
				if (Data_Grid_buildingIds[w->gridOffset] == w->buildingId) {
					// returned to own building
					w->state = FigureState_Dead;
				} else {
					w->actionState = FigureActionState_42_TaxCollectorRoaming;
					FigureMovement_initRoaming(walkerId);
					w->roamLength = 0;
				}
			}
			break;
		case FigureActionState_42_TaxCollectorRoaming:
			w->isGhost = 0;
			w->roamLength++;
			if (w->roamLength >= w->maxRoamLength) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					w->actionState = FigureActionState_43_TaxCollectorReturning;
					w->destinationX = xRoad;
					w->destinationY = yRoad;
				} else {
					w->state = FigureState_Dead;
				}
			}
			FigureMovement_roamTicks(walkerId, 1);
			break;
		case FigureActionState_43_TaxCollectorReturning:
			FigureMovement_walkTicks(walkerId, 1);
			if (w->direction == DirFigure_8_AtDestination) {
				w->actionState = FigureActionState_41_TaxCollectorEnteringExiting;
				FigureAction_Common_setCrossCountryDestination(walkerId, w, b->x, b->y);
				w->roamLength = 0;
			} else if (w->direction == DirFigure_9_Reroute || w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			}
			break;
	}
	WalkerActionUpdateGraphic(w, GraphicId(ID_Graphic_Figure_TaxCollector));
}

void FigureAction_engineer(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	
	w->terrainUsage = FigureTerrainUsage_Roads;
	w->useCrossCountry = 0;
	w->maxRoamLength = 640;
	if (!BuildingIsInUse(w->buildingId) || b->walkerId != walkerId) {
		w->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(w, 12);
	
	switch (w->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_60_EngineerCreated:
			w->isGhost = 1;
			w->graphicOffset = 0;
			w->waitTicks--;
			if (w->waitTicks <= 0) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					w->actionState = FigureActionState_61_EngineerEnteringExiting;
					FigureAction_Common_setCrossCountryDestination(walkerId, w, xRoad, yRoad);
					w->roamLength = 0;
				} else {
					w->state = FigureState_Dead;
				}
			}
			break;
		case FigureActionState_61_EngineerEnteringExiting:
			w->useCrossCountry = 1;
			w->isGhost = 1;
			if (FigureMovement_crossCountryWalkTicks(walkerId, 1) == 1) {
				if (Data_Grid_buildingIds[w->gridOffset] == w->buildingId) {
					// returned to own building
					w->state = FigureState_Dead;
				} else {
					w->actionState = FigureActionState_62_EngineerRoaming;
					FigureMovement_initRoaming(walkerId);
					w->roamLength = 0;
				}
			}
			break;
		case FigureActionState_62_EngineerRoaming:
			w->isGhost = 0;
			w->roamLength++;
			if (w->roamLength >= w->maxRoamLength) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					w->actionState = FigureActionState_63_EngineerReturning;
					w->destinationX = xRoad;
					w->destinationY = yRoad;
				} else {
					w->state = FigureState_Dead;
				}
			}
			FigureMovement_roamTicks(walkerId, 1);
			break;
		case FigureActionState_63_EngineerReturning:
			FigureMovement_walkTicks(walkerId, 1);
			if (w->direction == DirFigure_8_AtDestination) {
				w->actionState = FigureActionState_61_EngineerEnteringExiting;
				FigureAction_Common_setCrossCountryDestination(walkerId, w, b->x, b->y);
				w->roamLength = 0;
			} else if (w->direction == DirFigure_9_Reroute || w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			}
			break;
	}
	WalkerActionUpdateGraphic(w, GraphicId(ID_Graphic_Figure_Engineer));
}

static int prefectGetNearestEnemy(int x, int y, int *distance)
{
	int minEnemyId = 0;
	int minDist = 10000;
	for (int i = 1; i < MAX_FIGURES; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state != FigureState_Alive || w->targetedByWalkerId) {
			continue;
		}
		int dist;
		if (w->type == Figure_Rioter || w->type == Figure_Enemy54_Gladiator) {
			dist = Calc_distanceMaximum(x, y, w->x, w->y);
		} else if (w->type == Figure_IndigenousNative && w->actionState == FigureActionState_159_NativeAttacking) {
			dist = Calc_distanceMaximum(x, y, w->x, w->y);
		} else if (WalkerIsEnemy(w->type)) {
			dist = 3 * Calc_distanceMaximum(x, y, w->x, w->y);
		} else if (w->type == Figure_Wolf) {
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
		case FigureActionState_150_Attack:
		case FigureActionState_149_Corpse:
		case FigureActionState_70_PrefectCreated:
		case FigureActionState_71_PrefectEnteringExiting:
		case FigureActionState_74_PrefectGoingToFire:
		case FigureActionState_75_PrefectAtFire:
		case FigureActionState_76_PrefectGoingToEnemy:
		case FigureActionState_77_PrefectAtEnemy:
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
		w->actionState = FigureActionState_76_PrefectGoingToEnemy;
		w->destinationX = Data_Walkers[enemyId].x;
		w->destinationY = Data_Walkers[enemyId].y;
		w->targetWalkerId = enemyId;
		Data_Walkers[enemyId].targetedByWalkerId = walkerId;
		w->targetWalkerCreatedSequence = Data_Walkers[enemyId].createdSequence;
		FigureRoute_remove(walkerId);
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
		case FigureActionState_150_Attack:
		case FigureActionState_149_Corpse:
		case FigureActionState_70_PrefectCreated:
		case FigureActionState_71_PrefectEnteringExiting:
		case FigureActionState_74_PrefectGoingToFire:
		case FigureActionState_75_PrefectAtFire:
		case FigureActionState_76_PrefectGoingToEnemy:
		case FigureActionState_77_PrefectAtEnemy:
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
		w->actionState = FigureActionState_74_PrefectGoingToFire;
		w->destinationX = Data_Buildings[ruinId].roadAccessX;
		w->destinationY = Data_Buildings[ruinId].roadAccessY;
		w->destinationBuildingId = ruinId;
		FigureRoute_remove(walkerId);
		Data_Buildings[ruinId].walkerId4 = walkerId;
		return 1;
	}
	return 0;
}

static void prefectExtinguishFire(int walkerId, struct Data_Walker *w)
{
	struct Data_Building *burn = &Data_Buildings[w->destinationBuildingId];
	int distance = Calc_distanceMaximum(w->x, w->y, burn->x, burn->y);
	if (BuildingIsInUse(w->destinationBuildingId) && burn->type == Building_BurningRuin && distance < 2) {
		burn->fireDuration = 32;
		Sound_Effects_playChannel(SoundChannel_FireSplash);
	} else {
		w->waitTicks = 1;
	}
	w->attackDirection = Routing_getGeneralDirection(w->x, w->y, burn->x, burn->y);
	if (w->attackDirection >= 8) {
		w->attackDirection = 0;
	}
	w->waitTicks--;
	if (w->waitTicks <= 0) {
		w->waitTicksMissile = 20;
		if (!prefectGoFightFire(walkerId, w)) {
			struct Data_Building *b = &Data_Buildings[w->buildingId];
			int xRoad, yRoad;
			if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
				w->actionState = FigureActionState_73_PrefectReturning;
				w->destinationX = xRoad;
				w->destinationY = yRoad;
				FigureRoute_remove(walkerId);
			} else {
				w->state = FigureState_Dead;
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
	if (!FigureIsDead(w->targetWalkerId) && t->createdSequence == w->targetWalkerCreatedSequence) {
		return 1;
	}
	return 0;
}

void FigureAction_prefect(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	
	w->terrainUsage = FigureTerrainUsage_Roads;
	w->useCrossCountry = 0;
	w->maxRoamLength = 640;
	if (!BuildingIsInUse(w->buildingId) || b->walkerId != walkerId) {
		w->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(w, 12);
	
	// special actions
	if (!prefectGoFightEnemy(walkerId, w)) {
		prefectGoFightFire(walkerId, w);
	}
	switch (w->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_70_PrefectCreated:
			w->isGhost = 1;
			w->graphicOffset = 0;
			w->waitTicks--;
			if (w->waitTicks <= 0) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					w->actionState = FigureActionState_71_PrefectEnteringExiting;
					FigureAction_Common_setCrossCountryDestination(walkerId, w, xRoad, yRoad);
					w->roamLength = 0;
				} else {
					w->state = FigureState_Dead;
				}
			}
			break;
		case FigureActionState_71_PrefectEnteringExiting:
			w->useCrossCountry = 1;
			w->isGhost = 1;
			if (FigureMovement_crossCountryWalkTicks(walkerId, 1) == 1) {
				if (Data_Grid_buildingIds[w->gridOffset] == w->buildingId) {
					// returned to own building
					w->state = FigureState_Dead;
				} else {
					w->actionState = FigureActionState_72_PrefectRoaming;
					FigureMovement_initRoaming(walkerId);
					w->roamLength = 0;
				}
			}
			break;
		case FigureActionState_72_PrefectRoaming:
			w->isGhost = 0;
			w->roamLength++;
			if (w->roamLength >= w->maxRoamLength) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					w->actionState = FigureActionState_73_PrefectReturning;
					w->destinationX = xRoad;
					w->destinationY = yRoad;
					FigureRoute_remove(walkerId);
				} else {
					w->state = FigureState_Dead;
				}
			}
			FigureMovement_roamTicks(walkerId, 1);
			break;
		case FigureActionState_73_PrefectReturning:
			FigureMovement_walkTicks(walkerId, 1);
			if (w->direction == DirFigure_8_AtDestination) {
				w->actionState = FigureActionState_71_PrefectEnteringExiting;
				FigureAction_Common_setCrossCountryDestination(walkerId, w, b->x, b->y);
				w->roamLength = 0;
			} else if (w->direction == DirFigure_9_Reroute || w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			}
			break;
		case FigureActionState_74_PrefectGoingToFire:
			w->terrainUsage = FigureTerrainUsage_Any;
			FigureMovement_walkTicks(walkerId, 1);
			if (w->direction == DirFigure_8_AtDestination) {
				w->actionState = FigureActionState_75_PrefectAtFire;
				FigureRoute_remove(walkerId);
				w->roamLength = 0;
				w->waitTicks = 50;
			} else if (w->direction == DirFigure_9_Reroute || w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			}
			break;
		case FigureActionState_75_PrefectAtFire:
			prefectExtinguishFire(walkerId, w);
			break;
		case FigureActionState_76_PrefectGoingToEnemy:
			w->terrainUsage = FigureTerrainUsage_Any;
			if (!prefectTargetIsAlive(w)) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					w->actionState = FigureActionState_73_PrefectReturning;
					w->destinationX = xRoad;
					w->destinationY = yRoad;
					FigureRoute_remove(walkerId);
					w->roamLength = 0;
				} else {
					w->state = FigureState_Dead;
				}
			}
			FigureMovement_walkTicks(walkerId, 1);
			if (w->direction == DirFigure_8_AtDestination) {
				w->destinationX = Data_Walkers[w->targetWalkerId].x;
				w->destinationY = Data_Walkers[w->targetWalkerId].y;
				FigureRoute_remove(walkerId);
			} else if (w->direction == DirFigure_9_Reroute || w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			}
			break;
	}
	// graphic id
	int dir;
	if (w->actionState == FigureActionState_75_PrefectAtFire ||
		w->actionState == FigureActionState_150_Attack) {
		dir = w->attackDirection;
	} else if (w->direction < 8) {
		dir = w->direction;
	} else {
		dir = w->previousTileDirection;
	}
	WalkerActionNormalizeDirection(dir);
	switch (w->actionState) {
		case FigureActionState_74_PrefectGoingToFire:
			w->graphicId = GraphicId(ID_Graphic_Figure_PrefectWithBucket) +
				dir + 8 * w->graphicOffset;
			break;
		case FigureActionState_75_PrefectAtFire:
			w->graphicId = GraphicId(ID_Graphic_Figure_PrefectWithBucket) +
				dir + 96 + 8 * (w->graphicOffset / 2);
			break;
		case FigureActionState_150_Attack:
			if (w->attackGraphicOffset >= 12) {
				w->graphicId = GraphicId(ID_Graphic_Figure_Prefect) +
					104 + dir + 8 * ((w->attackGraphicOffset - 12) / 2);
			} else {
				w->graphicId = GraphicId(ID_Graphic_Figure_Prefect) + 104 + dir;
			}
			break;
		case FigureActionState_149_Corpse:
			w->graphicId = GraphicId(ID_Graphic_Figure_Prefect) +
				96 + WalkerActionCorpseGraphicOffset(w);
			break;
		default:
			w->graphicId = GraphicId(ID_Graphic_Figure_Prefect) +
				dir + 8 * w->graphicOffset;
			break;
	}
}

void FigureAction_worker(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = FigureTerrainUsage_Roads;
	w->useCrossCountry = 0;
	w->maxRoamLength = 384;
	if (!BuildingIsInUse(w->buildingId) ||
		Data_Buildings[w->buildingId].walkerId != walkerId) {
		w->state = FigureState_Dead;
	}
}
