#include "WalkerAction_private.h"

#include "Terrain.h"
#include "Walker.h"

#include "Data/CityInfo.h"
#include "Data/Formation.h"

void WalkerAction_indigenousNative(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	w->terrainUsage = 0;
	w->useCrossCountry = 0;
	w->maxRoamLength = 800;
	if (b->inUse != 1 || b->walkerId != walkerId) {
		w->state = WalkerState_Dead;
	}
	WalkerActionIncreaseGraphicOffset(w, 12);
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_156_NativeGoingToMeetingCenter:
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == 8) {
				w->actionState = WalkerActionState_157_NativeReturningFromMeetingCenter;
				w->destinationX = w->sourceX;
				w->destinationY = w->sourceY;
			} else if (w->direction == 9 || w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_157_NativeReturningFromMeetingCenter:
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == 8 || w->direction == 9 || w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_158_NativeCreated:
			w->graphicOffset = 0;
			w->waitTicks++;
			if (w->waitTicks > 10 + (walkerId & 3)) {
				w->waitTicks = 0;
				if (Data_CityInfo.nativeAttackDuration == 0) {
					int xTile, yTile;
					struct Data_Building *mc = &Data_Buildings[b->subtype.nativeMeetingCenterId];
					if (Terrain_getAdjacentRoadOrClearLand(mc->x, mc->y, mc->size, &xTile, &yTile)) {
						w->actionState = WalkerActionState_156_NativeGoingToMeetingCenter;
						w->destinationX = xTile;
						w->destinationY = yTile;
					}
				} else {
					w->actionState = WalkerActionState_159_NativeAttacking;
					w->destinationX = Data_Formations[0].destinationX;
					w->destinationY = Data_Formations[0].destinationY;
					w->destinationBuildingId = Data_Formations[0].destinationBuildingId;
				}
				WalkerRoute_remove(walkerId);
			}
			break;
		case WalkerActionState_159_NativeAttacking:
			Data_CityInfo.riotersOrAttackingNativesInCity = 10;
			Data_CityInfo.numAttackingNativesInCity++;
			w->terrainUsage = 2;
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == 8 || w->direction == 9 || w->direction == 10) {
				w->actionState = WalkerActionState_158_NativeCreated;
			}
			break;
	}
	int dir;
	if (w->actionState == WalkerActionState_150_Attack || w->direction == 11) {
		dir = w->attackDirection;
	} else if (w->direction < 8) {
		dir = w->direction;
	} else {
		dir = w->previousTileDirection;
	}
	WalkerActionNormalizeDirection(dir);
	
	w->isEnemyGraphic = 1;
	if (w->actionState == WalkerActionState_150_Attack) {
		if (w->attackGraphicOffset >= 12) {
			w->graphicId = 393 + dir + 8 * ((w->attackGraphicOffset - 12) / 2);
		} else {
			w->graphicId = 393 + dir;
		}
	} else if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = 441 + WalkerActionCorpseGraphicOffset(w);
	} else if (w->direction == 11) {
		w->graphicId = 393 + dir + 8 * (w->graphicOffset / 2);
	} else if (w->actionState == WalkerActionState_159_NativeAttacking) {
		w->graphicId = 297 + dir + 8 * w->graphicOffset;
	} else {
		w->graphicId = 201 + dir + 8 * w->graphicOffset;
	}
}
