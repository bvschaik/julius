#include "WalkerAction_private.h"

#include "Calc.h"
#include "Routing.h"
#include "WalkerMovement.h"

int WalkerAction_CombatSoldier_getTarget(int x, int y, int maxDistance)
{
	int minWalkerId = 0;
	int minDistance = 10000;
	for (int i = 1; i < MAX_WALKERS; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state != WalkerState_Alive || w->actionState == WalkerActionState_149_Corpse) {
			continue;
		}
		if (WalkerIsEnemy(w->type) || w->type == Walker_Rioter ||
			(w->type == Walker_IndigenousNative && w->actionState == WalkerActionState_159_NativeAttacking)) {
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
	for (int i = 1; i < MAX_WALKERS; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state != WalkerState_Alive || w->actionState == WalkerActionState_149_Corpse) {
			continue;
		}
		if (WalkerIsEnemy(w->type) || w->type == Walker_Rioter ||
			(w->type == Walker_IndigenousNative && w->actionState == WalkerActionState_159_NativeAttacking)) {
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
	for (int i = 1; i < MAX_WALKERS; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state != WalkerState_Alive || w->actionState == WalkerActionState_149_Corpse) {
			continue;
		}
		if (WalkerIsEnemy(w->type) || WalkerIsHerd(w->type) ||
			(w->type == Walker_IndigenousNative && w->actionState == WalkerActionState_159_NativeAttacking)) {
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
	for (int i = 1; i < MAX_WALKERS; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state != WalkerState_Alive || w->actionState == WalkerActionState_149_Corpse || !w->type) {
			continue;
		}
		switch (w->type) {
			case Walker_Explosion:
			case Walker_FortStandard:
			case Walker_TradeShip:
			case Walker_FishingBoat:
			case Walker_MapFlag:
			case Walker_Flotsam:
			case Walker_Shipwreck:
			case Walker_IndigenousNative:
			case Walker_TowerSentry:
			case Walker_NativeTrader:
			case Walker_Arrow:
			case Walker_Javelin:
			case Walker_Bolt:
			case Walker_Ballista:
			case Walker_Creature:
				continue;
		}
		if (WalkerIsEnemy(w->type) || WalkerIsHerd(w->type)) {
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
	for (int i = 1; i < MAX_WALKERS; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state != WalkerState_Alive || w->actionState == WalkerActionState_149_Corpse) {
			continue;
		}
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
	for (int i = 1; i < MAX_WALKERS; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state != WalkerState_Alive || w->actionState == WalkerActionState_149_Corpse) {
			continue;
		}
		if (WalkerIsLegion(w->type)) {
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
	for (int i = 1; i < MAX_WALKERS; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state != WalkerState_Alive || w->actionState == WalkerActionState_149_Corpse || !w->type) {
			continue;
		}
		switch (w->type) {
			case Walker_Explosion:
			case Walker_FortStandard:
			case Walker_MapFlag:
			case Walker_Flotsam:
			case Walker_IndigenousNative:
			case Walker_NativeTrader:
			case Walker_Arrow:
			case Walker_Javelin:
			case Walker_Bolt:
			case Walker_Ballista:
			case Walker_Creature:
			case Walker_FishGulls:
			case Walker_Shipwreck:
			case Walker_Sheep:
			case Walker_Wolf:
			case Walker_Zebra:
			case Walker_Spear:
				continue;
		}
		if (WalkerIsLegion(w->type) || (attackCitizens && w->isFriendly)) {
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


void WalkerAction_Combat_attackWalker(int walkerId, int opponentId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	int walkerCategory = Constant_WalkerProperties[w->type].category;
	if (walkerCategory <= WalkerCategory_Inactive || walkerCategory >= WalkerCategory_Criminal ||
			w->actionState == WalkerActionState_150_Attack) {
		return;
	}
	int guard = 0;
	while (1) {
		if (++guard >= 1000 || opponentId <= 0) {
			break;
		}
		if (opponentId == walkerId) {
			opponentId = Data_Walkers[opponentId].nextWalkerIdOnSameTile;
			continue;
		}
		struct Data_Walker *opponent = &Data_Walkers[opponentId];
		int opponentCategory = Constant_WalkerProperties[opponent->type].category;
		int attack = 0;
		if (opponent->state != WalkerState_Alive) {
			attack = 0;
		} else if (opponent->actionState == WalkerActionState_149_Corpse) {
			attack = 0;
		} else if (walkerCategory == WalkerCategory_Armed && opponentCategory == WalkerCategory_Native) {
			if (opponent->actionState == WalkerActionState_159_NativeAttacking) {
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
		if (attack && opponent->actionState == WalkerActionState_150_Attack && opponent->numAttackers >= 2) {
			attack = 0;
		}
		if (attack) {
			w->actionStateBeforeAttack = w->actionState;
			w->actionState = WalkerActionState_150_Attack;
			w->opponentId = opponentId;
			w->attackerId1 = opponentId;
			w->numAttackers = 1;
			w->attackGraphicOffset = 12;
			if (opponent->x != opponent->destinationX || opponent->y != opponent->destinationY) {
				w->attackDirection = Routing_getGeneralDirection(w->previousTileX, w->previousTileY,
					opponent->previousTileX, opponent->previousTileY);
			} else {
				w->attackDirection = Routing_getGeneralDirection(w->previousTileX, w->previousTileY,
					opponent->x, opponent->x);
			}
			if (w->attackDirection >= 8) {
				w->attackDirection = 0;
			}
			if (opponent->actionState != WalkerActionState_150_Attack) {
				opponent->actionStateBeforeAttack = opponent->actionState;
				opponent->actionState = WalkerActionState_150_Attack;
				opponent->attackGraphicOffset = 0;
				opponent->attackDirection = w->attackDirection + 4;
				if (opponent->attackDirection >= 8) {
					opponent->attackDirection -= 8;
				}
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
