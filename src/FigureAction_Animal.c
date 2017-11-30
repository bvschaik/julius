#include "FigureAction_private.h"

#include "Figure.h"

#include "Data/CityInfo.h"

#include "core/calc.h"
#include "core/random.h"
#include "figure/formation.h"
#include "figure/route.h"

static const int seagullOffsetsX[] = {0, 0, -2, 1, 2, -3, 4, -2, 0};
static const int seagullOffsetsY[] = {0, -2, 0, 2, 0, 1, -3, 4, 0};

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

void FigureAction_seagulls(int figureId)
{
	struct Data_Figure *f = &Data_Figures[figureId];
	f->terrainUsage = FigureTerrainUsage_Any;
	f->isGhost = 0;
	f->useCrossCountry = 1;
	if (!(f->graphicOffset & 3) && FigureMovement_crossCountryWalkTicks(f, 1)) {
		f->progressOnTile++;
		if (f->progressOnTile > 8) {
			f->progressOnTile = 0;
		}
		FigureAction_Common_setCrossCountryDestination(f,
			f->sourceX + seagullOffsetsX[f->progressOnTile],
			f->sourceY + seagullOffsetsY[f->progressOnTile]);
	}
	if (figureId & 1) {
		FigureActionIncreaseGraphicOffset(f, 54);
		f->graphicId = image_group(GROUP_FIGURE_SEAGULLS) + f->graphicOffset / 3;
	} else {
		FigureActionIncreaseGraphicOffset(f, 72);
		f->graphicId = image_group(GROUP_FIGURE_SEAGULLS) + 18 + f->graphicOffset / 3;
	}
}

void FigureAction_sheep(int figureId)
{
	struct Data_Figure *f = &Data_Figures[figureId];
	const formation *m = formation_get(f->formationId);
	f->terrainUsage = FigureTerrainUsage_Animal;
	f->useCrossCountry = 0;
	f->isGhost = 0;
	Data_CityInfo.numAnimalsInCity++;
	FigureActionIncreaseGraphicOffset(f, 6);

	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(f);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(f);
			break;
		case FigureActionState_196_HerdAnimalAtRest:
			f->waitTicks++;
			if (f->waitTicks > 400) {
				f->waitTicks = figureId & 0x1f;
				f->actionState = FigureActionState_197_HerdAnimalMoving;
				f->destinationX = m->destination_x + FigureActionFormationLayoutPositionX(FORMATION_HERD, f->indexInFormation);
				f->destinationY = m->destination_y + FigureActionFormationLayoutPositionY(FORMATION_HERD, f->indexInFormation);
				f->roamLength = 0;
			}
			break;
		case FigureActionState_197_HerdAnimalMoving:
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DirFigure_8_AtDestination || f->direction == DirFigure_10_Lost) {
				f->direction = f->previousTileDirection;
				f->actionState = FigureActionState_196_HerdAnimalAtRest;
				f->waitTicks = figureId & 0x1f;
			} else if (f->direction == DirFigure_9_Reroute) {
				figure_route_remove(f);
			}
			break;
	}
	int dir = FigureActionDirection(f);
	if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = image_group(GROUP_FIGURE_SHEEP) + 104 +
			FigureActionCorpseGraphicOffset(f);
	} else if (f->actionState == FigureActionState_196_HerdAnimalAtRest) {
		if (figureId & 3) {
			f->graphicId = image_group(GROUP_FIGURE_SHEEP) + 48 + dir +
				8 * sheepGraphicOffsets[f->waitTicks & 0x3f];
		} else {
			f->graphicId = image_group(GROUP_FIGURE_SHEEP) + 96 + dir;
		}
	} else {
		f->graphicId = image_group(GROUP_FIGURE_SHEEP) + dir + 8 * f->graphicOffset;
	}
}

void FigureAction_wolf(int figureId)
{
	struct Data_Figure *f = &Data_Figures[figureId];
	const formation *m = formation_get(f->formationId);
	f->terrainUsage = FigureTerrainUsage_Animal;
	f->useCrossCountry = 0;
	f->isGhost = 0;
	Data_CityInfo.numAnimalsInCity++;
	FigureActionIncreaseGraphicOffset(f, 12);

	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(f);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(f);
			break;
		case FigureActionState_196_HerdAnimalAtRest:
			f->waitTicks++;
			if (f->waitTicks > 400) {
				f->waitTicks = figureId & 0x1f;
				f->actionState = FigureActionState_197_HerdAnimalMoving;
				f->destinationX = m->destination_x + FigureActionFormationLayoutPositionX(FORMATION_HERD, f->indexInFormation);
				f->destinationY = m->destination_y + FigureActionFormationLayoutPositionY(FORMATION_HERD, f->indexInFormation);
				f->roamLength = 0;
			}
			break;
		case FigureActionState_197_HerdAnimalMoving:
			FigureMovement_walkTicks(f, 2);
			if (f->direction == DirFigure_8_AtDestination || f->direction == DirFigure_10_Lost) {
				f->direction = f->previousTileDirection;
				f->actionState = FigureActionState_196_HerdAnimalAtRest;
				f->waitTicks = figureId & 0x1f;
			} else if (f->direction == DirFigure_9_Reroute) {
				figure_route_remove(f);
			}
			break;
		case FigureActionState_199_WolfAttacking:
			FigureMovement_walkTicks(f, 2);
			if (f->direction == DirFigure_8_AtDestination) {
				int targetId = FigureAction_CombatWolf_getTarget(f->x, f->y, 6);
				if (targetId) {
					f->destinationX = Data_Figures[targetId].x;
					f->destinationY = Data_Figures[targetId].y;
					f->targetFigureId = targetId;
					Data_Figures[targetId].targetedByFigureId = figureId;
					f->targetFigureCreatedSequence = Data_Figures[targetId].createdSequence;
					figure_route_remove(f);
				} else {
					f->direction = f->previousTileDirection;
					f->actionState = FigureActionState_196_HerdAnimalAtRest;
					f->waitTicks = figureId & 0x1f;
				}
			} else if (f->direction == DirFigure_9_Reroute) {
				figure_route_remove(f);
			} else if (f->direction == DirFigure_10_Lost) {
				f->direction = f->previousTileDirection;
				f->actionState = FigureActionState_196_HerdAnimalAtRest;
				f->waitTicks = figureId & 0x1f;
			}
			break;
	}
	int dir = FigureActionDirection(f);
	if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = image_group(GROUP_FIGURE_WOLF) + 96 +
			FigureActionCorpseGraphicOffset(f);
	} else if (f->actionState == FigureActionState_150_Attack) {
		f->graphicId = image_group(GROUP_FIGURE_WOLF) + 104 +
			dir + 8 * (f->attackGraphicOffset / 4);
	} else if (f->actionState == FigureActionState_196_HerdAnimalAtRest) {
		f->graphicId = image_group(GROUP_FIGURE_WOLF) + 152 + dir;
	} else {
		f->graphicId = image_group(GROUP_FIGURE_WOLF) + dir + 8 * f->graphicOffset;
	}
}

void FigureAction_zebra(int figureId)
{
	struct Data_Figure *f = &Data_Figures[figureId];
	const formation *m = formation_get(f->formationId);
	f->terrainUsage = FigureTerrainUsage_Animal;
	f->useCrossCountry = 0;
	f->isGhost = 0;
	Data_CityInfo.numAnimalsInCity++;
	FigureActionIncreaseGraphicOffset(f, 12);

	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(f);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(f);
			break;
		case FigureActionState_196_HerdAnimalAtRest:
			f->waitTicks++;
			if (f->waitTicks > 200) {
				f->waitTicks = figureId & 0x1f;
				f->actionState = FigureActionState_197_HerdAnimalMoving;
				f->destinationX = m->destination_x + FigureActionFormationLayoutPositionX(FORMATION_HERD, f->indexInFormation);
				f->destinationY = m->destination_y + FigureActionFormationLayoutPositionY(FORMATION_HERD, f->indexInFormation);
				f->roamLength = 0;
			}
			break;
		case FigureActionState_197_HerdAnimalMoving:
			FigureMovement_walkTicks(f, 2);
			if (f->direction == DirFigure_8_AtDestination || f->direction == DirFigure_10_Lost) {
				f->direction = f->previousTileDirection;
				f->actionState = FigureActionState_196_HerdAnimalAtRest;
				f->waitTicks = figureId & 0x1f;
			} else if (f->direction == DirFigure_9_Reroute) {
				figure_route_remove(f);
			}
			break;
	}
	int dir = FigureActionDirection(f);
	if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = image_group(GROUP_FIGURE_ZEBRA) + 96 +
			FigureActionCorpseGraphicOffset(f);
	} else if (f->actionState == FigureActionState_196_HerdAnimalAtRest) {
		f->graphicId = image_group(GROUP_FIGURE_ZEBRA) + dir;
	} else {
		f->graphicId = image_group(GROUP_FIGURE_ZEBRA) + dir + 8 * f->graphicOffset;
	}
}

static void setDestinationHippodromeHorse(figure *f, int state)
{
	struct Data_Building *b = &Data_Buildings[f->buildingId];
	if (state == HippodromeHorse_Created) {
		Figure_removeFromTileList(f->id);
		if (Data_State.map.orientation == DIR_0_TOP || Data_State.map.orientation == DIR_6_LEFT) {
			f->destinationX = b->x + hippodromeHorseDestinationX1[f->waitTicksMissile];
			f->destinationY = b->y + hippodromeHorseDestinationY1[f->waitTicksMissile];
		} else {
			f->destinationX = b->x + hippodromeHorseDestinationX2[f->waitTicksMissile];
			f->destinationY = b->y + hippodromeHorseDestinationY2[f->waitTicksMissile];
		}
		if (f->resourceId == 1) {
			f->destinationY++;
		}
		f->x = f->destinationX;
		f->y = f->destinationY;
		f->crossCountryX = 15 * f->x;
		f->crossCountryY = 15 * f->y;
		f->gridOffset = GridOffset(f->x, f->y);
		Figure_addToTileList(f->id);
	} else if (state == HippodromeHorse_Racing) {
		if (Data_State.map.orientation == DIR_0_TOP || Data_State.map.orientation == DIR_6_LEFT) {
			f->destinationX = b->x + hippodromeHorseDestinationX1[f->waitTicksMissile];
			f->destinationY = b->y + hippodromeHorseDestinationY1[f->waitTicksMissile];
		} else {
			f->destinationX = b->x + hippodromeHorseDestinationX2[f->waitTicksMissile];
			f->destinationY = b->y + hippodromeHorseDestinationY2[f->waitTicksMissile];
		}
	} else if (state == HippodromeHorse_Finished) {
		if (Data_State.map.orientation == DIR_0_TOP || Data_State.map.orientation == DIR_6_LEFT) {
			if (f->resourceId) {
				f->destinationX = b->x + 1;
				f->destinationY = b->y + 2;
			} else {
				f->destinationX = b->x + 1;
				f->destinationY = b->y + 1;
			}
		} else {
			if (f->resourceId) {
				f->destinationX = b->x + 12;
				f->destinationY = b->y + 3;
			} else {
				f->destinationX = b->x + 12;
				f->destinationY = b->y + 2;
			}
		}
	}
}

void FigureAction_hippodromeHorse(int figureId)
{
	struct Data_Figure *f = &Data_Figures[figureId];
	Data_CityInfo.entertainmentHippodromeHasShow = 1;
	f->useCrossCountry = 1;
	f->isGhost = 0;
	FigureActionIncreaseGraphicOffset(f, 8);

	switch (f->actionState) {
		case FigureActionState_200_HippodromeMiniHorseCreated:
			f->graphicOffset = 0;
			f->waitTicksMissile = 0;
			setDestinationHippodromeHorse(f, HippodromeHorse_Created);
			f->waitTicks++;
			if (f->waitTicks > 60 && f->resourceId == 0) {
				f->actionState = FigureActionState_201_HippodromeMiniHorseRacing;
				f->waitTicks = 0;
			}
			f->waitTicks++;
			if (f->waitTicks > 20 && f->resourceId == 1) {
				f->actionState = FigureActionState_201_HippodromeMiniHorseRacing;
				f->waitTicks = 0;
			}
			break;
		case FigureActionState_201_HippodromeMiniHorseRacing:
			f->direction = calc_general_direction(f->x, f->y, f->destinationX, f->destinationY);
			if (f->direction == DirFigure_8_AtDestination) {
				f->waitTicksMissile++;
				if (f->waitTicksMissile >= 22) {
					f->waitTicksMissile = 0;
					f->inFrontFigureId++;
					if (f->inFrontFigureId >= 6) {
						f->waitTicks = 0;
						f->actionState = FigureActionState_202_HippodromeMiniHorseDone;
					}
					if ((figureId + random_byte()) & 1) {
						f->speedMultiplier = 3;
					} else {
						f->speedMultiplier = 4;
					}
				} else if (f->waitTicksMissile == 11) {
					if ((figureId + random_byte()) & 1) {
						f->speedMultiplier = 3;
					} else {
						f->speedMultiplier = 4;
					}
				}
				setDestinationHippodromeHorse(f, HippodromeHorse_Racing);
				f->direction = calc_general_direction(f->x, f->y, f->destinationX, f->destinationY);
				FigureMovement_crossCountrySetDirection(f,
					f->crossCountryX, f->crossCountryY, 15 * f->destinationX, 15 * f->destinationY, 0);
			}
			if (f->actionState != FigureActionState_202_HippodromeMiniHorseDone) {
				FigureMovement_crossCountryWalkTicks(f, f->speedMultiplier);
			}
			break;
		case FigureActionState_202_HippodromeMiniHorseDone:
			if (!f->waitTicks) {
				setDestinationHippodromeHorse(f, HippodromeHorse_Finished);
				f->direction = calc_general_direction(f->x, f->y, f->destinationX, f->destinationY);
				FigureMovement_crossCountrySetDirection(f,
					f->crossCountryX, f->crossCountryY, 15 * f->destinationX, 15 * f->destinationY, 0);
			}
			if (f->direction != DirFigure_8_AtDestination) {
				FigureMovement_crossCountryWalkTicks(f, 1);
			}
			f->waitTicks++;
			if (f->waitTicks > 30) {
				f->graphicOffset = 0;
			}
			f->waitTicks++;
			if (f->waitTicks > 150) {
				f->state = FigureState_Dead;
			}
			break;
	}

	int dir = FigureActionDirection(f);
	if (f->resourceId == 0) {
		f->graphicId = image_group(GROUP_FIGURE_HIPPODROME_HORSE_1) +
			dir + 8 * f->graphicOffset;
		f->cartGraphicId = image_group(GROUP_FIGURE_HIPPODROME_CART_1) + dir;
	} else {
		f->graphicId = image_group(GROUP_FIGURE_HIPPODROME_HORSE_2) +
			dir + 8 * f->graphicOffset;
		f->cartGraphicId = image_group(GROUP_FIGURE_HIPPODROME_CART_2) + dir;
	}
	int cartDir = (dir + 4) % 8;
	FigureAction_Common_setCartOffset(f, cartDir);
}

void FigureAction_HippodromeHorse_reroute()
{
	if (!Data_CityInfo.entertainmentHippodromeHasShow) {
		return;
	}
	for (int i = 1; i < MAX_FIGURES; i++) {
		struct Data_Figure *f = &Data_Figures[i];
		if (f->state == FigureState_Alive && f->type == FIGURE_HIPPODROME_HORSES) {
			f->waitTicksMissile = 0;
			setDestinationHippodromeHorse(f, HippodromeHorse_Created);
		}
	}
}
