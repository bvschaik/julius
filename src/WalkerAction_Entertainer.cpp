#include "FigureAction_private.h"

#include "Calc.h"
#include "Figure.h"
#include "Terrain.h"

#include "Data/Event.h"
#include "Data/Grid.h"

static int determineDestination(int x, int y, int btype1, int btype2)
{
	int roadNetwork = Data_Grid_roadNetworks[GridOffset(x,y)];

	Data_BuildingList.small.size = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (!BuildingIsInUse(i)) {
			continue;
		}
		if (b->type != btype1 && b->type != btype2) {
			continue;
		}
		if (b->distanceFromEntry && b->roadNetworkId == roadNetwork) {
			if (b->type == Building_Hippodrome && b->prevPartBuildingId) {
				continue;
			}
			DATA_BUILDINGLIST_SMALL_ENQUEUE(i);
		}
	}
	if (Data_BuildingList.small.size <= 0) {
		return 0;
	}
	int minBuildingId = 0;
	int minDistance = 10000;
	for (int i = 0; i < Data_BuildingList.small.size; i++) {
		struct Data_Building *b = &Data_Buildings[Data_BuildingList.small.items[i]];
		int daysLeft;
		if (b->type == btype1) {
			daysLeft = b->data.entertainment.days1;
		} else if (b->type == btype2) {
			daysLeft = b->data.entertainment.days2;
		} else {
			daysLeft = 0;
		}
		int dist = daysLeft + Calc_distanceMaximum(x, y, b->x, b->y);
		if (dist < minDistance) {
			minDistance = dist;
			minBuildingId = Data_BuildingList.small.items[i];
		}
	}
	if (minBuildingId) {
		return minBuildingId;
	}
	return 0;
}

static void updateShowsAtDestination(struct Data_Walker *f)
{
	struct Data_Building *b = &Data_Buildings[f->destinationBuildingId];
	switch (f->type) {
		case Figure_Actor:
			b->data.entertainment.play++;
			if (b->data.entertainment.play >= 5) {
				b->data.entertainment.play = 0;
			}
			if (b->type == Building_Theater) {
				b->data.entertainment.days1 = 32;
			} else {
				b->data.entertainment.days2 = 32;
			}
			break;
		case Figure_Gladiator:
			if (b->type == Building_Amphitheater) {
				b->data.entertainment.days1 = 32;
			} else {
				b->data.entertainment.days2 = 32;
			}
			break;
		case Figure_LionTamer:
		case Figure_Charioteer:
			b->data.entertainment.days1 = 32;
			break;
	}
}

static void updateGraphic(int walkerId, struct Data_Walker *f)
{
	int dir = f->direction < 8 ? f->direction : f->previousTileDirection;
	WalkerActionNormalizeDirection(dir);

	if (f->type == Figure_Charioteer) {
		f->cartGraphicId = 0;
		if (f->actionState == FigureActionState_150_Attack ||
			f->actionState == FigureActionState_149_Corpse) {
			f->graphicId = GraphicId(ID_Graphic_Figure_Charioteer) + dir;
		} else {
			f->graphicId = GraphicId(ID_Graphic_Figure_Charioteer) +
				dir + 8 * f->graphicOffset;
		}
		return;
	}
	int graphicId;
	if (f->type == Figure_Actor) {
		graphicId = GraphicId(ID_Graphic_Figure_Actor);
	} else if (f->type == Figure_Gladiator) {
		graphicId = GraphicId(ID_Graphic_Figure_Gladiator);
	} else if (f->type == Figure_LionTamer) {
		graphicId = GraphicId(ID_Graphic_Figure_LionTamer);
		if (f->waitTicksMissile >= 96) {
			graphicId = GraphicId(ID_Graphic_Figure_LionTamerWhip);
		}
		f->cartGraphicId = GraphicId(ID_Graphic_Figure_Lion);
	} else {
		return;
	}
	if (f->actionState == FigureActionState_150_Attack) {
		if (f->type == Figure_Gladiator) {
			f->graphicId = graphicId + 104 + dir + 8 * (f->graphicOffset / 2);
		} else {
			f->graphicId = graphicId + dir;
		}
	} else if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = graphicId + 96 + WalkerActionCorpseGraphicOffset(f);
		f->cartGraphicId = 0;
	} else {
		f->graphicId = graphicId + dir + 8 * f->graphicOffset;
	}
	if (f->cartGraphicId) {
		f->cartGraphicId += dir + 8 * f->graphicOffset;
		FigureAction_Common_setCartOffset(walkerId, dir);
	}
}

void FigureAction_entertainer(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[f->buildingId];
	f->cartGraphicId = GraphicId(ID_Graphic_Figure_CartpusherCart);
	f->terrainUsage = FigureTerrainUsage_Roads;
	f->useCrossCountry = 0;
	f->maxRoamLength = 512;
	FigureActionIncreaseGraphicOffset(f, 12);
	f->waitTicksMissile++;
	if (f->waitTicksMissile >= 120) {
		f->waitTicksMissile = 0;
	}
	if (Data_Event.gladiatorRevolt.state == SpecialEvent_InProgress && f->type == Figure_Gladiator) {
		if (f->actionState == FigureActionState_92_EntertainerGoingToVenue ||
			f->actionState == FigureActionState_94_EntertainerRoaming ||
			f->actionState == FigureActionState_95_EntertainerReturning) {
			f->type = Figure_Enemy54_Gladiator;
			FigureRoute_remove(walkerId);
			f->roamLength = 0;
			f->actionState = FigureActionState_158_NativeCreated;
			return;
		}
	}
	int speedFactor = f->type == Figure_Charioteer ? 2 : 1;
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(walkerId);
			FigureActionIncreaseGraphicOffset(f, 32);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(walkerId);
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
					FigureAction_Common_setCrossCountryDestination(walkerId, f, xRoad, yRoad);
					f->roamLength = 0;
				} else {
					f->state = FigureState_Dead;
				}
			}
			break;
		case FigureActionState_91_EntertainerExitingSchool:
			f->useCrossCountry = 1;
			f->isGhost = 1;
			if (FigureMovement_crossCountryWalkTicks(walkerId, 1) == 1) {
				int dstBuildingId = 0;
				switch (f->type) {
					case Figure_Actor:
						dstBuildingId = determineDestination(f->x, f->y, Building_Theater, Building_Amphitheater);
						break;
					case Figure_Gladiator:
						dstBuildingId = determineDestination(f->x, f->y, Building_Amphitheater, Building_Colosseum);
						break;
					case Figure_LionTamer:
						dstBuildingId = determineDestination(f->x, f->y, Building_Colosseum, 0);
						break;
					case Figure_Charioteer:
						dstBuildingId = determineDestination(f->x, f->y, Building_Hippodrome, 0);
						break;
				}
				if (dstBuildingId) {
					struct Data_Building *bDst = &Data_Buildings[dstBuildingId];
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
			FigureMovement_walkTicks(walkerId, speedFactor);
			if (f->direction == DirFigure_8_AtDestination) {
				updateShowsAtDestination(f);
				f->state = FigureState_Dead;
			} else if (f->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
			} else if (f->direction == DirFigure_10_Lost) {
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
			FigureMovement_roamTicks(walkerId, speedFactor);
			break;
		case FigureActionState_95_EntertainerReturning:
			FigureMovement_walkTicks(walkerId, speedFactor);
			if (f->direction == DirFigure_8_AtDestination ||
				f->direction == DirFigure_9_Reroute || f->direction == DirFigure_10_Lost) {
				f->state = FigureState_Dead;
			}
			break;
	}
	updateGraphic(walkerId, f);
}
