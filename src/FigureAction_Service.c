#include "FigureAction_private.h"

#include "Security.h"
#include "Terrain.h"

#include "Data/CityInfo.h"

#include "building/building.h"
#include "building/list.h"
#include "building/maintenance.h"
#include "core/calc.h"
#include "figure/enemy_army.h"
#include "figure/route.h"
#include "figure/type.h"
#include "map/building.h"
#include "sound/effect.h"

void FigureAction_taxCollector(figure *f)
{
	building *b = building_get(f->buildingId);
	
	f->terrainUsage = FigureTerrainUsage_Roads;
	f->useCrossCountry = 0;
	f->maxRoamLength = 512;
	if (!BuildingIsInUse(b) || b->figureId != f->id) {
		f->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(f, 12);
	
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(f);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(f);
			break;
		case FigureActionState_40_TaxCollectorCreated:
			f->isGhost = 1;
			f->graphicOffset = 0;
			f->waitTicks--;
			if (f->waitTicks <= 0) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					f->actionState = FigureActionState_41_TaxCollectorEnteringExiting;
					FigureAction_Common_setCrossCountryDestination(f, xRoad, yRoad);
					f->roamLength = 0;
				} else {
					f->state = FigureState_Dead;
				}
			}
			break;
		case FigureActionState_41_TaxCollectorEnteringExiting:
			f->useCrossCountry = 1;
			f->isGhost = 1;
			if (FigureMovement_crossCountryWalkTicks(f, 1) == 1) {
				if (map_building_at(f->gridOffset) == f->buildingId) {
					// returned to own building
					f->state = FigureState_Dead;
				} else {
					f->actionState = FigureActionState_42_TaxCollectorRoaming;
					FigureMovement_initRoaming(f);
					f->roamLength = 0;
				}
			}
			break;
		case FigureActionState_42_TaxCollectorRoaming:
			f->isGhost = 0;
			f->roamLength++;
			if (f->roamLength >= f->maxRoamLength) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					f->actionState = FigureActionState_43_TaxCollectorReturning;
					f->destinationX = xRoad;
					f->destinationY = yRoad;
				} else {
					f->state = FigureState_Dead;
				}
			}
			FigureMovement_roamTicks(f, 1);
			break;
		case FigureActionState_43_TaxCollectorReturning:
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DIR_FIGURE_AT_DESTINATION) {
				f->actionState = FigureActionState_41_TaxCollectorEnteringExiting;
				FigureAction_Common_setCrossCountryDestination(f, b->x, b->y);
				f->roamLength = 0;
			} else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
				f->state = FigureState_Dead;
			}
			break;
	}
	FigureActionUpdateGraphic(f, image_group(GROUP_FIGURE_TAX_COLLECTOR));
}

void FigureAction_engineer(figure *f)
{
	building *b = building_get(f->buildingId);
	
	f->terrainUsage = FigureTerrainUsage_Roads;
	f->useCrossCountry = 0;
	f->maxRoamLength = 640;
	if (!BuildingIsInUse(b) || b->figureId != f->id) {
		f->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(f, 12);
	
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(f);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(f);
			break;
		case FigureActionState_60_EngineerCreated:
			f->isGhost = 1;
			f->graphicOffset = 0;
			f->waitTicks--;
			if (f->waitTicks <= 0) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					f->actionState = FigureActionState_61_EngineerEnteringExiting;
					FigureAction_Common_setCrossCountryDestination(f, xRoad, yRoad);
					f->roamLength = 0;
				} else {
					f->state = FigureState_Dead;
				}
			}
			break;
		case FigureActionState_61_EngineerEnteringExiting:
			f->useCrossCountry = 1;
			f->isGhost = 1;
			if (FigureMovement_crossCountryWalkTicks(f, 1) == 1) {
				if (map_building_at(f->gridOffset) == f->buildingId) {
					// returned to own building
					f->state = FigureState_Dead;
				} else {
					f->actionState = FigureActionState_62_EngineerRoaming;
					FigureMovement_initRoaming(f);
					f->roamLength = 0;
				}
			}
			break;
		case FigureActionState_62_EngineerRoaming:
			f->isGhost = 0;
			f->roamLength++;
			if (f->roamLength >= f->maxRoamLength) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					f->actionState = FigureActionState_63_EngineerReturning;
					f->destinationX = xRoad;
					f->destinationY = yRoad;
				} else {
					f->state = FigureState_Dead;
				}
			}
			FigureMovement_roamTicks(f, 1);
			break;
		case FigureActionState_63_EngineerReturning:
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DIR_FIGURE_AT_DESTINATION) {
				f->actionState = FigureActionState_61_EngineerEnteringExiting;
				FigureAction_Common_setCrossCountryDestination(f, b->x, b->y);
				f->roamLength = 0;
			} else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
				f->state = FigureState_Dead;
			}
			break;
	}
	FigureActionUpdateGraphic(f, image_group(GROUP_FIGURE_ENGINEER));
}

static int prefectGetNearestEnemy(int x, int y, int *distance)
{
	int minEnemyId = 0;
	int minDist = 10000;
	for (int i = 1; i < MAX_FIGURES; i++) {
		figure *f = figure_get(i);
		if (f->state != FigureState_Alive || f->targetedByFigureId) {
			continue;
		}
		int dist;
		if (f->type == FIGURE_RIOTER || f->type == FIGURE_ENEMY54_GLADIATOR) {
			dist = calc_maximum_distance(x, y, f->x, f->y);
		} else if (f->type == FIGURE_INDIGENOUS_NATIVE && f->actionState == FigureActionState_159_NativeAttacking) {
			dist = calc_maximum_distance(x, y, f->x, f->y);
		} else if (FigureIsEnemy(f->type)) {
			dist = 3 * calc_maximum_distance(x, y, f->x, f->y);
		} else if (f->type == FIGURE_WOLF) {
			dist = 4 * calc_maximum_distance(x, y, f->x, f->y);
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

static int prefectGoFightEnemy(figure *f)
{
	if (Data_CityInfo.riotersOrAttackingNativesInCity <= 0 && enemy_army_total_enemy_formations() <= 0) {
		return 0;
	}
	switch (f->actionState) {
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
	f->waitTicksNextTarget++;
	if (f->waitTicksNextTarget < 10) {
		return 0;
	}
	f->waitTicksNextTarget = 0;
	int distance;
	int enemyId = prefectGetNearestEnemy(f->x, f->y, &distance);
	if (enemyId > 0 && distance <= 30) {
        figure *enemy = figure_get(enemyId);
		f->waitTicksNextTarget = 0;
		f->actionState = FigureActionState_76_PrefectGoingToEnemy;
		f->destinationX = enemy->x;
		f->destinationY = enemy->y;
		f->targetFigureId = enemyId;
		enemy->targetedByFigureId = f->id;
		f->targetFigureCreatedSequence = enemy->createdSequence;
		figure_route_remove(f);
		return 1;
	}
	return 0;
}

static int prefectGoFightFire(figure *f)
{
	if (building_list_burning_size() <= 0) {
		return 0;
	}
	switch (f->actionState) {
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
	f->waitTicksMissile++;
	if (f->waitTicksMissile < 20) {
		return 0;
	}
	int distance;
	int ruinId = building_maintenance_get_closest_burning_ruin(f->x, f->y, &distance);
	if (ruinId > 0 && distance <= 25) {
        building *ruin = building_get(ruinId);
		f->waitTicksMissile = 0;
		f->actionState = FigureActionState_74_PrefectGoingToFire;
		f->destinationX = ruin->roadAccessX;
		f->destinationY = ruin->roadAccessY;
		f->destinationBuildingId = ruinId;
		figure_route_remove(f);
		ruin->figureId4 = f->id;
		return 1;
	}
	return 0;
}

static void prefectExtinguishFire(figure *f)
{
	building *burn = building_get(f->destinationBuildingId);
	int distance = calc_maximum_distance(f->x, f->y, burn->x, burn->y);
	if (BuildingIsInUse(burn) && burn->type == BUILDING_BURNING_RUIN && distance < 2) {
		burn->fireDuration = 32;
		sound_effect_play(SOUND_EFFECT_FIRE_SPLASH);
	} else {
		f->waitTicks = 1;
	}
	f->attackDirection = calc_general_direction(f->x, f->y, burn->x, burn->y);
	if (f->attackDirection >= 8) {
		f->attackDirection = 0;
	}
	f->waitTicks--;
	if (f->waitTicks <= 0) {
		f->waitTicksMissile = 20;
		if (!prefectGoFightFire(f)) {
			building *b = building_get(f->buildingId);
			int xRoad, yRoad;
			if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
				f->actionState = FigureActionState_73_PrefectReturning;
				f->destinationX = xRoad;
				f->destinationY = yRoad;
				figure_route_remove(f);
			} else {
				f->state = FigureState_Dead;
			}
		}
	}
}

static int prefectTargetIsAlive(figure *f)
{
	if (f->targetFigureId <= 0) {
		return 0;
	}
	figure *target = figure_get(f->targetFigureId);
	if (!figure_is_dead(target) && target->createdSequence == f->targetFigureCreatedSequence) {
		return 1;
	}
	return 0;
}

void FigureAction_prefect(figure *f)
{
	building *b = building_get(f->buildingId);
	
	f->terrainUsage = FigureTerrainUsage_Roads;
	f->useCrossCountry = 0;
	f->maxRoamLength = 640;
	if (!BuildingIsInUse(b) || b->figureId != f->id) {
		f->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(f, 12);
	
	// special actions
	if (!prefectGoFightEnemy(f)) {
		prefectGoFightFire(f);
	}
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(f);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(f);
			break;
		case FigureActionState_70_PrefectCreated:
			f->isGhost = 1;
			f->graphicOffset = 0;
			f->waitTicks--;
			if (f->waitTicks <= 0) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					f->actionState = FigureActionState_71_PrefectEnteringExiting;
					FigureAction_Common_setCrossCountryDestination(f, xRoad, yRoad);
					f->roamLength = 0;
				} else {
					f->state = FigureState_Dead;
				}
			}
			break;
		case FigureActionState_71_PrefectEnteringExiting:
			f->useCrossCountry = 1;
			f->isGhost = 1;
			if (FigureMovement_crossCountryWalkTicks(f, 1) == 1) {
				if (map_building_at(f->gridOffset) == f->buildingId) {
					// returned to own building
					f->state = FigureState_Dead;
				} else {
					f->actionState = FigureActionState_72_PrefectRoaming;
					FigureMovement_initRoaming(f);
					f->roamLength = 0;
				}
			}
			break;
		case FigureActionState_72_PrefectRoaming:
			f->isGhost = 0;
			f->roamLength++;
			if (f->roamLength >= f->maxRoamLength) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					f->actionState = FigureActionState_73_PrefectReturning;
					f->destinationX = xRoad;
					f->destinationY = yRoad;
					figure_route_remove(f);
				} else {
					f->state = FigureState_Dead;
				}
			}
			FigureMovement_roamTicks(f, 1);
			break;
		case FigureActionState_73_PrefectReturning:
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DIR_FIGURE_AT_DESTINATION) {
				f->actionState = FigureActionState_71_PrefectEnteringExiting;
				FigureAction_Common_setCrossCountryDestination(f, b->x, b->y);
				f->roamLength = 0;
			} else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_74_PrefectGoingToFire:
			f->terrainUsage = FigureTerrainUsage_Any;
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DIR_FIGURE_AT_DESTINATION) {
				f->actionState = FigureActionState_75_PrefectAtFire;
				figure_route_remove(f);
				f->roamLength = 0;
				f->waitTicks = 50;
			} else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_75_PrefectAtFire:
			prefectExtinguishFire(f);
			break;
		case FigureActionState_76_PrefectGoingToEnemy:
			f->terrainUsage = FigureTerrainUsage_Any;
			if (!prefectTargetIsAlive(f)) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					f->actionState = FigureActionState_73_PrefectReturning;
					f->destinationX = xRoad;
					f->destinationY = yRoad;
					figure_route_remove(f);
					f->roamLength = 0;
				} else {
					f->state = FigureState_Dead;
				}
			}
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                figure *target = figure_get(f->targetFigureId);
				f->destinationX = target->x;
				f->destinationY = target->y;
				figure_route_remove(f);
			} else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
				f->state = FigureState_Dead;
			}
			break;
	}
	// graphic id
	int dir;
	if (f->actionState == FigureActionState_75_PrefectAtFire ||
		f->actionState == FigureActionState_150_Attack) {
		dir = f->attackDirection;
	} else if (f->direction < 8) {
		dir = f->direction;
	} else {
		dir = f->previousTileDirection;
	}
	FigureActionNormalizeDirection(dir);
	switch (f->actionState) {
		case FigureActionState_74_PrefectGoingToFire:
			f->graphicId = image_group(GROUP_FIGURE_PREFECT_WITH_BUCKET) +
				dir + 8 * f->graphicOffset;
			break;
		case FigureActionState_75_PrefectAtFire:
			f->graphicId = image_group(GROUP_FIGURE_PREFECT_WITH_BUCKET) +
				dir + 96 + 8 * (f->graphicOffset / 2);
			break;
		case FigureActionState_150_Attack:
			if (f->attackGraphicOffset >= 12) {
				f->graphicId = image_group(GROUP_FIGURE_PREFECT) +
					104 + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
			} else {
				f->graphicId = image_group(GROUP_FIGURE_PREFECT) + 104 + dir;
			}
			break;
		case FigureActionState_149_Corpse:
			f->graphicId = image_group(GROUP_FIGURE_PREFECT) +
				96 + FigureActionCorpseGraphicOffset(f);
			break;
		default:
			f->graphicId = image_group(GROUP_FIGURE_PREFECT) +
				dir + 8 * f->graphicOffset;
			break;
	}
}

void FigureAction_worker(figure *f)
{
	f->terrainUsage = FigureTerrainUsage_Roads;
	f->useCrossCountry = 0;
	f->maxRoamLength = 384;
    building *b = building_get(f->buildingId);
	if (!BuildingIsInUse(b) || b->figureId != f->id) {
		f->state = FigureState_Dead;
	}
}
