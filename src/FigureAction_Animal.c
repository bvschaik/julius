#include "FigureAction_private.h"

#include "Figure.h"
#include "Routing.h"

#include "Data/CityInfo.h"

#include "core/random.h"
#include "figure/formation.h"

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

void FigureAction_seagulls(int figureId)
{
	struct Data_Figure *f = &Data_Figures[figureId];
	f->terrainUsage = FigureTerrainUsage_Any;
	f->isGhost = 0;
	f->useCrossCountry = 1;
	if (!(f->graphicOffset & 3) && FigureMovement_crossCountryWalkTicks(figureId, 1)) {
		f->progressOnTile++;
		if (f->progressOnTile > 8) {
			f->progressOnTile = 0;
		}
		FigureAction_Common_setCrossCountryDestination(figureId, f,
			f->sourceX + seagullOffsetsX[f->progressOnTile],
			f->sourceY + seagullOffsetsY[f->progressOnTile]);
	}
	if (figureId & 1) {
		FigureActionIncreaseGraphicOffset(f, 54);
		f->graphicId = GraphicId(ID_Graphic_Figure_Seagulls) + f->graphicOffset / 3;
	} else {
		FigureActionIncreaseGraphicOffset(f, 72);
		f->graphicId = GraphicId(ID_Graphic_Figure_Seagulls) + 18 + f->graphicOffset / 3;
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
			FigureAction_Common_handleAttack(figureId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(figureId);
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
			FigureMovement_walkTicks(figureId, 1);
			if (f->direction == DirFigure_8_AtDestination || f->direction == DirFigure_10_Lost) {
				f->direction = f->previousTileDirection;
				f->actionState = FigureActionState_196_HerdAnimalAtRest;
				f->waitTicks = figureId & 0x1f;
			} else if (f->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(figureId);
			}
			break;
	}
	int dir = FigureActionDirection(f);
	if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = GraphicId(ID_Graphic_Figure_Sheep) + 104 +
			FigureActionCorpseGraphicOffset(f);
	} else if (f->actionState == FigureActionState_196_HerdAnimalAtRest) {
		if (figureId & 3) {
			f->graphicId = GraphicId(ID_Graphic_Figure_Sheep) + 48 + dir +
				8 * sheepGraphicOffsets[f->waitTicks & 0x3f];
		} else {
			f->graphicId = GraphicId(ID_Graphic_Figure_Sheep) + 96 + dir;
		}
	} else {
		f->graphicId = GraphicId(ID_Graphic_Figure_Sheep) + dir + 8 * f->graphicOffset;
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
			FigureAction_Common_handleAttack(figureId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(figureId);
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
			FigureMovement_walkTicks(figureId, 2);
			if (f->direction == DirFigure_8_AtDestination || f->direction == DirFigure_10_Lost) {
				f->direction = f->previousTileDirection;
				f->actionState = FigureActionState_196_HerdAnimalAtRest;
				f->waitTicks = figureId & 0x1f;
			} else if (f->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(figureId);
			}
			break;
		case FigureActionState_199_WolfAttacking:
			FigureMovement_walkTicks(figureId, 2);
			if (f->direction == DirFigure_8_AtDestination) {
				int targetId = FigureAction_CombatWolf_getTarget(f->x, f->y, 6);
				if (targetId) {
					f->destinationX = Data_Figures[targetId].x;
					f->destinationY = Data_Figures[targetId].y;
					f->targetFigureId = targetId;
					Data_Figures[targetId].targetedByFigureId = figureId;
					f->targetFigureCreatedSequence = Data_Figures[targetId].createdSequence;
					FigureRoute_remove(figureId);
				} else {
					f->direction = f->previousTileDirection;
					f->actionState = FigureActionState_196_HerdAnimalAtRest;
					f->waitTicks = figureId & 0x1f;
				}
			} else if (f->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(figureId);
			} else if (f->direction == DirFigure_10_Lost) {
				f->direction = f->previousTileDirection;
				f->actionState = FigureActionState_196_HerdAnimalAtRest;
				f->waitTicks = figureId & 0x1f;
			}
			break;
	}
	int dir = FigureActionDirection(f);
	if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = GraphicId(ID_Graphic_Figure_Wolf) + 96 +
			FigureActionCorpseGraphicOffset(f);
	} else if (f->actionState == FigureActionState_150_Attack) {
		f->graphicId = GraphicId(ID_Graphic_Figure_Wolf) + 104 +
			dir + 8 * (f->attackGraphicOffset / 4);
	} else if (f->actionState == FigureActionState_196_HerdAnimalAtRest) {
		f->graphicId = GraphicId(ID_Graphic_Figure_Wolf) + 152 + dir;
	} else {
		f->graphicId = GraphicId(ID_Graphic_Figure_Wolf) + dir + 8 * f->graphicOffset;
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
			FigureAction_Common_handleAttack(figureId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(figureId);
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
			FigureMovement_walkTicks(figureId, 2);
			if (f->direction == DirFigure_8_AtDestination || f->direction == DirFigure_10_Lost) {
				f->direction = f->previousTileDirection;
				f->actionState = FigureActionState_196_HerdAnimalAtRest;
				f->waitTicks = figureId & 0x1f;
			} else if (f->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(figureId);
			}
			break;
	}
	int dir = FigureActionDirection(f);
	if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = GraphicId(ID_Graphic_Figure_Zebra) + 96 +
			FigureActionCorpseGraphicOffset(f);
	} else if (f->actionState == FigureActionState_196_HerdAnimalAtRest) {
		f->graphicId = GraphicId(ID_Graphic_Figure_Zebra) + dir;
	} else {
		f->graphicId = GraphicId(ID_Graphic_Figure_Zebra) + dir + 8 * f->graphicOffset;
	}
}

static void setDestinationHippodromeHorse(int figureId, struct Data_Figure *f, int state)
{
	struct Data_Building *b = &Data_Buildings[f->buildingId];
	if (state == HippodromeHorse_Created) {
		Figure_removeFromTileList(figureId);
		if (Data_Settings_Map.orientation == Dir_0_Top || Data_Settings_Map.orientation == Dir_6_Left) {
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
		Figure_addToTileList(figureId);
	} else if (state == HippodromeHorse_Racing) {
		if (Data_Settings_Map.orientation == Dir_0_Top || Data_Settings_Map.orientation == Dir_6_Left) {
			f->destinationX = b->x + hippodromeHorseDestinationX1[f->waitTicksMissile];
			f->destinationY = b->y + hippodromeHorseDestinationY1[f->waitTicksMissile];
		} else {
			f->destinationX = b->x + hippodromeHorseDestinationX2[f->waitTicksMissile];
			f->destinationY = b->y + hippodromeHorseDestinationY2[f->waitTicksMissile];
		}
	} else if (state == HippodromeHorse_Finished) {
		if (Data_Settings_Map.orientation == Dir_0_Top || Data_Settings_Map.orientation == Dir_6_Left) {
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
			setDestinationHippodromeHorse(figureId, f, HippodromeHorse_Created);
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
			f->direction = Routing_getGeneralDirection(f->x, f->y, f->destinationX, f->destinationY);
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
				setDestinationHippodromeHorse(figureId, f, HippodromeHorse_Racing);
				f->direction = Routing_getGeneralDirection(f->x, f->y, f->destinationX, f->destinationY);
				FigureMovement_crossCountrySetDirection(figureId,
					f->crossCountryX, f->crossCountryY, 15 * f->destinationX, 15 * f->destinationY, 0);
			}
			if (f->actionState != FigureActionState_202_HippodromeMiniHorseDone) {
				FigureMovement_crossCountryWalkTicks(figureId, f->speedMultiplier);
			}
			break;
		case FigureActionState_202_HippodromeMiniHorseDone:
			if (!f->waitTicks) {
				setDestinationHippodromeHorse(figureId, f, HippodromeHorse_Finished);
				f->direction = Routing_getGeneralDirection(f->x, f->y, f->destinationX, f->destinationY);
				FigureMovement_crossCountrySetDirection(figureId,
					f->crossCountryX, f->crossCountryY, 15 * f->destinationX, 15 * f->destinationY, 0);
			}
			if (f->direction != DirFigure_8_AtDestination) {
				FigureMovement_crossCountryWalkTicks(figureId, 1);
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
		f->graphicId = GraphicId(ID_Graphic_Figure_HippodromeHorse1) +
			dir + 8 * f->graphicOffset;
		f->cartGraphicId = GraphicId(ID_Graphic_Figure_HippodromeCart1) + dir;
	} else {
		f->graphicId = GraphicId(ID_Graphic_Figure_HippodromeHorse2) +
			dir + 8 * f->graphicOffset;
		f->cartGraphicId = GraphicId(ID_Graphic_Figure_HippodromeCart2) + dir;
	}
	int cartDir = (dir + 4) % 8;
	FigureAction_Common_setCartOffset(figureId, cartDir);
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
			setDestinationHippodromeHorse(i, f, HippodromeHorse_Created);
		}
	}
}
