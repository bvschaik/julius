#include "WalkerAction_private.h"

#include "Routing.h"
#include "Walker.h"

#include "Data/CityInfo.h"
#include "Data/Formation.h"
#include "Data/Random.h"

static const int seagullOffsetsX[] = {0, 0, -2, 1, 2, -3, 4, -2};
static const int seagullOffsetsY[] = {0, -2, 0, 2, 0, 1, -3, 4};

static const int hippodromeHorseDestinationX1[] = {
	2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2
};
static const int hippodromeHorseDestinationY1[] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2
};
static const int hippodromeHorseDestinationX2[] = {
	12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};
static const int hippodromeHorseDestinationY2[] = {
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2
};

static const int sheepGraphicOffsets[] = {
	0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 4, 4, 5, 5, -1, -1, -1, -1, -1, -1, -1, -1,
	0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 5, 5
};

void WalkerAction_seagulls(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = 0;
	w->isGhost = 0;
	w->useCrossCountry = 1;
	if (!(w->graphicOffset & 3) && WalkerMovement_crossCountryWalkTicks(walkerId, 1)) {
		w->progressOnTile++;
		if (w->progressOnTile > 8) {
			w->progressOnTile = 0;
		}
		WalkerAction_Common_setCrossCountryDestination(walkerId, w,
			w->sourceX + seagullOffsetsX[w->progressOnTile],
			w->sourceY + seagullOffsetsY[w->progressOnTile]);
	}
	if (walkerId & 1) {
		WalkerActionIncreaseGraphicOffset(w, 54);
		w->graphicId = GraphicId(ID_Graphic_Walker_Seagulls) + w->graphicOffset / 3;
	} else {
		WalkerActionIncreaseGraphicOffset(w, 72);
		w->graphicId = GraphicId(ID_Graphic_Walker_Seagulls) + 18 + w->graphicOffset / 3;
	}
}

void WalkerAction_sheep(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[w->formationId];
	w->terrainUsage = WalkerTerrainUsage_Animal;
	w->useCrossCountry = 0;
	w->isGhost = 0;
	Data_CityInfo.numAnimalsInCity++;
	WalkerActionIncreaseGraphicOffset(w, 6);

	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_196_HerdAnimalAtRest:
			w->waitTicks++;
			if (w->waitTicks > 400) {
				w->waitTicks = walkerId & 0x1f;
				w->actionState = WalkerActionState_197_HerdAnimalMoving;
				w->destinationX = f->destinationX + WalkerActionFormationLayoutPositionX(FormationLayout_Herd, w->indexInFormation);
				w->destinationY = f->destinationY + WalkerActionFormationLayoutPositionY(FormationLayout_Herd, w->indexInFormation);
				w->roamLength = 0;
			}
			break;
		case WalkerActionState_197_HerdAnimalMoving:
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirWalker_8_AtDestination || w->direction == DirWalker_10_Lost) {
				w->direction = w->previousTileDirection;
				w->actionState = WalkerActionState_196_HerdAnimalAtRest;
				w->waitTicks = walkerId & 0x1f;
			} else if (w->direction == DirWalker_9_Reroute) {
				WalkerRoute_remove(walkerId);
			}
			break;
	}
	int dir = WalkerActionDirection(w);
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Sheep) + 104 +
			WalkerActionCorpseGraphicOffset(w);
	} else if (w->actionState == WalkerActionState_196_HerdAnimalAtRest) {
		if (walkerId & 3) {
			w->graphicId = GraphicId(ID_Graphic_Walker_Sheep) + 48 + dir +
				8 * sheepGraphicOffsets[w->waitTicks & 0x3f];
		} else {
			w->graphicId = GraphicId(ID_Graphic_Walker_Sheep) + 96 + dir;
		}
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_Sheep) + dir + 8 * w->graphicOffset;
	}
}

void WalkerAction_wolf(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[w->formationId];
	w->terrainUsage = WalkerTerrainUsage_Animal;
	w->useCrossCountry = 0;
	w->isGhost = 0;
	Data_CityInfo.numAnimalsInCity++;
	WalkerActionIncreaseGraphicOffset(w, 12);

	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_196_HerdAnimalAtRest:
			w->waitTicks++;
			if (w->waitTicks > 400) {
				w->waitTicks = walkerId & 0x1f;
				w->actionState = WalkerActionState_197_HerdAnimalMoving;
				w->destinationX = f->destinationX + WalkerActionFormationLayoutPositionX(FormationLayout_Herd, w->indexInFormation);
				w->destinationY = f->destinationY + WalkerActionFormationLayoutPositionY(FormationLayout_Herd, w->indexInFormation);
				w->roamLength = 0;
			}
			break;
		case WalkerActionState_197_HerdAnimalMoving:
			WalkerMovement_walkTicks(walkerId, 2);
			if (w->direction == DirWalker_8_AtDestination || w->direction == DirWalker_10_Lost) {
				w->direction = w->previousTileDirection;
				w->actionState = WalkerActionState_196_HerdAnimalAtRest;
				w->waitTicks = walkerId & 0x1f;
			} else if (w->direction == DirWalker_9_Reroute) {
				WalkerRoute_remove(walkerId);
			}
			break;
		case WalkerActionState_199_WolfAttacking:
			WalkerMovement_walkTicks(walkerId, 2);
			if (w->direction == DirWalker_8_AtDestination) {
				int targetId = WalkerAction_CombatWolf_getTarget(w->x, w->y, 6);
				if (targetId) {
					w->destinationX = Data_Walkers[targetId].x;
					w->destinationY = Data_Walkers[targetId].y;
					w->targetWalkerId = targetId;
					Data_Walkers[targetId].targetedByWalkerId = walkerId;
					w->targetWalkerCreatedSequence = Data_Walkers[targetId].createdSequence;
					WalkerRoute_remove(walkerId);
				} else {
					w->direction = w->previousTileDirection;
					w->actionState = WalkerActionState_196_HerdAnimalAtRest;
					w->waitTicks = walkerId & 0x1f;
				}
			} else if (w->direction == DirWalker_9_Reroute) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == DirWalker_10_Lost) {
				w->direction = w->previousTileDirection;
				w->actionState = WalkerActionState_196_HerdAnimalAtRest;
				w->waitTicks = walkerId & 0x1f;
			}
			break;
	}
	int dir = WalkerActionDirection(w);
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Wolf) + 96 +
			WalkerActionCorpseGraphicOffset(w);
	} else if (w->actionState == WalkerActionState_150_Attack) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Wolf) + 104 +
			dir + 8 * (w->attackGraphicOffset / 4);
	} else if (w->actionState == WalkerActionState_196_HerdAnimalAtRest) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Wolf) + 152 + dir;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_Wolf) + dir + 8 * w->graphicOffset;
	}
}

void WalkerAction_zebra(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[w->formationId];
	w->terrainUsage = WalkerTerrainUsage_Animal;
	w->useCrossCountry = 0;
	w->isGhost = 0;
	Data_CityInfo.numAnimalsInCity++;
	WalkerActionIncreaseGraphicOffset(w, 12);

	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_196_HerdAnimalAtRest:
			w->waitTicks++;
			if (w->waitTicks > 200) {
				w->waitTicks = walkerId & 0x1f;
				w->actionState = WalkerActionState_197_HerdAnimalMoving;
				w->destinationX = f->destinationX + WalkerActionFormationLayoutPositionX(FormationLayout_Herd, w->indexInFormation);
				w->destinationY = f->destinationY + WalkerActionFormationLayoutPositionY(FormationLayout_Herd, w->indexInFormation);
				w->roamLength = 0;
			}
			break;
		case WalkerActionState_197_HerdAnimalMoving:
			WalkerMovement_walkTicks(walkerId, 2);
			if (w->direction == 8 || w->direction == 10) {
				w->direction = w->previousTileDirection;
				w->actionState = WalkerActionState_196_HerdAnimalAtRest;
				w->waitTicks = walkerId & 0x1f;
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			}
			break;
	}
	int dir = WalkerActionDirection(w);
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Zebra) + 96 +
			WalkerActionCorpseGraphicOffset(w);
	} else if (w->actionState == WalkerActionState_196_HerdAnimalAtRest) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Zebra) + dir;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_Zebra) + dir + 8 * w->graphicOffset;
	}
}

static void setDestinationHippodromeHorse(int walkerId, struct Data_Walker *w, int state)
{
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	if (state == 0) {
		Walker_removeFromTileList(walkerId);
		if (Data_Settings_Map.orientation == 0 || Data_Settings_Map.orientation == 6) {
			w->destinationX = b->x + hippodromeHorseDestinationX1[w->waitTicksMissile];
			w->destinationY = b->y + hippodromeHorseDestinationY1[w->waitTicksMissile];
		} else {
			w->destinationX = b->x + hippodromeHorseDestinationX2[w->waitTicksMissile];
			w->destinationY = b->y + hippodromeHorseDestinationY2[w->waitTicksMissile];
		}
		w->x = w->destinationX;
		w->y = w->destinationY;
		if (w->resourceId == 1) {
			w->y++;
			w->destinationY++;
		}
		w->crossCountryX = 15 * w->x;
		w->crossCountryY = 15 * w->y;
		w->gridOffset = GridOffset(w->x, w->y);
		Walker_addToTileList(walkerId);
	} else if (state == 1) {
		if (Data_Settings_Map.orientation == 0 || Data_Settings_Map.orientation == 6) {
			w->destinationX = b->x + hippodromeHorseDestinationX1[w->waitTicksMissile];
			w->destinationY = b->y + hippodromeHorseDestinationY1[w->waitTicksMissile];
		} else {
			w->destinationX = b->x + hippodromeHorseDestinationX2[w->waitTicksMissile];
			w->destinationY = b->y + hippodromeHorseDestinationY2[w->waitTicksMissile];
		}
	} else if (state == 2) {
		if (Data_Settings_Map.orientation == 0 || Data_Settings_Map.orientation == 6) {
			if (w->resourceId) {
				w->destinationX = b->x + 1;
				w->destinationY = b->y + 2;
			} else {
				w->destinationX = b->x + 1;
				w->destinationY = b->y + 1;
			}
		} else {
			if (w->resourceId) {
				w->destinationX = b->x + 12;
				w->destinationY = b->y + 3;
			} else {
				w->destinationX = b->x + 12;
				w->destinationY = b->y + 2;
			}
		}
	}
}

void WalkerAction_hippodromeHorse(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	Data_CityInfo.entertainmentHippodromeHasShow = 1;
	w->useCrossCountry = 1;
	w->isGhost = 0;
	WalkerActionIncreaseGraphicOffset(w, 8);

	switch (w->actionState) {
		case WalkerActionState_200_HippodromeMiniHorseCreated:
			w->graphicOffset = 0;
			w->waitTicksMissile = 0;
			setDestinationHippodromeHorse(walkerId, w, 0);
			w->waitTicks++;
			if (w->waitTicks > 60 && w->resourceId == 0) {
				w->actionState = WalkerActionState_201_HippodromeMiniHorseRacing;
				w->waitTicks = 0;
			}
			w->waitTicks++;
			if (w->waitTicks > 20 && w->resourceId == 1) {
				w->actionState = WalkerActionState_201_HippodromeMiniHorseRacing;
				w->waitTicks = 0;
			}
			break;
		case WalkerActionState_201_HippodromeMiniHorseRacing:
			w->direction = Routing_getGeneralDirection(w->x, w->y, w->destinationX, w->destinationY);
			if (w->direction == 8) {
				w->waitTicksMissile++;
				if (w->waitTicksMissile >= 22) {
					w->waitTicksMissile = 0;
					w->inFrontWalkerId++;
					if (w->inFrontWalkerId >= 6) {
						w->waitTicks = 0;
						w->actionState = WalkerActionState_202_HippodromeMiniHorseDone;
					}
					if ((walkerId + Data_Random.random1_7bit) & 1) {
						w->speedMultiplier = 3;
					} else {
						w->speedMultiplier = 4;
					}
				} else if (w->waitTicksMissile == 11) {
					if ((walkerId + Data_Random.random1_7bit) & 1) {
						w->speedMultiplier = 3;
					} else {
						w->speedMultiplier = 4;
					}
				}
				setDestinationHippodromeHorse(walkerId, w, 1);
				w->direction = Routing_getGeneralDirection(w->x, w->y, w->destinationX, w->destinationY);
				WalkerMovement_crossCountrySetDirection(walkerId,
					w->crossCountryX, w->crossCountryY, 15 * w->destinationX, 15 * w->destinationY, 0);
			}
			if (w->actionState != WalkerActionState_202_HippodromeMiniHorseDone) {
				WalkerMovement_crossCountryWalkTicks(walkerId, w->speedMultiplier);
			}
			break;
		case WalkerActionState_202_HippodromeMiniHorseDone:
			if (!w->waitTicks) {
				setDestinationHippodromeHorse(walkerId, w, 2);
				w->direction = Routing_getGeneralDirection(w->x, w->y, w->destinationX, w->destinationY);
				WalkerMovement_crossCountrySetDirection(walkerId,
					w->crossCountryX, w->crossCountryY, 15 * w->destinationX, 15 * w->destinationY, 0);
			}
			if (w->direction != 8) {
				WalkerMovement_crossCountryWalkTicks(walkerId, 1);
			}
			w->waitTicks++;
			if (w->waitTicks > 30) {
				w->graphicOffset = 0;
			}
			w->waitTicks++;
			if (w->waitTicks > 150) {
				w->state = WalkerState_Dead;
			}
			break;
	}

	int dir = WalkerActionDirection(w);
	if (w->resourceId == 0) {
		w->graphicId = GraphicId(ID_Graphic_Walker_HippodromeHorse1) +
			dir + 8 * w->graphicOffset;
		w->cartGraphicId = GraphicId(ID_Graphic_Walker_HippodromeCart1) + dir;
		int cartDir = (dir + 4) % 8;
		WalkerAction_Common_setCartOffset(walkerId, cartDir);
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_HippodromeHorse2) +
			dir + 8 * w->graphicOffset;
		w->cartGraphicId = GraphicId(ID_Graphic_Walker_HippodromeCart2) + dir;
		int cartDir = (dir + 4) % 8;
		WalkerAction_Common_setCartOffset(walkerId, cartDir);
	}
}

void WalkerAction_HippodromeHorse_reroute()
{
	if (!Data_CityInfo.entertainmentHippodromeHasShow) {
		return;
	}
	for (int i = 1; i < MAX_WALKERS; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state == WalkerState_Alive && w->type == Walker_HippodromeMiniHorses) {
			w->waitTicksMissile = 0;
			setDestinationHippodromeHorse(i, w, 0);
		}
	}
}
