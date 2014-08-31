#include "WalkerAction_private.h"

#include "Routing.h"
#include "Sound.h"
#include "Walker.h"

#include "Data/Formation.h"

static int ballistaFiringOffsets[] = {
	0, 1, 2, 3, 4, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void WalkerAction_ballista(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	w->terrainUsage = WalkerTerrainUsage_Walls;
	w->useCrossCountry = 0;
	w->isGhost = 1;
	w->heightFromGround = 10;
	w->currentHeight = 45;
	
	if (b->inUse != 1 || b->walkerId4 != walkerId) {
		w->state = WalkerState_Dead;
	}
	if (b->numWorkers <= 0 || b->walkerId <= 0) {
		w->state = WalkerState_Dead;
	}
	Walker_removeFromTileList(walkerId);
	switch (Data_Settings_Map.orientation) {
		case 0: w->x = b->x; w->y = b->y; break;
		case 2: w->x = b->x + 1; w->y = b->y; break;
		case 4: w->x = b->x + 1; w->y = b->y + 1; break;
		case 6: w->x = b->x; w->y = b->y + 1; break;
	}
	w->gridOffset = GridOffset(w->x, w->y);
	Walker_addToTileList(walkerId);

	switch (w->actionState) {
		case WalkerActionState_149_Corpse:
			w->state = WalkerState_Dead;
			break;
		case WalkerActionState_180_BallistaCreated:
			w->waitTicks++;
			if (w->waitTicks > 20) {
				w->waitTicks = 0;
				int xTile, yTile;
				if (WalkerAction_CombatSoldier_getMissileTarget(walkerId, 15, &xTile, &yTile)) {
					w->actionState = WalkerActionState_181_BallistaFiring;
					w->waitTicksMissile = Constant_WalkerProperties[w->type].missileFrequency;
				}
			}
			break;
		case WalkerActionState_181_BallistaFiring:
			w->waitTicksMissile++;
			if (w->waitTicksMissile > Constant_WalkerProperties[w->type].missileFrequency) {
				int xTile, yTile;
				if (WalkerAction_CombatSoldier_getMissileTarget(walkerId, 15, &xTile, &yTile)) {
					w->direction = Routing_getGeneralDirection(w->x, w->y, xTile, yTile);
					w->waitTicksMissile = 0;
					Walker_createMissile(walkerId, w->x, w->y, xTile, yTile, Walker_Bolt);
					Sound_Effects_playChannel(SoundChannel_BallistaShoot);
				} else {
					w->actionState = WalkerActionState_180_BallistaCreated;
				}
			}
			break;
	}
	int dir = (8 + w->direction - Data_Settings_Map.orientation) % 8;
	if (w->actionState == WalkerActionState_181_BallistaFiring) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Ballista) + dir +
			8 * ballistaFiringOffsets[w->waitTicksMissile / 2];
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_Ballista) + dir;
	}
}

static void towerSentryPickTarget(int walkerId, struct Data_Walker *w)
{
	if (Data_Formation_Extra.numEnemyFormations <= 0) {
		return;
	}
	if (w->actionState == WalkerActionState_150_Attack ||
		w->actionState == WalkerActionState_149_Corpse) {
		return;
	}
	if (w->inBuildingWaitTicks) {
		return;
	}
	w->waitTicksNextTarget++;
	if (w->waitTicksNextTarget >= 40) {
		w->waitTicksNextTarget = 0;
		int xTile, yTile;
		if (WalkerAction_CombatSoldier_getMissileTarget(walkerId, 10, &xTile, &yTile)) {
			w->actionState = WalkerActionState_172_TowerSentryFiring;
			w->destinationX = w->x;
			w->destinationY = w->y;
		}
	}
}

void WalkerAction_towerSentry(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	w->terrainUsage = WalkerTerrainUsage_Walls;
	w->useCrossCountry = 0;
	w->isGhost = 1;
	w->heightFromGround = 10;
	w->maxRoamLength = 800;
	if (b->inUse != 1 || b->walkerId != walkerId) {
		w->state = WalkerState_Dead;
	}
	WalkerActionIncreaseGraphicOffset(w, 12);
	
	towerSentryPickTarget(walkerId, w);
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		// TODO
	}
	// TODO graphic
}
