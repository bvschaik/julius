#include "FigureAction_private.h"

#include "Terrain.h"

#include "building/building.h"
#include "building/list.h"
#include "core/calc.h"
#include "figure/route.h"
#include "figure/type.h"
#include "map/grid.h"
#include "map/road_network.h"
#include "scenario/gladiator_revolt.h"

static int determineDestination(int x, int y, int btype1, int btype2)
{
	int roadNetwork = map_road_network_get(map_grid_offset(x,y));

    building_list_small_clear();
	
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		building *b = building_get(i);
		if (!BuildingIsInUse(b)) {
			continue;
		}
		if (b->type != btype1 && b->type != btype2) {
			continue;
		}
		if (b->distanceFromEntry && b->roadNetworkId == roadNetwork) {
			if (b->type == BUILDING_HIPPODROME && b->prevPartBuildingId) {
				continue;
			}
			building_list_small_add(i);
		}
	}
	int total_venues = building_list_small_size();
	if (total_venues <= 0) {
		return 0;
	}
	const int *venues = building_list_small_items();
	int minBuildingId = 0;
	int minDistance = 10000;
	for (int i = 0; i < total_venues; i++) {
		building *b = building_get(venues[i]);
		int daysLeft;
		if (b->type == btype1) {
			daysLeft = b->data.entertainment.days1;
		} else if (b->type == btype2) {
			daysLeft = b->data.entertainment.days2;
		} else {
			daysLeft = 0;
		}
		int dist = daysLeft + calc_maximum_distance(x, y, b->x, b->y);
		if (dist < minDistance) {
			minDistance = dist;
			minBuildingId = venues[i];
		}
	}
	if (minBuildingId) {
		return minBuildingId;
	}
	return 0;
}

static void updateShowsAtDestination(figure *f)
{
	building *b = building_get(f->destinationBuildingId);
	switch (f->type) {
		case FIGURE_ACTOR:
			b->data.entertainment.play++;
			if (b->data.entertainment.play >= 5) {
				b->data.entertainment.play = 0;
			}
			if (b->type == BUILDING_THEATER) {
				b->data.entertainment.days1 = 32;
			} else {
				b->data.entertainment.days2 = 32;
			}
			break;
		case FIGURE_GLADIATOR:
			if (b->type == BUILDING_AMPHITHEATER) {
				b->data.entertainment.days1 = 32;
			} else {
				b->data.entertainment.days2 = 32;
			}
			break;
		case FIGURE_LION_TAMER:
		case FIGURE_CHARIOTEER:
			b->data.entertainment.days1 = 32;
			break;
	}
}

static void updateGraphic(figure *f)
{
	int dir = f->direction < 8 ? f->direction : f->previousTileDirection;
	FigureActionNormalizeDirection(dir);

	if (f->type == FIGURE_CHARIOTEER) {
		f->cartGraphicId = 0;
		if (f->actionState == FigureActionState_150_Attack ||
			f->actionState == FigureActionState_149_Corpse) {
			f->graphicId = image_group(GROUP_FIGURE_CHARIOTEER) + dir;
		} else {
			f->graphicId = image_group(GROUP_FIGURE_CHARIOTEER) +
				dir + 8 * f->graphicOffset;
		}
		return;
	}
	int graphicId;
	if (f->type == FIGURE_ACTOR) {
		graphicId = image_group(GROUP_FIGURE_ACTOR);
	} else if (f->type == FIGURE_GLADIATOR) {
		graphicId = image_group(GROUP_FIGURE_GLADIATOR);
	} else if (f->type == FIGURE_LION_TAMER) {
		graphicId = image_group(GROUP_FIGURE_LION_TAMER);
		if (f->waitTicksMissile >= 96) {
			graphicId = image_group(GROUP_FIGURE_LION_TAMER_WHIP);
		}
		f->cartGraphicId = image_group(GROUP_FIGURE_LION);
	} else {
		return;
	}
	if (f->actionState == FigureActionState_150_Attack) {
		if (f->type == FIGURE_GLADIATOR) {
			f->graphicId = graphicId + 104 + dir + 8 * (f->graphicOffset / 2);
		} else {
			f->graphicId = graphicId + dir;
		}
	} else if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = graphicId + 96 + FigureActionCorpseGraphicOffset(f);
		f->cartGraphicId = 0;
	} else {
		f->graphicId = graphicId + dir + 8 * f->graphicOffset;
	}
	if (f->cartGraphicId) {
		f->cartGraphicId += dir + 8 * f->graphicOffset;
		FigureAction_Common_setCartOffset(f, dir);
	}
}

void FigureAction_entertainer(figure *f)
{
	building *b = building_get(f->buildingId);
	f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART);
	f->terrainUsage = FigureTerrainUsage_Roads;
	f->useCrossCountry = 0;
	f->maxRoamLength = 512;
	FigureActionIncreaseGraphicOffset(f, 12);
	f->waitTicksMissile++;
	if (f->waitTicksMissile >= 120) {
		f->waitTicksMissile = 0;
	}
	if (scenario_gladiator_revolt_is_in_progress() && f->type == FIGURE_GLADIATOR) {
		if (f->actionState == FigureActionState_92_EntertainerGoingToVenue ||
			f->actionState == FigureActionState_94_EntertainerRoaming ||
			f->actionState == FigureActionState_95_EntertainerReturning) {
			f->type = FIGURE_ENEMY54_GLADIATOR;
			figure_route_remove(f);
			f->roamLength = 0;
			f->actionState = FigureActionState_158_NativeCreated;
			return;
		}
	}
	int speedFactor = f->type == FIGURE_CHARIOTEER ? 2 : 1;
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(f);
			FigureActionIncreaseGraphicOffset(f, 32);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(f);
			break;
		case FigureActionState_90_EntertainerAtSchoolCreated:
			f->isGhost = 1;
			f->graphicOffset = 0;
			f->waitTicksMissile = 0;
			f->waitTicks--;
			if (f->waitTicks <= 0) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					f->actionState = FigureActionState_91_EntertainerExitingSchool;
					FigureAction_Common_setCrossCountryDestination(f, xRoad, yRoad);
					f->roamLength = 0;
				} else {
					f->state = FigureState_Dead;
				}
			}
			break;
		case FigureActionState_91_EntertainerExitingSchool:
			f->useCrossCountry = 1;
			f->isGhost = 1;
			if (FigureMovement_crossCountryWalkTicks(f, 1) == 1) {
				int dstBuildingId = 0;
				switch (f->type) {
					case FIGURE_ACTOR:
						dstBuildingId = determineDestination(f->x, f->y, BUILDING_THEATER, BUILDING_AMPHITHEATER);
						break;
					case FIGURE_GLADIATOR:
						dstBuildingId = determineDestination(f->x, f->y, BUILDING_AMPHITHEATER, BUILDING_COLOSSEUM);
						break;
					case FIGURE_LION_TAMER:
						dstBuildingId = determineDestination(f->x, f->y, BUILDING_COLOSSEUM, 0);
						break;
					case FIGURE_CHARIOTEER:
						dstBuildingId = determineDestination(f->x, f->y, BUILDING_HIPPODROME, 0);
						break;
				}
				if (dstBuildingId) {
					building *bDst = building_get(dstBuildingId);
					int xRoad, yRoad;
					if (Terrain_getClosestRoadWithinRadius(bDst->x, bDst->y, bDst->size, 2, &xRoad, &yRoad)) {
						f->destinationBuildingId = dstBuildingId;
						f->actionState = FigureActionState_92_EntertainerGoingToVenue;
						f->destinationX = xRoad;
						f->destinationY = yRoad;
						f->roamLength = 0;
					} else {
						f->state = FigureState_Dead;
					}
				} else {
					f->state = FigureState_Dead;
				}
			}
			f->isGhost = 1;
			break;
		case FigureActionState_92_EntertainerGoingToVenue:
			f->isGhost = 0;
			f->roamLength++;
			if (f->roamLength >= 3200) {
				f->state = FigureState_Dead;
			}
			FigureMovement_walkTicks(f, speedFactor);
			if (f->direction == DIR_FIGURE_AT_DESTINATION) {
				updateShowsAtDestination(f);
				f->state = FigureState_Dead;
			} else if (f->direction == DIR_FIGURE_REROUTE) {
				figure_route_remove(f);
			} else if (f->direction == DIR_FIGURE_LOST) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_94_EntertainerRoaming:
			f->isGhost = 0;
			f->roamLength++;
			if (f->roamLength >= f->maxRoamLength) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					f->actionState = FigureActionState_95_EntertainerReturning;
					f->destinationX = xRoad;
					f->destinationY = yRoad;
				} else {
					f->state = FigureState_Dead;
				}
			}
			FigureMovement_roamTicks(f, speedFactor);
			break;
		case FigureActionState_95_EntertainerReturning:
			FigureMovement_walkTicks(f, speedFactor);
			if (f->direction == DIR_FIGURE_AT_DESTINATION ||
				f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
				f->state = FigureState_Dead;
			}
			break;
	}
	updateGraphic(f);
}
