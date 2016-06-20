#include "FigureAction_private.h"

#include "Figure.h"
#include "Routing.h"

#include "Data/CityInfo.h"
#include "Data/Formation.h"
#include "Data/Grid.h"

static const struct {
	int x;
	int y;
} soldierAlternativePoints[] = {{-1, -6},
	{0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1},
	{0, -2}, {1, -2}, {2, -2}, {2, -1}, {2, 0}, {2, 1}, {2, 2}, {1, 2},
	{0, 2}, {-1, 2}, {-2, 2}, {-2, 1}, {-2, 0}, {-2, -1}, {-2, -2}, {-1, -2},
	{0, -3}, {1, -3}, {2, -3}, {3, -3}, {3, -2}, {3, -1}, {3, 0}, {3, 1},
	{3, 2}, {3, 3}, {2, 3}, {1, 3}, {0, 3}, {-1, 3}, {-2, 3}, {-3, 3},
	{-3, 2}, {-3, 1}, {-3, 0}, {-3, -1}, {-3, -2}, {-3, -3}, {-2, -3}, {-1, -3},
	{0, -4}, {1, -4}, {2, -4}, {3, -4}, {4, -4}, {4, -3}, {4, -2}, {4, -1},
	{4, 0}, {4, 1}, {4, 2}, {4, 3}, {4, 4}, {3, 4}, {2, 4}, {1, 4},
	{0, 4}, {-1, 4}, {-2, 4}, {-3, 4}, {-4, 4}, {-4, 3}, {-4, 2}, {-4, 1},
	{-4, 0}, {-4, -1}, {-4, -2}, {-4, -3}, {-4, -4}, {-3, -4}, {-2, -4}, {-1, -4},
	{0, -5}, {1, -5}, {2, -5}, {3, -5}, {4, -5}, {5, -5}, {5, -4}, {5, -3},
	{5, -2}, {5, -1}, {5, 0}, {5, 1}, {5, 2}, {5, 3}, {5, 4}, {5, 5},
	{4, 5}, {3, 5}, {2, 5}, {1, 5}, {0, 5}, {-1, 5}, {-2, 5}, {-3, 5},
	{-4, 5}, {-5, 5}, {-5, 4}, {-5, 3}, {-5, 2}, {-5, 1}, {-5, 0}, {-5, -1},
	{-5, -2}, {-5, -3}, {-5, -4}, {-5, -5}, {-4, -5}, {-3, -5}, {-2, -5}, {-1, -5},
	{0, -6}, {1, -6}, {2, -6}, {3, -6}, {4, -6}, {5, -6}, {6, -6}, {6, -5},
	{6, -4}, {6, -3}, {6, -2}, {6, -1}, {6, 0}, {6, 1}, {6, 2}, {6, 3},
	{6, 4}, {6, 5}, {6, 6}, {5, 6}, {4, 6}, {3, 6}, {2, 6}, {1, 6},
	{0, 6}, {-1, 6}, {-2, 6}, {-3, 6}, {-4, 6}, {-5, 6}, {-6, 6}, {-6, 5},
	{-6, 4}, {-6, 3}, {-6, 2}, {-6, 1}, {-6, 0}, {-6, -1}, {-6, -2}, {-6, -3},
	{-6, -4}, {-6, -5}, {-6, -6}, {-5, -6}, {-4, -6}, {-3, -6}, {-2, -6}, {-1, -6},
};

void FigureAction_militaryStandard(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[w->formationId];

	w->terrainUsage = FigureTerrainUsage_Any;
	FigureActionIncreaseGraphicOffset(w, 16);
	Figure_removeFromTileList(walkerId);
	if (f->isAtFort) {
		w->x = f->x;
		w->y = f->y;
	} else {
		w->x = f->xStandard;
		w->y = f->yStandard;
	}
	w->gridOffset = GridOffset(w->x, w->y);
	w->crossCountryX = 15 * w->x + 7;
	w->crossCountryY = 15 * w->y + 7;
	Figure_addToTileList(walkerId);

	w->graphicId = GraphicId(ID_Graphic_FortStandardPole) + 20 - f->morale / 5;
	if (f->figureType == Figure_FortLegionary) {
		if (f->isHalted) {
			w->cartGraphicId = GraphicId(ID_Graphic_FortFlags) + 8;
		} else {
			w->cartGraphicId = GraphicId(ID_Graphic_FortFlags) + w->graphicOffset / 2;
		}
	} else if (f->figureType == Figure_FortMounted) {
		if (f->isHalted) {
			w->cartGraphicId = GraphicId(ID_Graphic_FortFlags) + 26;
		} else {
			w->cartGraphicId = GraphicId(ID_Graphic_FortFlags) + 18 + w->graphicOffset / 2;
		}
	} else {
		if (f->isHalted) {
			w->cartGraphicId = GraphicId(ID_Graphic_FortFlags) + 17;
		} else {
			w->cartGraphicId = GraphicId(ID_Graphic_FortFlags) + 9 + w->graphicOffset / 2;
		}
	}
}

static void javelinLaunchMissile(int walkerId, struct Data_Walker *w)
{
	int xTile, yTile;
	w->waitTicksMissile++;
	if (w->waitTicksMissile > Constant_FigureProperties[w->type].missileFrequency) {
		w->waitTicksMissile = 0;
		if (FigureAction_CombatSoldier_getMissileTarget(walkerId, 10, &xTile, &yTile)) {
			w->attackGraphicOffset = 1;
			w->direction = Routing_getDirectionForMissileShooter(w->x, w->y, xTile, yTile);
		} else {
			w->attackGraphicOffset = 0;
		}
	}
	if (w->attackGraphicOffset) {
		if (w->attackGraphicOffset == 1) {
			Figure_createMissile(walkerId, w->x, w->y, xTile, yTile, Figure_Javelin);
			Data_Formations[w->formationId].missileFired = 6;
		}
		w->attackGraphicOffset++;
		if (w->attackGraphicOffset > 100) {
			w->attackGraphicOffset = 0;
		}
	}
}

static void legionaryAttackAdjacentEnemy(int walkerId, struct Data_Walker *w)
{
	int gridOffset = w->gridOffset;
	for (int i = 0; i < 8 && w->actionState != FigureActionState_150_Attack; i++) {
		FigureAction_Combat_attackWalker(walkerId,
			Data_Grid_figureIds[gridOffset + Constant_DirectionGridOffsets[i]]);
	}
}

static int soldierFindMopUpTarget(int walkerId, struct Data_Walker *w)
{
	int targetId = w->targetWalkerId;
	if (FigureIsDead(targetId)) {
		w->targetWalkerId = 0;
		targetId = 0;
	}
	if (targetId <= 0) {
		targetId = FigureAction_CombatSoldier_getTarget(w->x, w->y, 20);
		if (targetId) {
			struct Data_Walker *t = &Data_Walkers[targetId];
			w->destinationX = t->x;
			w->destinationY = t->y;
			w->targetWalkerId = targetId;
			t->targetedByWalkerId = walkerId;
			w->targetWalkerCreatedSequence = t->createdSequence;
		} else {
			w->actionState = FigureActionState_84_SoldierAtStandard;
			w->graphicOffset = 0;
		}
		FigureRoute_remove(walkerId);
	}
	return targetId;
}

static void updateSoldierGraphicJavelin(struct Data_Walker *w, int dir)
{
	int graphicId = GraphicId(ID_Graphic_Figure_FortJavelin);
	if (w->actionState == FigureActionState_150_Attack) {
		if (w->attackGraphicOffset < 12) {
			w->graphicId = graphicId + 96 + dir;
		} else {
			w->graphicId = graphicId + 96 + dir + 8 * ((w->attackGraphicOffset - 12) / 2);
		}
	} else if (w->actionState == FigureActionState_149_Corpse) {
		w->graphicId = graphicId + 144 + WalkerActionCorpseGraphicOffset(w);
	} else if (w->actionState == FigureActionState_84_SoldierAtStandard) {
		w->graphicId = graphicId + 96 + dir +
			8 * WalkerActionMissileLauncherGraphicOffset(w);
	} else {
		w->graphicId = graphicId + dir + 8 * w->graphicOffset;
	}
}

static void updateSoldierGraphicMounted(struct Data_Walker *w, int dir)
{
	int graphicId = GraphicId(ID_Graphic_Figure_FortMounted);
	if (w->actionState == FigureActionState_150_Attack) {
		if (w->attackGraphicOffset < 12) {
			w->graphicId = graphicId + 96 + dir;
		} else {
			w->graphicId = graphicId + 96 + dir + 8 * ((w->attackGraphicOffset - 12) / 2);
		}
	} else if (w->actionState == FigureActionState_149_Corpse) {
		w->graphicId = graphicId + 144 + WalkerActionCorpseGraphicOffset(w);
	} else {
		w->graphicId = graphicId + dir + 8 * w->graphicOffset;
	}
}

static void updateSoldierGraphicLegionary(struct Data_Walker *w, struct Data_Formation *f, int dir)
{
	int graphicId = GraphicId(ID_Graphic_Figure_FortLegionary);
	if (w->actionState == FigureActionState_150_Attack) {
		if (w->attackGraphicOffset < 12) {
			w->graphicId = graphicId + 96 + dir;
		} else {
			w->graphicId = graphicId + 96 + dir + 8 * ((w->attackGraphicOffset - 12) / 2);
		}
	} else if (w->actionState == FigureActionState_149_Corpse) {
		w->graphicId = graphicId + 152 + WalkerActionCorpseGraphicOffset(w);
	} else if (w->actionState == FigureActionState_84_SoldierAtStandard) {
		if (f->isHalted && f->layout == FormationLayout_Tortoise && f->missileAttackTimeout) {
			w->graphicId = graphicId + dir + 144;
		} else {
			w->graphicId = graphicId + dir;
		}
	} else {
		w->graphicId = graphicId + dir + 8 * w->graphicOffset;
	}
}

static void updateSoldierGraphic(int walkerId, struct Data_Walker *w, struct Data_Formation *f)
{
	int dir;
	if (w->actionState == FigureActionState_150_Attack) {
		dir = w->attackDirection;
	} else if (f->missileFired) {
		dir = w->direction;
	} else if (w->actionState == FigureActionState_84_SoldierAtStandard) {
		dir = f->direction;
	} else if (w->direction < 8) {
		dir = w->direction;
	} else {
		dir = w->previousTileDirection;
	}
	WalkerActionNormalizeDirection(dir);
	if (w->type == Figure_FortJavelin) {
		updateSoldierGraphicJavelin(w, dir);
	} else if (w->type == Figure_FortMounted) {
		updateSoldierGraphicMounted(w, dir);
	} else if (w->type == Figure_FortLegionary) {
		updateSoldierGraphicLegionary(w, f, dir);
	}
}

void FigureAction_soldier(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[w->formationId];
	Data_CityInfo.numSoldiersInCity++;
	w->terrainUsage = FigureTerrainUsage_Any;
	FigureActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	if (f->inUse != 1) {
		w->actionState = FigureActionState_149_Corpse;
	}
	int speedFactor;
	if (w->type == Figure_FortMounted) {
		speedFactor = 3;
	} else if (w->type == Figure_FortJavelin) {
		speedFactor = 2;
	} else {
		speedFactor = 1;
	}
	int layout = f->layout;
	if (w->formationAtRest || w->actionState == FigureActionState_81_SoldierGoingToFort) {
		layout = FormationLayout_AtRest;
	}
	w->formationPositionX = f->x + WalkerActionFormationLayoutPositionX(layout, w->indexInFormation);
	w->formationPositionY = f->y + WalkerActionFormationLayoutPositionY(layout, w->indexInFormation);
	
	switch (w->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_80_SoldierAtRest:
			Figure_updatePositionInTileList(walkerId);
			w->waitTicks = 0;
			w->formationAtRest = 1;
			w->graphicOffset = 0;
			if (w->x != w->formationPositionX || w->y != w->formationPositionY) {
				w->actionState = FigureActionState_81_SoldierGoingToFort;
			}
			break;
		case FigureActionState_81_SoldierGoingToFort:
		case FigureActionState_148_Fleeing:
			w->waitTicks = 0;
			w->formationAtRest = 1;
			w->destinationX = w->formationPositionX;
			w->destinationY = w->formationPositionY;
			w->destinationGridOffsetSoldier = GridOffset(w->destinationX, w->destinationY);
			FigureMovement_walkTicks(walkerId, speedFactor);
			if (w->direction == DirFigure_8_AtDestination) {
				w->actionState = FigureActionState_80_SoldierAtRest;
			} else if (w->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
			} else if (w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			}
			break;
		case FigureActionState_82_SoldierReturningToBarracks:
			w->formationAtRest = 1;
			w->destinationX = w->sourceX;
			w->destinationY = w->sourceY;
			FigureMovement_walkTicks(walkerId, speedFactor);
			if (w->direction == DirFigure_8_AtDestination || w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			} else if (w->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
			}
			break;
		case FigureActionState_83_SoldierGoingToStandard:
			w->formationAtRest = 0;
			w->destinationX = f->xStandard + WalkerActionFormationLayoutPositionX(f->layout, w->indexInFormation);
			w->destinationY = f->yStandard + WalkerActionFormationLayoutPositionY(f->layout, w->indexInFormation);
			if (w->alternativeLocationIndex) {
				w->destinationX += soldierAlternativePoints[w->alternativeLocationIndex].x;
				w->destinationY += soldierAlternativePoints[w->alternativeLocationIndex].y;
			}
			w->destinationGridOffsetSoldier = GridOffset(w->destinationX, w->destinationY);
			FigureMovement_walkTicks(walkerId, speedFactor);
			if (w->direction == DirFigure_8_AtDestination) {
				w->actionState = FigureActionState_84_SoldierAtStandard;
				w->graphicOffset = 0;
			} else if (w->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
			} else if (w->direction == DirFigure_10_Lost) {
				w->alternativeLocationIndex++;
				if (w->alternativeLocationIndex > 168) {
					w->state = FigureState_Dead;
				}
				w->graphicOffset = 0;
			}
			break;
		case FigureActionState_84_SoldierAtStandard:
			w->formationAtRest = 0;
			w->graphicOffset = 0;
			Figure_updatePositionInTileList(walkerId);
			w->destinationX = f->xStandard + WalkerActionFormationLayoutPositionX(f->layout, w->indexInFormation);
			w->destinationY = f->yStandard + WalkerActionFormationLayoutPositionY(f->layout, w->indexInFormation);
			if (w->alternativeLocationIndex) {
				w->destinationX += soldierAlternativePoints[w->alternativeLocationIndex].x;
				w->destinationY += soldierAlternativePoints[w->alternativeLocationIndex].y;
			}
			if (w->x != w->destinationX || w->y != w->destinationY) {
				if (f->missileFired <= 0 && f->recentFight <= 0 && f->missileAttackTimeout <= 0) {
					w->actionState = FigureActionState_83_SoldierGoingToStandard;
					w->alternativeLocationIndex = 0;
				}
			}
			if (w->actionState != FigureActionState_83_SoldierGoingToStandard) {
				if (w->type == Figure_FortJavelin) {
					javelinLaunchMissile(walkerId, w);
				} else if (w->type == Figure_FortLegionary) {
					legionaryAttackAdjacentEnemy(walkerId, w);
				}
			}
			break;
		case FigureActionState_85_SoldierGoingToMilitaryAcademy:
			f->hasMilitaryTraining = 1;
			w->formationAtRest = 1;
			FigureMovement_walkTicks(walkerId, speedFactor);
			if (w->direction == DirFigure_8_AtDestination) {
				w->actionState = FigureActionState_81_SoldierGoingToFort;
			} else if (w->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
			} else if (w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			}
			break;
		case FigureActionState_86_SoldierMoppingUp:
			w->formationAtRest = 0;
			if (soldierFindMopUpTarget(walkerId, w)) {
				FigureMovement_walkTicks(walkerId, speedFactor);
				if (w->direction == DirFigure_8_AtDestination) {
					w->destinationX = Data_Walkers[w->targetWalkerId].x;
					w->destinationY = Data_Walkers[w->targetWalkerId].y;
					FigureRoute_remove(walkerId);
				} else if (w->direction == DirFigure_9_Reroute || w->direction == DirFigure_10_Lost) {
					w->actionState = FigureActionState_84_SoldierAtStandard;
					w->targetWalkerId = 0;
					w->graphicOffset = 0;
				}
			}
			break;
		case FigureActionState_87_SoldierGoingToDistantBattle:
			w->formationAtRest = 0;
			w->destinationX = Data_CityInfo.exitPointX;
			w->destinationY = Data_CityInfo.exitPointY;
			FigureMovement_walkTicks(walkerId, speedFactor);
			if (w->direction == DirFigure_8_AtDestination) {
				w->actionState = FigureActionState_89_SoldierAtDistantBattle;
				FigureRoute_remove(walkerId);
			} else if (w->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
			} else if (w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			}
			break;
		case FigureActionState_88_SoldierReturningFromDistantBattle:
			w->isGhost = 0;
			w->waitTicks = 0;
			w->formationAtRest = 1;
			w->destinationX = w->formationPositionX;
			w->destinationY = w->formationPositionY;
			w->destinationGridOffsetSoldier = GridOffset(w->destinationX, w->destinationY);
			FigureMovement_walkTicks(walkerId, speedFactor);
			if (w->direction == DirFigure_8_AtDestination) {
				w->actionState = FigureActionState_80_SoldierAtRest;
			} else if (w->direction == DirFigure_9_Reroute) {
				FigureRoute_remove(walkerId);
			} else if (w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			}
			break;
		case FigureActionState_89_SoldierAtDistantBattle:
			w->isGhost = 1;
			w->formationAtRest = 1;
			break;
	}
	
	updateSoldierGraphic(walkerId, w, f);
}
