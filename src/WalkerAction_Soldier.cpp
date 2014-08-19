#include "WalkerAction_private.h"

#include "Routing.h"
#include "Walker.h"

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

static void javelinLaunchMissile(int walkerId, struct Data_Walker *w)
{
	int xTile, yTile;
	w->waitTicksMissile++;
	if (w->waitTicksMissile > Constant_WalkerProperties[w->type].missileFrequency) {
		w->waitTicksMissile = 0;
		if (WalkerAction_CombatSoldier_getMissileTarget(walkerId, 10, &xTile, &yTile)) {
			w->attackGraphicOffset = 1;
			w->direction = Routing_getDirection(w->x, w->y, xTile, yTile);
		} else {
			w->attackGraphicOffset = 0;
		}
	}
	if (w->attackGraphicOffset) {
		if (w->attackGraphicOffset == 1) {
			Walker_createMissile(walkerId, w->x, w->y, xTile, yTile, Walker_Javelin);
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
	for (int i = 0; i < 8 && w->actionState != WalkerActionState_150_Attack; i++) {
		WalkerAction_CombatSoldier_attackWalker(walkerId,
			Data_Grid_walkerIds[gridOffset + Constant_DirectionGridOffsets[i]]);
	}
}

static int soldierFindMopUpTarget(int walkerId, struct Data_Walker *w)
{
	int targetId = w->targetWalkerId;
	struct Data_Walker *t = &Data_Walkers[w->targetWalkerId];
	if (w->state != WalkerState_Alive || w->actionState == WalkerActionState_149_Corpse) {
		targetId = 0;
	}
	if (targetId <= 0) {
		targetId = WalkerAction_CombatSoldier_getTarget(w->x, w->y, 20);
		if (targetId) {
			t = &Data_Walkers[w->targetWalkerId];
			w->destinationX = t->x;
			w->destinationY = t->y;
			w->targetWalkerId = targetId;
			t->targetedByWalkerId = walkerId;
			w->targetWalkerCreatedSequence = t->createdSequence;
		} else {
			w->actionState = WalkerActionState_84_SoldierAtStandard;
			w->graphicOffset = 0;
		}
	}
	return targetId;
}

static void updateSoldierGraphicJavelin(struct Data_Walker *w, int dir)
{
	int graphicId = GraphicId(ID_Graphic_Walker_FortJavelin);
	if (w->actionState == WalkerActionState_150_Attack) {
		if (w->attackGraphicOffset < 12) {
			w->graphicId = graphicId + 96 + dir;
		} else {
			w->graphicId = graphicId + 96 + dir +
				8 * ((w->attackGraphicOffset - 12) / 2);
		}
	} else if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = graphicId + 144 +
			WalkerActionCorpseGraphicOffset(w);
	} else if (w->actionState == WalkerActionState_84_SoldierAtStandard) {
		w->graphicId = graphicId + 96 + dir +
			8 * WalkerActionMissileLauncherGraphicOffset(w);
	} else {
		w->graphicId = graphicId + dir + 8 * w->graphicOffset;
	}
}

static void updateSoldierGraphicMounted(struct Data_Walker *w, int dir)
{
	int graphicId = GraphicId(ID_Graphic_Walker_FortMounted);
	if (w->actionState == WalkerActionState_150_Attack) {
		if (w->attackGraphicOffset < 12) {
			w->graphicId = graphicId + 96 + dir;
		} else {
			w->graphicId = graphicId + 96 + dir +
				8 * ((w->attackGraphicOffset - 12) / 2);
		}
	} else if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = graphicId + 144 +
			WalkerActionCorpseGraphicOffset(w);
	} else {
		w->graphicId = graphicId + dir + 8 * w->graphicOffset;
	}
}

static void updateSoldierGraphicLegionary(struct Data_Walker *w, struct Data_Formation *f, int dir)
{
	int graphicId = GraphicId(ID_Graphic_Walker_FortJavelin);
	if (w->actionState == WalkerActionState_150_Attack) {
		if (w->attackGraphicOffset < 12) {
			w->graphicId = graphicId + 96 + dir;
		} else {
			w->graphicId = graphicId + 96 + dir +
				8 * ((w->attackGraphicOffset - 12) / 2);
		}
	} else if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = graphicId + 144 +
			WalkerActionCorpseGraphicOffset(w);
	} else if (w->actionState == WalkerActionState_84_SoldierAtStandard) {
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
	if (w->actionState == WalkerActionState_150_Attack) {
		dir = w->attackDirection;
	} else if (f->missileFired) {
		dir = w->direction;
	} else if (w->actionState == WalkerActionState_84_SoldierAtStandard) {
		dir = f->direction;
	} else if (w->direction < 8) {
		dir = w->direction;
	} else {
		dir = w->previousTileDirection;
	}
	dir = (8 + dir - Data_Settings_Map.orientation) % 8;
	// TODO
	if (w->type == Walker_FortJavelin) {
		updateSoldierGraphicJavelin(w, dir);
	} else if (w->type == Walker_FortMounted) {
		updateSoldierGraphicMounted(w, dir);
	} else if (w->type == Walker_FortLegionary) {
		updateSoldierGraphicLegionary(w, f, dir);
	}
}

void WalkerAction_soldier(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[w->formationId];
	Data_CityInfo.numSoldiersInCity++;
	w->terrainUsage = 0;
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	if (f->inUse != 1) {
		w->actionState = WalkerActionState_149_Corpse;
	}
	int speedFactor;
	if (w->type == Walker_FortMounted) {
		speedFactor = 3;
	} else if (w->type == Walker_FortJavelin) {
		speedFactor = 2;
	} else {
		speedFactor = 1;
	}
	int layout = f->layout;
	if (w->formationAtRest || w->actionState == WalkerActionState_81_SoldierGoingToFort) {
		layout = FormationLayout_AtRest;
	}
	w->formationPositionX = f->x + WalkerActionFormationLayoutPositionX(layout, w->indexInFormation);
	w->formationPositionY = f->y + WalkerActionFormationLayoutPositionY(layout, w->indexInFormation);
	
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_80_SoldierAtRest:
			Walker_updatePositionInTileList(walkerId);
			w->waitTicks = 0;
			w->formationAtRest = 1;
			w->graphicOffset = 0;
			if (w->x != w->formationPositionX || w->y != w->formationPositionY) {
				w->actionState = WalkerActionState_81_SoldierGoingToFort;
			}
			break;
		case WalkerActionState_81_SoldierGoingToFort:
		case WalkerActionState_148_Fleeing:
			w->waitTicks = 0;
			w->formationAtRest = 1;
			w->destinationX = w->formationPositionX;
			w->destinationY = w->formationPositionY;
			w->destinationGridOffsetSoldier = GridOffset(w->destinationX, w->destinationY);
			WalkerMovement_walkTicks(walkerId, speedFactor);
			if (w->direction == 8) {
				w->actionState = WalkerActionState_80_SoldierAtRest;
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_82_SoldierReturningToBarracks:
			w->formationAtRest = 1;
			w->destinationX = w->sourceX;
			w->destinationY = w->sourceY;
			WalkerMovement_walkTicks(walkerId, speedFactor);
			if (w->direction == 8 || w->direction == 10) {
				w->state = WalkerState_Dead;
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			}
			break;
		case WalkerActionState_83_SoldierGoingToStandard:
			w->formationAtRest = 0;
			w->destinationX = f->xStandard + WalkerActionFormationLayoutPositionX(f->layout, w->indexInFormation);
			w->destinationY = f->yStandard + WalkerActionFormationLayoutPositionY(f->layout, w->indexInFormation);
			if (w->alternativeLocationIndex) {
				w->destinationX += soldierAlternativePoints[w->alternativeLocationIndex].x;
				w->destinationY += soldierAlternativePoints[w->alternativeLocationIndex].y;
			}
			w->destinationGridOffsetSoldier = GridOffset(w->destinationX, w->destinationY);
			WalkerMovement_walkTicks(walkerId, speedFactor);
			if (w->direction == 8) {
				w->actionState = WalkerActionState_84_SoldierAtStandard;
				w->graphicOffset = 0;
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == 10) {
				w->alternativeLocationIndex++;
				if (w->alternativeLocationIndex > 168) {
					w->state = WalkerState_Dead;
				}
				w->graphicOffset = 0;
			}
			break;
		case WalkerActionState_84_SoldierAtStandard:
			w->formationAtRest = 0;
			w->graphicOffset = 0;
			Walker_updatePositionInTileList(walkerId);
			w->destinationX = f->xStandard + WalkerActionFormationLayoutPositionX(f->layout, w->indexInFormation);
			w->destinationY = f->yStandard + WalkerActionFormationLayoutPositionY(f->layout, w->indexInFormation);
			if (w->alternativeLocationIndex) {
				w->destinationX += soldierAlternativePoints[w->alternativeLocationIndex].x;
				w->destinationY += soldierAlternativePoints[w->alternativeLocationIndex].y;
			}
			if (w->x != w->destinationX || w->y != w->destinationY) {
				if (f->missileFired <= 0 && f->recentFight <= 0 && f->missileAttackTimeout) {
					w->actionState = WalkerActionState_83_SoldierGoingToStandard;
					w->alternativeLocationIndex = 0;
				}
			}
			if (w->actionState != WalkerActionState_83_SoldierGoingToStandard) {
				if (w->type == Walker_FortJavelin) {
					javelinLaunchMissile(walkerId, w);
				} else if (w->type == Walker_FortLegionary) {
					legionaryAttackAdjacentEnemy(walkerId, w);
				}
			}
			break;
		case WalkerActionState_85_SoldierGoingToMilitaryAcademy:
			f->hasMilitaryTraining = 1;
			w->formationAtRest = 1;
			WalkerMovement_walkTicks(walkerId, speedFactor);
			if (w->direction == 8) {
				w->actionState = WalkerActionState_81_SoldierGoingToFort;
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_86_SoldierMoppingUp:
			w->formationAtRest = 0;
			if (soldierFindMopUpTarget(walkerId, w)) {
				WalkerMovement_walkTicks(walkerId, speedFactor);
				if (w->direction == 8) {
					w->destinationX = Data_Walkers[w->targetWalkerId].x;
					w->destinationY = Data_Walkers[w->targetWalkerId].y;
					WalkerRoute_remove(walkerId);
				} else if (w->direction == 9 || w->direction == 10) {
					w->actionState = WalkerActionState_84_SoldierAtStandard;
					w->targetWalkerId = 0;
					w->graphicOffset = 0;
				}
			}
			break;
		case WalkerActionState_87_SoldierGoingToDistantBattle:
			w->formationAtRest = 0;
			w->destinationX = Data_CityInfo.exitPointX;
			w->destinationY = Data_CityInfo.exitPointY;
			WalkerMovement_walkTicks(walkerId, speedFactor);
			if (w->direction == 8) {
				w->actionState = WalkerActionState_89_SoldierAtDistantBattle;
				WalkerRoute_remove(walkerId);
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_88_SoldierReturningFromDistantBattle:
			w->isGhost = 0;
			w->waitTicks = 0;
			w->formationAtRest = 1;
			w->destinationX = w->formationPositionX;
			w->destinationY = w->formationPositionY;
			w->destinationGridOffsetSoldier = GridOffset(w->destinationX, w->destinationY);
			WalkerMovement_walkTicks(walkerId, speedFactor);
			if (w->direction == 8) {
				w->actionState = WalkerActionState_80_SoldierAtRest;
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_89_SoldierAtDistantBattle:
			w->isGhost = 1;
			w->formationAtRest = 1;
			break;
	}
	
	updateSoldierGraphic(walkerId, w, f);
}
