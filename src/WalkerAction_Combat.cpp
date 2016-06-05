#include "WalkerAction_private.h"

#include "Calc.h"
#include "Routing.h"
#include "WalkerMovement.h"

int WalkerAction_CombatSoldier_getTarget(int x, int y, int maxDistance)
{
	int minWalkerId = 0;
	int minDistance = 10000;
	for (int i = 1; i < MAX_FIGURES; i++) {
		if (FigureIsDead(i)) {
			continue;
		}
		struct Data_Walker *w = &Data_Walkers[i];
		if (WalkerIsEnemy(w->type) || w->type == Figure_Rioter ||
			(w->type == Figure_IndigenousNative && w->actionState == FigureActionState_159_NativeAttacking)) {
			int distance = Calc_distanceMaximum(x, y, w->x, w->y);
			if (distance <= maxDistance) {
				if (w->targetedByWalkerId) {
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
		struct Data_Walker *w = &Data_Walkers[i];
		if (WalkerIsEnemy(w->type) || w->type == Figure_Rioter ||
			(w->type == Figure_IndigenousNative && w->actionState == FigureActionState_159_NativeAttacking)) {
			return i;
		}
	}
	return 0;
}

int WalkerAction_CombatSoldier_getMissileTarget(int soldierId, int maxDistance, int *xTile, int *yTile)
{
	int x = Data_Walkers[soldierId].x;
	int y = Data_Walkers[soldierId].y;
	
	int minWalkerId = 0;
	int minDistance = maxDistance;
	for (int i = 1; i < MAX_FIGURES; i++) {
		if (FigureIsDead(i)) {
			continue;
		}
		struct Data_Walker *w = &Data_Walkers[i];
		if (WalkerIsEnemy(w->type) || WalkerIsHerd(w->type) ||
			(w->type == Figure_IndigenousNative && w->actionState == FigureActionState_159_NativeAttacking)) {
			int distance = Calc_distanceMaximum(x, y, w->x, w->y);
			if (distance < minDistance && WalkerMovement_canLaunchCrossCountryMissile(x, y, w->x, w->y)) {
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

int WalkerAction_CombatWolf_getTarget(int x, int y, int maxDistance)
{
	int minWalkerId = 0;
	int minDistance = 10000;
	for (int i = 1; i < MAX_FIGURES; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (FigureIsDead(i) || !w->type) {
			continue;
		}
		switch (w->type) {
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
		if (WalkerIsEnemy(w->type) || WalkerIsHerd(w->type)) {
			continue;
		}
		if (WalkerIsLegion(w->type) && w->actionState == FigureActionState_80_SoldierAtRest) {
			continue;
		}
		int distance = Calc_distanceMaximum(x, y, w->x, w->y);
		if (w->targetedByWalkerId) {
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

int WalkerAction_CombatEnemy_getTarget(int x, int y)
{
	int minWalkerId = 0;
	int minDistance = 10000;
	for (int i = 1; i < MAX_FIGURES; i++) {
		if (FigureIsDead(i)) {
			continue;
		}
		struct Data_Walker *w = &Data_Walkers[i];
		if (!w->targetedByWalkerId && WalkerIsLegion(w->type)) {
			int distance = Calc_distanceMaximum(x, y, w->x, w->y);
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

int WalkerAction_CombatEnemy_getMissileTarget(int enemyId, int maxDistance, int attackCitizens, int *xTile, int *yTile)
{
	int x = Data_Walkers[enemyId].x;
	int y = Data_Walkers[enemyId].y;
	
	int minWalkerId = 0;
	int minDistance = maxDistance;
	for (int i = 1; i < MAX_FIGURES; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (FigureIsDead(i) || !w->type) {
			continue;
		}
		switch (w->type) {
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
		if (WalkerIsLegion(w->type)) {
			distance = Calc_distanceMaximum(x, y, w->x, w->y);
		} else if (attackCitizens && w->isFriendly) {
			distance = Calc_distanceMaximum(x, y, w->x, w->y) + 5;
		} else {
			continue;
		}
		if (distance < minDistance && WalkerMovement_canLaunchCrossCountryMissile(x, y, w->x, w->y)) {
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


void WalkerAction_Combat_attackWalker(int walkerId, int opponentId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	int walkerCategory = Constant_WalkerProperties[w->type].category;
	if (walkerCategory <= WalkerCategory_Inactive || walkerCategory >= WalkerCategory_Criminal ||
			w->actionState == FigureActionState_150_Attack) {
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
		int opponentCategory = Constant_WalkerProperties[opponent->type].category;
		int attack = 0;
		if (opponent->state != FigureState_Alive) {
			attack = 0;
		} else if (opponent->actionState == FigureActionState_149_Corpse) {
			attack = 0;
		} else if (walkerCategory == WalkerCategory_Armed && opponentCategory == WalkerCategory_Native) {
			if (opponent->actionState == FigureActionState_159_NativeAttacking) {
				attack = 1;
			}
		} else if (walkerCategory == WalkerCategory_Armed && opponentCategory == WalkerCategory_Criminal) {
			attack = 1;
		} else if (walkerCategory == WalkerCategory_Armed && opponentCategory == WalkerCategory_Hostile) {
			attack = 1;
		} else if (walkerCategory == WalkerCategory_Hostile && opponentCategory == WalkerCategory_Citizen) {
			attack = 1;
		} else if (walkerCategory == WalkerCategory_Hostile && opponentCategory == WalkerCategory_Armed) {
			attack = 1;
		} else if (walkerCategory == WalkerCategory_Hostile && opponentCategory == WalkerCategory_Criminal) {
			attack = 1;
		} else if (walkerCategory == WalkerCategory_Armed && opponentCategory == WalkerCategory_Animal) {
			attack = 1;
		} else if (walkerCategory == WalkerCategory_Hostile && opponentCategory == WalkerCategory_Animal) {
			attack = 1;
		}
		if (attack && opponent->actionState == FigureActionState_150_Attack && opponent->numAttackers >= 2) {
			attack = 0;
		}
		if (attack) {
			w->actionStateBeforeAttack = w->actionState;
			w->actionState = FigureActionState_150_Attack;
			w->opponentId = opponentId;
			w->attackerId1 = opponentId;
			w->numAttackers = 1;
			w->attackGraphicOffset = 12;
			if (opponent->x != opponent->destinationX || opponent->y != opponent->destinationY) {
				w->attackDirection = Routing_getGeneralDirection(w->previousTileX, w->previousTileY,
					opponent->previousTileX, opponent->previousTileY);
			} else {
				w->attackDirection = Routing_getGeneralDirection(w->previousTileX, w->previousTileY,
					opponent->x, opponent->y);
			}
			if (w->attackDirection >= 8) {
				w->attackDirection = 0;
			}
			if (opponent->actionState != FigureActionState_150_Attack) {
				opponent->actionStateBeforeAttack = opponent->actionState;
				opponent->actionState = FigureActionState_150_Attack;
				opponent->attackGraphicOffset = 0;
				opponent->attackDirection = (w->attackDirection + 4) % 8;
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
