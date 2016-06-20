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

static void updateShowsAtDestination(struct Data_Walker *w)
{
	struct Data_Building *b = &Data_Buildings[w->destinationBuildingId];
	switch (w->type) {
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

static void updateGraphic(int walkerId, struct Data_Walker *w)
{
	int dir = w->direction < 8 ? w->direction : w->previousTileDirection;
	WalkerActionNormalizeDirection(dir);

	if (w->type == Figure_Charioteer) {
		w->cartGraphicId = 0;
		if (w->actionState == FigureActionState_150_Attack ||
			w->actionState == FigureActionState_149_Corpse) {
			w->graphicId = GraphicId(ID_Graphic_Figure_Charioteer) + dir;
		} else {
			w->graphicId = GraphicId(ID_Graphic_Figure_Charioteer) +
				dir + 8 * w->graphicOffset;
		}
		return;
	}
	int graphicId;
	if (w->type == Figure_Actor) {
		graphicId = GraphicId(ID_Graphic_Figure_Actor);
	} else if (w->type == Figure_Gladiator) {
		graphicId = GraphicId(ID_Graphic_Figure_Gladiator);
	} else if (w->type == Figure_LionTamer) {
		graphicId = GraphicId(ID_Graphic_Figure_LionTamer);
		if (w->waitTicksMissile >= 96) {
			graphicId = GraphicId(ID_Graphic_Figure_LionTamerWhip);
		}
		w->cartGraphicId = GraphicId(ID_Graphic_Figure_Lion);
	} else {
		return;
	}
	if (w->actionState == FigureActionState_150_Attack) {
		if (w->type == Figure_Gladiator) {
			w->graphicId = graphicId + 104 + dir + 8 * (w->graphicOffset / 2);
		} else {
			w->graphicId = graphicId + dir;
		}
	} else if (w->actionState == FigureActionState_149_Corpse) {
		w->graphicId = graphicId + 96 + WalkerActionCorpseGraphicOffset(w);
		w->cartGraphicId = 0;
	} else {
		w->graphicId = graphicId + dir + 8 * w->graphicOffset;
	}
	if (w->cartGraphicId) {
		w->cartGraphicId += dir + 8 * w->graphicOffset;
		FigureAction_Common_setCartOffset(walkerId, dir);
	}
}

void FigureAction_entertainer(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	w->cartGraphicId = GraphicId(ID_Graphic_Figure_CartpusherCart);
	w->terrainUsage = FigureTerrainUsage_Roads;
	w->useCrossCountry = 0;
	w->maxRoamLength = 512;
	FigureActionIncreaseGraphicOffset(w, 12);
	w->waitTicksMissile++;
	if (w->waitTicksMissile >= 120) {
		w->waitTicksMissile = 0;
	}
	if (Data_Event.gladiatorRevolt.state == SpecialEvent_InProgress && w->type == Figure_Gladiator) {
		if (w->actionState == FigureActionState_92_EntertainerGoingToVenue ||
			w->actionState == FigureActionState_94_EntertainerRoaming ||
			w->actionState == FigureActionState_95_EntertainerReturning) {
			w->type = Figure_Enemy54_Gladiator;
			FigureRoute_remove(walkerId);
			w->roamLength = 0;
			w->actionState = FigureActionState_158_NativeCreated;
			return;
		}
	}
	int speedFactor = w->type == Figure_Charioteer ? 2 : 1;
	switch (w->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(walkerId);
			FigureActionIncreaseGraphicOffset(w, 32);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_90_EntertainerAtSchoolCreated:
			w->isGhost = 1;
			w->graphicOffset = 0;
			w->waitTicksMissile = 0;
			w->waitTicks--;
			if (w->waitTicks <= 0) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					w->actionState = FigureActionState_91_EntertainerExitingSchool;
					FigureAction_Common_setCrossCountryDestination(walkerId, w, xRoad, yRoad);
					w->roamLength = 0;
				} else {
					w->state = FigureState_Dead;
				}
			}
			break;
		case FigureActionState_91_EntertainerExitingSchool:
			w->useCrossCountry = 1;
			w->isGhost = 1;
			if (FigureMovement_crossCountryWalkTicks(walkerId, 1) == 1) {
				int dstBuildingId = 0;
				switch (w->type) {
					case Figure_Actor:
						dstBuildingId = determineDestination(w->x, w->y, Building_Theater, Building_Amphitheater);
						break;
					case Figure_Gladiator:
						dstBuildingId = determineDestination(w->x, w->y, Building_Amphitheater, Building_Colosseum);
						break;
					case Figure_LionTamer:
						dstBuildingId = determineDestination(w->x, w->y, Building_Colosseum, 0);
						break;
					case Figure_Charioteer:
						dstBuildingId = determineDestination(w->x, w->y, Building_Hippodrome, 0);
						break;
				}
				if (dstBuildingId) {
					struct Data_Building *db = &Data_Buildings[dstBuildingId];
					int xRoad, yRoad;
					if (Terrain_getClosestRoadWithinRadius(db->x, db->y, db->size, 2, &xRoad, &yRoad)) {
						w->destinationBuildingId = dstBuildingId;
						w->actionState = FigureActionState_92_EntertainerGoingToVenue;
						w->destinationX = xRoad;
						w->destinationY = yRoad;
						w->roamLength = 0;
					} else {
						w->state = FigureState_Dead;
					}
				} else {
					w->state = FigureState_Dead;
				}
			}
			w->isGhost = 1;
			break;
		case FigureActionState_92_EntertainerGoingToVenue:
			w->isGhost = 0;
			w->roamLength++;
			if (w->roamLength >= 3200) {
				w->state = FigureState_Dead;
			}
			FigureMovement_walkTicks(walkerId, speedFactor);
			if (w->direction == DirFigure_8_AtDestination) {
				updateShowsAtDestination(w);
				w->state = FigureState_Dead;
			} else if (w->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
			} else if (w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			}
			break;
		case FigureActionState_94_EntertainerRoaming:
			w->isGhost = 0;
			w->roamLength++;
			if (w->roamLength >= w->maxRoamLength) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					w->actionState = FigureActionState_95_EntertainerReturning;
					w->destinationX = xRoad;
					w->destinationY = yRoad;
				} else {
					w->state = FigureState_Dead;
				}
			}
			FigureMovement_roamTicks(walkerId, speedFactor);
			break;
		case FigureActionState_95_EntertainerReturning:
			FigureMovement_walkTicks(walkerId, speedFactor);
			if (w->direction == DirFigure_8_AtDestination ||
				w->direction == DirFigure_9_Reroute || w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			}
			break;
	}
	updateGraphic(walkerId, w);
}
