#include "FigureAction.h"

#include "core/calc.h"
#include "FigureMovement.h"

#include "figure/properties.h"
#include "figure/type.h"
#include "map/figure.h"

int FigureAction_CombatSoldier_getTarget(int x, int y, int maxDistance)
{
	int minFigureId = 0;
	int minDistance = 10000;
	for (int i = 1; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
		if (figure_is_dead(f)) {
			continue;
		}
		if (FigureIsEnemy(f->type) || f->type == FIGURE_RIOTER ||
			(f->type == FIGURE_INDIGENOUS_NATIVE && f->actionState == FIGURE_ACTION_159_NATIVE_ATTACKING)) {
			int distance = calc_maximum_distance(x, y, f->x, f->y);
			if (distance <= maxDistance) {
				if (f->targetedByFigureId) {
					distance *= 2; // penalty
				}
				if (distance < minDistance) {
					minDistance = distance;
					minFigureId = i;
				}
			}
		}
	}
	if (minFigureId) {
		return minFigureId;
	}
	for (int i = 1; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
		if (figure_is_dead(f)) {
			continue;
		}
		if (FigureIsEnemy(f->type) || f->type == FIGURE_RIOTER ||
			(f->type == FIGURE_INDIGENOUS_NATIVE && f->actionState == FIGURE_ACTION_159_NATIVE_ATTACKING)) {
			return i;
		}
	}
	return 0;
}

int FigureAction_CombatSoldier_getMissileTarget(figure *shooter, int maxDistance, int *xTile, int *yTile)
{
	int x = shooter->x;
	int y = shooter->y;
	
	int minDistance = maxDistance;
    figure *minFigure = 0;
	for (int i = 1; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
		if (figure_is_dead(f)) {
			continue;
		}
		if (FigureIsEnemy(f->type) || FigureIsHerd(f->type) ||
			(f->type == FIGURE_INDIGENOUS_NATIVE && f->actionState == FIGURE_ACTION_159_NATIVE_ATTACKING)) {
			int distance = calc_maximum_distance(x, y, f->x, f->y);
			if (distance < minDistance && FigureMovement_canLaunchCrossCountryMissile(x, y, f->x, f->y)) {
				minDistance = distance;
				minFigure = f;
			}
		}
	}
	if (minFigure) {
		*xTile = minFigure->x;
		*yTile = minFigure->y;
		return minFigure->id;
	}
	return 0;
}

int FigureAction_CombatWolf_getTarget(int x, int y, int maxDistance)
{
	int minFigureId = 0;
	int minDistance = 10000;
	for (int i = 1; i < MAX_FIGURES; i++) {
		figure *f = figure_get(i);
		if (figure_is_dead(f) || !f->type) {
			continue;
		}
		switch (f->type) {
			case FIGURE_EXPLOSION:
			case FIGURE_FORT_STANDARD:
			case FIGURE_TRADE_SHIP:
			case FIGURE_FISHING_BOAT:
			case FIGURE_MAP_FLAG:
			case FIGURE_FLOTSAM:
			case FIGURE_SHIPWRECK:
			case FIGURE_INDIGENOUS_NATIVE:
			case FIGURE_TOWER_SENTRY:
			case FIGURE_NATIVE_TRADER:
			case FIGURE_ARROW:
			case FIGURE_JAVELIN:
			case FIGURE_BOLT:
			case FIGURE_BALLISTA:
			case FIGURE_CREATURE:
				continue;
		}
		if (FigureIsEnemy(f->type) || FigureIsHerd(f->type)) {
			continue;
		}
		if (FigureIsLegion(f->type) && f->actionState == FIGURE_ACTION_80_SOLDIER_AT_REST) {
			continue;
		}
		int distance = calc_maximum_distance(x, y, f->x, f->y);
		if (f->targetedByFigureId) {
			distance *= 2;
		}
		if (distance < minDistance) {
			minDistance = distance;
			minFigureId = i;
		}
	}
	if (minDistance <= maxDistance && minFigureId) {
		return minFigureId;
	}
	return 0;
}

int FigureAction_CombatEnemy_getTarget(int x, int y)
{
	int minFigureId = 0;
	int minDistance = 10000;
	for (int i = 1; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
		if (figure_is_dead(f)) {
			continue;
		}
		if (!f->targetedByFigureId && FigureIsLegion(f->type)) {
			int distance = calc_maximum_distance(x, y, f->x, f->y);
			if (distance < minDistance) {
				minDistance = distance;
				minFigureId = i;
			}
		}
	}
	if (minFigureId) {
		return minFigureId;
	}
	// no 'free' soldier found, take first one
	for (int i = 1; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
		if (figure_is_dead(f)) {
			continue;
		}
		if (FigureIsLegion(f->type)) {
			return i;
		}
	}
	return 0;
}

int FigureAction_CombatEnemy_getMissileTarget(figure *enemy, int maxDistance, int attackCitizens, int *xTile, int *yTile)
{
	int x = enemy->x;
	int y = enemy->y;
	
	figure *minFigure = 0;
	int minDistance = maxDistance;
	for (int i = 1; i < MAX_FIGURES; i++) {
		figure *f = figure_get(i);
		if (figure_is_dead(f) || !f->type) {
			continue;
		}
		switch (f->type) {
			case FIGURE_EXPLOSION:
			case FIGURE_FORT_STANDARD:
			case FIGURE_MAP_FLAG:
			case FIGURE_FLOTSAM:
			case FIGURE_INDIGENOUS_NATIVE:
			case FIGURE_NATIVE_TRADER:
			case FIGURE_ARROW:
			case FIGURE_JAVELIN:
			case FIGURE_BOLT:
			case FIGURE_BALLISTA:
			case FIGURE_CREATURE:
			case FIGURE_FISH_GULLS:
			case FIGURE_SHIPWRECK:
			case FIGURE_SHEEP:
			case FIGURE_WOLF:
			case FIGURE_ZEBRA:
			case FIGURE_SPEAR:
				continue;
		}
		int distance;
		if (FigureIsLegion(f->type)) {
			distance = calc_maximum_distance(x, y, f->x, f->y);
		} else if (attackCitizens && f->isFriendly) {
			distance = calc_maximum_distance(x, y, f->x, f->y) + 5;
		} else {
			continue;
		}
		if (distance < minDistance && FigureMovement_canLaunchCrossCountryMissile(x, y, f->x, f->y)) {
			minDistance = distance;
			minFigure = f;
		}
	}
	if (minFigure) {
		*xTile = minFigure->x;
		*yTile = minFigure->y;
		return minFigure->id;
	}
	return 0;
}


void FigureAction_Combat_attackFigureAt(figure *f, int grid_offset)
{
	int figureCategory = figure_properties_for_type(f->type)->category;
	if (figureCategory <= FIGURE_CATEGORY_INACTIVE || figureCategory >= FIGURE_CATEGORY_CRIMINAL ||
			f->actionState == FIGURE_ACTION_150_ATTACK) {
		return;
	}
	int guard = 0;
    int opponentId = map_figure_at(grid_offset);
	while (1) {
		if (++guard >= MAX_FIGURES || opponentId <= 0) {
			break;
		}
		figure *opponent = figure_get(opponentId);
		if (opponentId == f->id) {
			opponentId = opponent->nextFigureIdOnSameTile;
			continue;
		}
		
		int opponentCategory = figure_properties_for_type(opponent->type)->category;
		int attack = 0;
		if (opponent->state != FigureState_Alive) {
			attack = 0;
		} else if (opponent->actionState == FIGURE_ACTION_149_CORPSE) {
			attack = 0;
		} else if (figureCategory == FIGURE_CATEGORY_ARMED && opponentCategory == FIGURE_CATEGORY_NATIVE) {
			if (opponent->actionState == FIGURE_ACTION_159_NATIVE_ATTACKING) {
				attack = 1;
			}
		} else if (figureCategory == FIGURE_CATEGORY_ARMED && opponentCategory == FIGURE_CATEGORY_CRIMINAL) {
			attack = 1;
		} else if (figureCategory == FIGURE_CATEGORY_ARMED && opponentCategory == FIGURE_CATEGORY_HOSTILE) {
			attack = 1;
		} else if (figureCategory == FIGURE_CATEGORY_HOSTILE && opponentCategory == FIGURE_CATEGORY_CITIZEN) {
			attack = 1;
		} else if (figureCategory == FIGURE_CATEGORY_HOSTILE && opponentCategory == FIGURE_CATEGORY_ARMED) {
			attack = 1;
		} else if (figureCategory == FIGURE_CATEGORY_HOSTILE && opponentCategory == FIGURE_CATEGORY_CRIMINAL) {
			attack = 1;
		} else if (figureCategory == FIGURE_CATEGORY_ARMED && opponentCategory == FIGURE_CATEGORY_ANIMAL) {
			attack = 1;
		} else if (figureCategory == FIGURE_CATEGORY_HOSTILE && opponentCategory == FIGURE_CATEGORY_ANIMAL) {
			attack = 1;
		}
		if (attack && opponent->actionState == FIGURE_ACTION_150_ATTACK && opponent->numAttackers >= 2) {
			attack = 0;
		}
		if (attack) {
			f->actionStateBeforeAttack = f->actionState;
			f->actionState = FIGURE_ACTION_150_ATTACK;
			f->opponentId = opponentId;
			f->attackerId1 = opponentId;
			f->numAttackers = 1;
			f->attackGraphicOffset = 12;
			if (opponent->x != opponent->destinationX || opponent->y != opponent->destinationY) {
				f->attackDirection = calc_general_direction(f->previousTileX, f->previousTileY,
					opponent->previousTileX, opponent->previousTileY);
			} else {
				f->attackDirection = calc_general_direction(f->previousTileX, f->previousTileY,
					opponent->x, opponent->y);
			}
			if (f->attackDirection >= 8) {
				f->attackDirection = 0;
			}
			if (opponent->actionState != FIGURE_ACTION_150_ATTACK) {
				opponent->actionStateBeforeAttack = opponent->actionState;
				opponent->actionState = FIGURE_ACTION_150_ATTACK;
				opponent->attackGraphicOffset = 0;
				opponent->attackDirection = (f->attackDirection + 4) % 8;
			}
			if (opponent->numAttackers == 0) {
				opponent->attackerId1 = f->id;
				opponent->opponentId = f->id;
				opponent->numAttackers = 1;
			} else if (opponent->numAttackers == 1) {
				opponent->attackerId2 = f->id;
				opponent->numAttackers = 2;
			}
			return;
		}
		opponentId = opponent->nextFigureIdOnSameTile;
	}
}
