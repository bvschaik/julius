#include "FigureAction_private.h"

#include "Calc.h"
#include "FigureMovement.h"
#include "Routing.h"

int FigureAction_CombatSoldier_getTarget(int x, int y, int maxDistance)
{
	int minWalkerId = 0;
	int minDistance = 10000;
	for (int i = 1; i < MAX_FIGURES; i++) {
		if (FigureIsDead(i)) {
			continue;
		}
		struct Data_Walker *f = &Data_Walkers[i];
		if (WalkerIsEnemy(f->type) || f->type == Figure_Rioter ||
			(f->type == Figure_IndigenousNative && f->actionState == FigureActionState_159_NativeAttacking)) {
			int distance = Calc_distanceMaximum(x, y, f->x, f->y);
			if (distance <= maxDistance) {
				if (f->targetedByWalkerId) {
					distance *= 2; // penalty
				}
				if (distance < minDistance) {
					minDistance = distance;
					minWalkerId = i;
				}
			}
		}
	}
	if (minWalkerId) {
		return minWalkerId;
	}
	for (int i = 1; i < MAX_FIGURES; i++) {
		if (FigureIsDead(i)) {
			continue;
		}
		struct Data_Walker *f = &Data_Walkers[i];
		if (WalkerIsEnemy(f->type) || f->type == Figure_Rioter ||
			(f->type == Figure_IndigenousNative && f->actionState == FigureActionState_159_NativeAttacking)) {
			return i;
		}
	}
	return 0;
}

int FigureAction_CombatSoldier_getMissileTarget(int soldierId, int maxDistance, int *xTile, int *yTile)
{
	int x = Data_Walkers[soldierId].x;
	int y = Data_Walkers[soldierId].y;
	
	int minWalkerId = 0;
	int minDistance = maxDistance;
	for (int i = 1; i < MAX_FIGURES; i++) {
		if (FigureIsDead(i)) {
			continue;
		}
		struct Data_Walker *f = &Data_Walkers[i];
		if (WalkerIsEnemy(f->type) || WalkerIsHerd(f->type) ||
			(f->type == Figure_IndigenousNative && f->actionState == FigureActionState_159_NativeAttacking)) {
			int distance = Calc_distanceMaximum(x, y, f->x, f->y);
			if (distance < minDistance && FigureMovement_canLaunchCrossCountryMissile(x, y, f->x, f->y)) {
				minDistance = distance;
				minWalkerId = i;
			}
		}
	}
	if (minWalkerId) {
		*xTile = Data_Walkers[minWalkerId].x;
		*yTile = Data_Walkers[minWalkerId].y;
		return minWalkerId;
	}
	return 0;
}

int FigureAction_CombatWolf_getTarget(int x, int y, int maxDistance)
{
	int minWalkerId = 0;
	int minDistance = 10000;
	for (int i = 1; i < MAX_FIGURES; i++) {
		struct Data_Walker *f = &Data_Walkers[i];
		if (FigureIsDead(i) || !f->type) {
			continue;
		}
		switch (f->type) {
			case Figure_Explosion:
			case Figure_FortStandard:
			case Figure_TradeShip:
			case Figure_FishingBoat:
			case Figure_MapFlag:
			case Figure_Flotsam:
				
			case Figure_Shipwreck:
			case Figure_IndigenousNative:
			case Figure_TowerSentry:
			case Figure_NativeTrader:
			case Figure_Arrow:
			case Figure_Javelin:
			case Figure_Bolt:
			case Figure_Ballista:
			case Figure_Creature:
				continue;
		}
		if (WalkerIsEnemy(f->type) || WalkerIsHerd(f->type)) {
			continue;
		}
		if (WalkerIsLegion(f->type) && f->actionState == FigureActionState_80_SoldierAtRest) {
			continue;
		}
		int distance = Calc_distanceMaximum(x, y, f->x, f->y);
		if (f->targetedByWalkerId) {
			distance *= 2;
		}
		if (distance < minDistance) {
			minDistance = distance;
			minWalkerId = i;
		}
	}
	if (minDistance <= maxDistance && minWalkerId) {
		return minWalkerId;
	}
	return 0;
}

int FigureAction_CombatEnemy_getTarget(int x, int y)
{
	int minWalkerId = 0;
	int minDistance = 10000;
	for (int i = 1; i < MAX_FIGURES; i++) {
		if (FigureIsDead(i)) {
			continue;
		}
		struct Data_Walker *f = &Data_Walkers[i];
		if (!f->targetedByWalkerId && WalkerIsLegion(f->type)) {
			int distance = Calc_distanceMaximum(x, y, f->x, f->y);
			if (distance < minDistance) {
				minDistance = distance;
				minWalkerId = i;
			}
		}
	}
	if (minWalkerId) {
		return minWalkerId;
	}
	// no 'free' soldier found, take first one
	for (int i = 1; i < MAX_FIGURES; i++) {
		if (FigureIsDead(i)) {
			continue;
		}
		if (WalkerIsLegion(Data_Walkers[i].type)) {
			return i;
		}
	}
	return 0;
}

int FigureAction_CombatEnemy_getMissileTarget(int enemyId, int maxDistance, int attackCitizens, int *xTile, int *yTile)
{
	int x = Data_Walkers[enemyId].x;
	int y = Data_Walkers[enemyId].y;
	
	int minWalkerId = 0;
	int minDistance = maxDistance;
	for (int i = 1; i < MAX_FIGURES; i++) {
		struct Data_Walker *f = &Data_Walkers[i];
		if (FigureIsDead(i) || !f->type) {
			continue;
		}
		switch (f->type) {
			case Figure_Explosion:
			case Figure_FortStandard:
			case Figure_MapFlag:
			case Figure_Flotsam:
			case Figure_IndigenousNative:
			case Figure_NativeTrader:
			case Figure_Arrow:
			case Figure_Javelin:
			case Figure_Bolt:
			case Figure_Ballista:
			case Figure_Creature:
			case Figure_FishGulls:
			case Figure_Shipwreck:
			case Figure_Sheep:
			case Figure_Wolf:
			case Figure_Zebra:
			case Figure_Spear:
				continue;
		}
		int distance;
		if (WalkerIsLegion(f->type)) {
			distance = Calc_distanceMaximum(x, y, f->x, f->y);
		} else if (attackCitizens && f->isFriendly) {
			distance = Calc_distanceMaximum(x, y, f->x, f->y) + 5;
		} else {
			continue;
		}
		if (distance < minDistance && FigureMovement_canLaunchCrossCountryMissile(x, y, f->x, f->y)) {
			minDistance = distance;
			minWalkerId = i;
		}
	}
	if (minWalkerId) {
		*xTile = Data_Walkers[minWalkerId].x;
		*yTile = Data_Walkers[minWalkerId].y;
		return minWalkerId;
	}
	return 0;
}


void FigureAction_Combat_attackWalker(int walkerId, int opponentId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	int figureCategory = Constant_FigureProperties[f->type].category;
	if (figureCategory <= FigureCategory_Inactive || figureCategory >= FigureCategory_Criminal ||
			f->actionState == FigureActionState_150_Attack) {
		return;
	}
	int guard = 0;
	while (1) {
		if (++guard >= MAX_FIGURES || opponentId <= 0) {
			break;
		}
		if (opponentId == walkerId) {
			opponentId = Data_Walkers[opponentId].nextWalkerIdOnSameTile;
			continue;
		}
		struct Data_Walker *opponent = &Data_Walkers[opponentId];
		int opponentCategory = Constant_FigureProperties[opponent->type].category;
		int attack = 0;
		if (opponent->state != FigureState_Alive) {
			attack = 0;
		} else if (opponent->actionState == FigureActionState_149_Corpse) {
			attack = 0;
		} else if (figureCategory == FigureCategory_Armed && opponentCategory == FigureCategory_Native) {
			if (opponent->actionState == FigureActionState_159_NativeAttacking) {
				attack = 1;
			}
		} else if (figureCategory == FigureCategory_Armed && opponentCategory == FigureCategory_Criminal) {
			attack = 1;
		} else if (figureCategory == FigureCategory_Armed && opponentCategory == FigureCategory_Hostile) {
			attack = 1;
		} else if (figureCategory == FigureCategory_Hostile && opponentCategory == FigureCategory_Citizen) {
			attack = 1;
		} else if (figureCategory == FigureCategory_Hostile && opponentCategory == FigureCategory_Armed) {
			attack = 1;
		} else if (figureCategory == FigureCategory_Hostile && opponentCategory == FigureCategory_Criminal) {
			attack = 1;
		} else if (figureCategory == FigureCategory_Armed && opponentCategory == FigureCategory_Animal) {
			attack = 1;
		} else if (figureCategory == FigureCategory_Hostile && opponentCategory == FigureCategory_Animal) {
			attack = 1;
		}
		if (attack && opponent->actionState == FigureActionState_150_Attack && opponent->numAttackers >= 2) {
			attack = 0;
		}
		if (attack) {
			f->actionStateBeforeAttack = f->actionState;
			f->actionState = FigureActionState_150_Attack;
			f->opponentId = opponentId;
			f->attackerId1 = opponentId;
			f->numAttackers = 1;
			f->attackGraphicOffset = 12;
			if (opponent->x != opponent->destinationX || opponent->y != opponent->destinationY) {
				f->attackDirection = Routing_getGeneralDirection(f->previousTileX, f->previousTileY,
					opponent->previousTileX, opponent->previousTileY);
			} else {
				f->attackDirection = Routing_getGeneralDirection(f->previousTileX, f->previousTileY,
					opponent->x, opponent->y);
			}
			if (f->attackDirection >= 8) {
				f->attackDirection = 0;
			}
			if (opponent->actionState != FigureActionState_150_Attack) {
				opponent->actionStateBeforeAttack = opponent->actionState;
				opponent->actionState = FigureActionState_150_Attack;
				opponent->attackGraphicOffset = 0;
				opponent->attackDirection = (f->attackDirection + 4) % 8;
			}
			if (opponent->numAttackers == 0) {
				opponent->attackerId1 = walkerId;
				opponent->opponentId = walkerId;
				opponent->numAttackers = 1;
			} else if (opponent->numAttackers == 1) {
				opponent->attackerId2 = walkerId;
				opponent->numAttackers = 2;
			}
			return;
		}
		opponentId = opponent->nextWalkerIdOnSameTile;
	}
}
