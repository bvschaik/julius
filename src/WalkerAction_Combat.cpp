#include "WalkerAction_private.h"

#include "Calc.h"
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

int WalkerAction_CombatEnemy_getTargetForMissile(int enemyId, int maxDistance, int attackCitizens, int *xTile, int *yTile)
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


void WalkerAction_CombatSoldier_attackWalker(int walkerId, int targetWalkerId)
{
	// TODO
}
