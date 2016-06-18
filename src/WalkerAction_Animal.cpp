#include "FigureAction_private.h"

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

enum {
	HippodromeHorse_Created = 0,
	HippodromeHorse_Racing = 1,
	HippodromeHorse_Finished = 2
};

void FigureAction_seagulls(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = FigureTerrainUsage_Any;
	w->isGhost = 0;
	w->useCrossCountry = 1;
	if (!(w->graphicOffset & 3) && FigureMovement_crossCountryWalkTicks(walkerId, 1)) {
		w->progressOnTile++;
		if (w->progressOnTile > 8) {
			w->progressOnTile = 0;
		}
		FigureAction_Common_setCrossCountryDestination(walkerId, w,
			w->sourceX + seagullOffsetsX[w->progressOnTile],
			w->sourceY + seagullOffsetsY[w->progressOnTile]);
	}
	if (walkerId & 1) {
		FigureActionIncreaseGraphicOffset(w, 54);
		w->graphicId = GraphicId(ID_Graphic_Figure_Seagulls) + w->graphicOffset / 3;
	} else {
		FigureActionIncreaseGraphicOffset(w, 72);
		w->graphicId = GraphicId(ID_Graphic_Figure_Seagulls) + 18 + w->graphicOffset / 3;
	}
}

void FigureAction_sheep(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[w->formationId];
	w->terrainUsage = FigureTerrainUsage_Animal;
	w->useCrossCountry = 0;
	w->isGhost = 0;
	Data_CityInfo.numAnimalsInCity++;
	FigureActionIncreaseGraphicOffset(w, 6);

	switch (w->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_196_HerdAnimalAtRest:
			w->waitTicks++;
			if (w->waitTicks > 400) {
				w->waitTicks = walkerId & 0x1f;
				w->actionState = FigureActionState_197_HerdAnimalMoving;
				w->destinationX = f->destinationX + WalkerActionFormationLayoutPositionX(FormationLayout_Herd, w->indexInFormation);
				w->destinationY = f->destinationY + WalkerActionFormationLayoutPositionY(FormationLayout_Herd, w->indexInFormation);
				w->roamLength = 0;
			}
			break;
		case FigureActionState_197_HerdAnimalMoving:
			FigureMovement_walkTicks(walkerId, 1);
			if (w->direction == DirFigure_8_AtDestination || w->direction == DirFigure_10_Lost) {
				w->direction = w->previousTileDirection;
				w->actionState = FigureActionState_196_HerdAnimalAtRest;
				w->waitTicks = walkerId & 0x1f;
			} else if (w->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
			}
			break;
	}
	int dir = WalkerActionDirection(w);
	if (w->actionState == FigureActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Figure_Sheep) + 104 +
			WalkerActionCorpseGraphicOffset(w);
	} else if (w->actionState == FigureActionState_196_HerdAnimalAtRest) {
		if (walkerId & 3) {
			w->graphicId = GraphicId(ID_Graphic_Figure_Sheep) + 48 + dir +
				8 * sheepGraphicOffsets[w->waitTicks & 0x3f];
		} else {
			w->graphicId = GraphicId(ID_Graphic_Figure_Sheep) + 96 + dir;
		}
	} else {
		w->graphicId = GraphicId(ID_Graphic_Figure_Sheep) + dir + 8 * w->graphicOffset;
	}
}

void FigureAction_wolf(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[w->formationId];
	w->terrainUsage = FigureTerrainUsage_Animal;
	w->useCrossCountry = 0;
	w->isGhost = 0;
	Data_CityInfo.numAnimalsInCity++;
	FigureActionIncreaseGraphicOffset(w, 12);

	switch (w->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_196_HerdAnimalAtRest:
			w->waitTicks++;
			if (w->waitTicks > 400) {
				w->waitTicks = walkerId & 0x1f;
				w->actionState = FigureActionState_197_HerdAnimalMoving;
				w->destinationX = f->destinationX + WalkerActionFormationLayoutPositionX(FormationLayout_Herd, w->indexInFormation);
				w->destinationY = f->destinationY + WalkerActionFormationLayoutPositionY(FormationLayout_Herd, w->indexInFormation);
				w->roamLength = 0;
			}
			break;
		case FigureActionState_197_HerdAnimalMoving:
			FigureMovement_walkTicks(walkerId, 2);
			if (w->direction == DirFigure_8_AtDestination || w->direction == DirFigure_10_Lost) {
				w->direction = w->previousTileDirection;
				w->actionState = FigureActionState_196_HerdAnimalAtRest;
				w->waitTicks = walkerId & 0x1f;
			} else if (w->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
			}
			break;
		case FigureActionState_199_WolfAttacking:
			FigureMovement_walkTicks(walkerId, 2);
			if (w->direction == DirFigure_8_AtDestination) {
				int targetId = FigureAction_CombatWolf_getTarget(w->x, w->y, 6);
				if (targetId) {
					w->destinationX = Data_Walkers[targetId].x;
					w->destinationY = Data_Walkers[targetId].y;
					w->targetWalkerId = targetId;
					Data_Walkers[targetId].targetedByWalkerId = walkerId;
					w->targetWalkerCreatedSequence = Data_Walkers[targetId].createdSequence;
					FigureRoute_remove(walkerId);
				} else {
					w->direction = w->previousTileDirection;
					w->actionState = FigureActionState_196_HerdAnimalAtRest;
					w->waitTicks = walkerId & 0x1f;
				}
			} else if (w->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
			} else if (w->direction == DirFigure_10_Lost) {
				w->direction = w->previousTileDirection;
				w->actionState = FigureActionState_196_HerdAnimalAtRest;
				w->waitTicks = walkerId & 0x1f;
			}
			break;
	}
	int dir = WalkerActionDirection(w);
	if (w->actionState == FigureActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Figure_Wolf) + 96 +
			WalkerActionCorpseGraphicOffset(w);
	} else if (w->actionState == FigureActionState_150_Attack) {
		w->graphicId = GraphicId(ID_Graphic_Figure_Wolf) + 104 +
			dir + 8 * (w->attackGraphicOffset / 4);
	} else if (w->actionState == FigureActionState_196_HerdAnimalAtRest) {
		w->graphicId = GraphicId(ID_Graphic_Figure_Wolf) + 152 + dir;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Figure_Wolf) + dir + 8 * w->graphicOffset;
	}
}

void FigureAction_zebra(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[w->formationId];
	w->terrainUsage = FigureTerrainUsage_Animal;
	w->useCrossCountry = 0;
	w->isGhost = 0;
	Data_CityInfo.numAnimalsInCity++;
	FigureActionIncreaseGraphicOffset(w, 12);

	switch (w->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_196_HerdAnimalAtRest:
			w->waitTicks++;
			if (w->waitTicks > 200) {
				w->waitTicks = walkerId & 0x1f;
				w->actionState = FigureActionState_197_HerdAnimalMoving;
				w->destinationX = f->destinationX + WalkerActionFormationLayoutPositionX(FormationLayout_Herd, w->indexInFormation);
				w->destinationY = f->destinationY + WalkerActionFormationLayoutPositionY(FormationLayout_Herd, w->indexInFormation);
				w->roamLength = 0;
			}
			break;
		case FigureActionState_197_HerdAnimalMoving:
			FigureMovement_walkTicks(walkerId, 2);
			if (w->direction == DirFigure_8_AtDestination || w->direction == DirFigure_10_Lost) {
				w->direction = w->previousTileDirection;
				w->actionState = FigureActionState_196_HerdAnimalAtRest;
				w->waitTicks = walkerId & 0x1f;
			} else if (w->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
			}
			break;
	}
	int dir = WalkerActionDirection(w);
	if (w->actionState == FigureActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Figure_Zebra) + 96 +
			WalkerActionCorpseGraphicOffset(w);
	} else if (w->actionState == FigureActionState_196_HerdAnimalAtRest) {
		w->graphicId = GraphicId(ID_Graphic_Figure_Zebra) + dir;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Figure_Zebra) + dir + 8 * w->graphicOffset;
	}
}

static void setDestinationHippodromeHorse(int walkerId, struct Data_Walker *w, int state)
{
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	if (state == HippodromeHorse_Created) {
		Figure_removeFromTileList(walkerId);
		if (Data_Settings_Map.orientation == Dir_0_Top || Data_Settings_Map.orientation == Dir_6_Left) {
			w->destinationX = b->x + hippodromeHorseDestinationX1[w->waitTicksMissile];
			w->destinationY = b->y + hippodromeHorseDestinationY1[w->waitTicksMissile];
		} else {
			w->destinationX = b->x + hippodromeHorseDestinationX2[w->waitTicksMissile];
			w->destinationY = b->y + hippodromeHorseDestinationY2[w->waitTicksMissile];
		}
		if (w->resourceId == 1) {
			w->destinationY++;
		}
		w->x = w->destinationX;
		w->y = w->destinationY;
		w->crossCountryX = 15 * w->x;
		w->crossCountryY = 15 * w->y;
		w->gridOffset = GridOffset(w->x, w->y);
		Figure_addToTileList(walkerId);
	} else if (state == HippodromeHorse_Racing) {
		if (Data_Settings_Map.orientation == Dir_0_Top || Data_Settings_Map.orientation == Dir_6_Left) {
			w->destinationX = b->x + hippodromeHorseDestinationX1[w->waitTicksMissile];
			w->destinationY = b->y + hippodromeHorseDestinationY1[w->waitTicksMissile];
		} else {
			w->destinationX = b->x + hippodromeHorseDestinationX2[w->waitTicksMissile];
			w->destinationY = b->y + hippodromeHorseDestinationY2[w->waitTicksMissile];
		}
	} else if (state == HippodromeHorse_Finished) {
		if (Data_Settings_Map.orientation == Dir_0_Top || Data_Settings_Map.orientation == Dir_6_Left) {
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

void FigureAction_hippodromeHorse(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	Data_CityInfo.entertainmentHippodromeHasShow = 1;
	w->useCrossCountry = 1;
	w->isGhost = 0;
	FigureActionIncreaseGraphicOffset(w, 8);

	switch (w->actionState) {
		case FigureActionState_200_HippodromeMiniHorseCreated:
			w->graphicOffset = 0;
			w->waitTicksMissile = 0;
			setDestinationHippodromeHorse(walkerId, w, HippodromeHorse_Created);
			w->waitTicks++;
			if (w->waitTicks > 60 && w->resourceId == 0) {
				w->actionState = FigureActionState_201_HippodromeMiniHorseRacing;
				w->waitTicks = 0;
			}
			w->waitTicks++;
			if (w->waitTicks > 20 && w->resourceId == 1) {
				w->actionState = FigureActionState_201_HippodromeMiniHorseRacing;
				w->waitTicks = 0;
			}
			break;
		case FigureActionState_201_HippodromeMiniHorseRacing:
			w->direction = Routing_getGeneralDirection(w->x, w->y, w->destinationX, w->destinationY);
			if (w->direction == DirFigure_8_AtDestination) {
				w->waitTicksMissile++;
				if (w->waitTicksMissile >= 22) {
					w->waitTicksMissile = 0;
					w->inFrontWalkerId++;
					if (w->inFrontWalkerId >= 6) {
						w->waitTicks = 0;
						w->actionState = FigureActionState_202_HippodromeMiniHorseDone;
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
				setDestinationHippodromeHorse(walkerId, w, HippodromeHorse_Racing);
				w->direction = Routing_getGeneralDirection(w->x, w->y, w->destinationX, w->destinationY);
				FigureMovement_crossCountrySetDirection(walkerId,
					w->crossCountryX, w->crossCountryY, 15 * w->destinationX, 15 * w->destinationY, 0);
			}
			if (w->actionState != FigureActionState_202_HippodromeMiniHorseDone) {
				FigureMovement_crossCountryWalkTicks(walkerId, w->speedMultiplier);
			}
			break;
		case FigureActionState_202_HippodromeMiniHorseDone:
			if (!w->waitTicks) {
				setDestinationHippodromeHorse(walkerId, w, HippodromeHorse_Finished);
				w->direction = Routing_getGeneralDirection(w->x, w->y, w->destinationX, w->destinationY);
				FigureMovement_crossCountrySetDirection(walkerId,
					w->crossCountryX, w->crossCountryY, 15 * w->destinationX, 15 * w->destinationY, 0);
			}
			if (w->direction != DirFigure_8_AtDestination) {
				FigureMovement_crossCountryWalkTicks(walkerId, 1);
			}
			w->waitTicks++;
			if (w->waitTicks > 30) {
				w->graphicOffset = 0;
			}
			w->waitTicks++;
			if (w->waitTicks > 150) {
				w->state = FigureState_Dead;
			}
			break;
	}

	int dir = WalkerActionDirection(w);
	if (w->resourceId == 0) {
		w->graphicId = GraphicId(ID_Graphic_Figure_HippodromeHorse1) +
			dir + 8 * w->graphicOffset;
		w->cartGraphicId = GraphicId(ID_Graphic_Figure_HippodromeCart1) + dir;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Figure_HippodromeHorse2) +
			dir + 8 * w->graphicOffset;
		w->cartGraphicId = GraphicId(ID_Graphic_Figure_HippodromeCart2) + dir;
	}
	int cartDir = (dir + 4) % 8;
	FigureAction_Common_setCartOffset(walkerId, cartDir);
}

void FigureAction_HippodromeHorse_reroute()
{
	if (!Data_CityInfo.entertainmentHippodromeHasShow) {
		return;
	}
	for (int i = 1; i < MAX_FIGURES; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state == FigureState_Alive && w->type == Figure_HippodromeMiniHorses) {
			w->waitTicksMissile = 0;
			setDestinationHippodromeHorse(i, w, HippodromeHorse_Created);
		}
	}
}
