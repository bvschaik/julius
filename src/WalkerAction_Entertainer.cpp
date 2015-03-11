#include "WalkerAction_private.h"

#include "Calc.h"
#include "Terrain.h"
#include "Walker.h"

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
		case Walker_Actor:
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
		case Walker_Gladiator:
			if (b->type == Building_Amphitheater) {
				b->data.entertainment.days1 = 32;
			} else {
				b->data.entertainment.days2 = 32;
			}
			break;
		case Walker_LionTamer:
		case Walker_Charioteer:
			b->data.entertainment.days1 = 32;
			break;
	}
}

static void updateGraphic(int walkerId, struct Data_Walker *w)
{
	int dir = w->direction < 8 ? w->direction : w->previousTileDirection;
	WalkerActionNormalizeDirection(dir);

	if (w->type == Walker_Charioteer) {
		w->cartGraphicId = 0;
		if (w->actionState == WalkerActionState_150_Attack ||
			w->actionState == WalkerActionState_149_Corpse) {
			w->graphicId = GraphicId(ID_Graphic_Walker_Charioteer) + dir;
		} else {
			w->graphicId = GraphicId(ID_Graphic_Walker_Charioteer) +
				dir + 8 * w->graphicOffset;
		}
		return;
	}
	int graphicId;
	if (w->type == Walker_Actor) {
		graphicId = GraphicId(ID_Graphic_Walker_Actor);
	} else if (w->type == Walker_Gladiator) {
		graphicId = GraphicId(ID_Graphic_Walker_Gladiator);
	} else if (w->type == Walker_LionTamer) {
		graphicId = GraphicId(ID_Graphic_Walker_LionTamer);
		if (w->waitTicksMissile >= 96) {
			graphicId = GraphicId(ID_Graphic_Walker_LionTamerWhip);
		}
		w->cartGraphicId = GraphicId(ID_Graphic_Walker_Lion);
	} else if (w->type == Walker_Charioteer) {
		graphicId = GraphicId(ID_Graphic_Walker_Charioteer);
		w->cartGraphicId = 0;
		if (w->actionState == WalkerActionState_150_Attack ||
			w->actionState == WalkerActionState_149_Corpse) {
			w->graphicId = graphicId + dir;
		} else {
			w->graphicId = graphicId + dir + 8 * w->graphicOffset;
		}
		return;
	}
	if (w->actionState == WalkerActionState_150_Attack) {
		if (w->type == Walker_Gladiator) {
			w->graphicId = graphicId + 104 + dir + 8 * (w->graphicOffset / 2);
		} else {
			w->graphicId = graphicId + dir;
		}
	} else if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = graphicId + 96 + WalkerActionCorpseGraphicOffset(w);
		w->cartGraphicId = 0;
	} else {
		w->graphicId = graphicId + dir + 8 * w->graphicOffset;
	}
	if (w->cartGraphicId) {
		w->cartGraphicId += dir + 8 * w->graphicOffset;
		WalkerAction_Common_setCartOffset(walkerId, dir);
	}
}

void WalkerAction_entertainer(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	w->cartGraphicId = GraphicId(ID_Graphic_Walker_CartpusherCart);
	w->terrainUsage = 1;
	w->useCrossCountry = 0;
	w->maxRoamLength = 512;
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->waitTicksMissile++;
	if (w->waitTicksMissile >= 120) {
		w->waitTicksMissile = 0;
	}
	if (Data_Event.gladiatorRevolt.state == 1 && w->type == Walker_Gladiator) {
		if (w->actionState == WalkerActionState_92_EntertainerGoingToVenue ||
			w->actionState == WalkerActionState_94_EntertainerRoaming ||
			w->actionState == WalkerActionState_95_EntertainerReturning) {
			w->type = Walker_Enemy54_Gladiator;
			WalkerRoute_remove(walkerId);
			w->roamLength = 0;
			w->actionState = WalkerActionState_158_NativeCreated;
			return;
		}
	}
	int speedFactor = w->type == Walker_Charioteer ? 2 : 1;
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			WalkerActionIncreaseGraphicOffset(w, 32);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_90_EntertainerAtSchoolCreated:
			w->isGhost = 1;
			w->graphicOffset = 0;
			w->waitTicksMissile = 0;
			w->waitTicks--;
			if (w->waitTicks <= 0) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					w->actionState = WalkerActionState_91_EntertainerExitingSchool;
					WalkerAction_Common_setCrossCountryDestination(walkerId, w, xRoad, yRoad);
					w->roamLength = 0;
				} else {
					w->state = WalkerState_Dead;
				}
			}
			break;
		case WalkerActionState_91_EntertainerExitingSchool:
			w->useCrossCountry = 1;
			w->isGhost = 1;
			if (WalkerMovement_crossCountryWalkTicks(walkerId, 1) == 1) {
				int dstBuildingId = 0;
				switch (w->type) {
					case Walker_Actor:
						dstBuildingId = determineDestination(w->x, w->y, Building_Theater, Building_Amphitheater);
						break;
					case Walker_Gladiator:
						dstBuildingId = determineDestination(w->x, w->y, Building_Amphitheater, Building_Colosseum);
						break;
					case Walker_LionTamer:
						dstBuildingId = determineDestination(w->x, w->y, Building_Colosseum, 0);
						break;
					case Walker_Charioteer:
						dstBuildingId = determineDestination(w->x, w->y, Building_Hippodrome, 0);
						break;
				}
				if (dstBuildingId) {
					struct Data_Building *db = &Data_Buildings[dstBuildingId];
					int xRoad, yRoad;
					if (Terrain_getClosestRoadWithinRadius(db->x, db->y, db->size, 2, &xRoad, &yRoad)) {
						w->destinationBuildingId = dstBuildingId;
						w->actionState = WalkerActionState_92_EntertainerGoingToVenue;
						w->destinationX = xRoad;
						w->destinationY = yRoad;
						w->roamLength = 0;
					} else {
						w->state = WalkerState_Dead;
					}
				} else {
					w->state = WalkerState_Dead;
				}
			}
			w->isGhost = 1;
			break;
		case WalkerActionState_92_EntertainerGoingToVenue:
			w->isGhost = 0;
			w->roamLength++;
			if (w->roamLength >= 3200) {
				w->state = WalkerState_Dead;
			}
			WalkerMovement_walkTicks(walkerId, speedFactor);
			if (w->direction == 8) {
				updateShowsAtDestination(w);
				w->state = WalkerState_Dead;
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_94_EntertainerRoaming:
			w->isGhost = 0;
			w->roamLength++;
			if (w->roamLength >= w->maxRoamLength) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					w->actionState = WalkerActionState_95_EntertainerReturning;
					w->destinationX = xRoad;
					w->destinationY = yRoad;
				} else {
					w->state = WalkerState_Dead;
				}
			}
			WalkerMovement_roamTicks(walkerId, speedFactor);
			break;
		case WalkerActionState_95_EntertainerReturning:
			WalkerMovement_walkTicks(walkerId, speedFactor);
			if (w->direction == 8 || w->direction == 9 || w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
	}
	updateGraphic(walkerId, w);
}
