#include "FigureAction_private.h"

#include "Figure.h"
#include "Terrain.h"

#include "Data/CityInfo.h"
#include "Data/Formation.h"

void FigureAction_indigenousNative(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Building *b = &Data_Buildings[w->buildingId];
	w->terrainUsage = FigureTerrainUsage_Any;
	w->useCrossCountry = 0;
	w->maxRoamLength = 800;
	if (!BuildingIsInUse(w->buildingId) || b->walkerId != walkerId) {
		w->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(w, 12);
	switch (w->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_156_NativeGoingToMeetingCenter:
			FigureMovement_walkTicks(walkerId, 1);
			if (w->direction == DirFigure_8_AtDestination) {
				w->actionState = FigureActionState_157_NativeReturningFromMeetingCenter;
				w->destinationX = w->sourceX;
				w->destinationY = w->sourceY;
			} else if (w->direction == DirFigure_9_Reroute || w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			}
			break;
		case FigureActionState_157_NativeReturningFromMeetingCenter:
			FigureMovement_walkTicks(walkerId, 1);
			if (w->direction == DirFigure_8_AtDestination ||
				w->direction == DirFigure_9_Reroute ||
				w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			}
			break;
		case FigureActionState_158_NativeCreated:
			w->graphicOffset = 0;
			w->waitTicks++;
			if (w->waitTicks > 10 + (walkerId & 3)) {
				w->waitTicks = 0;
				if (Data_CityInfo.nativeAttackDuration == 0) {
					int xTile, yTile;
					struct Data_Building *mc = &Data_Buildings[b->subtype.nativeMeetingCenterId];
					if (Terrain_getAdjacentRoadOrClearLand(mc->x, mc->y, mc->size, &xTile, &yTile)) {
						w->actionState = FigureActionState_156_NativeGoingToMeetingCenter;
						w->destinationX = xTile;
						w->destinationY = yTile;
					}
				} else {
					w->actionState = FigureActionState_159_NativeAttacking;
					w->destinationX = Data_Formations[0].destinationX;
					w->destinationY = Data_Formations[0].destinationY;
					w->destinationBuildingId = Data_Formations[0].destinationBuildingId;
				}
				FigureRoute_remove(walkerId);
			}
			break;
		case FigureActionState_159_NativeAttacking:
			Data_CityInfo.riotersOrAttackingNativesInCity = 10;
			Data_CityInfo.numAttackingNativesInCity++;
			w->terrainUsage = FigureTerrainUsage_Enemy;
			FigureMovement_walkTicks(walkerId, 1);
			if (w->direction == DirFigure_8_AtDestination ||
				w->direction == DirFigure_9_Reroute ||
				w->direction == DirFigure_10_Lost) {
				w->actionState = FigureActionState_158_NativeCreated;
			}
			break;
	}
	int dir;
	if (w->actionState == FigureActionState_150_Attack || w->direction == DirFigure_11_Attack) {
		dir = w->attackDirection;
	} else if (w->direction < 8) {
		dir = w->direction;
	} else {
		dir = w->previousTileDirection;
	}
	WalkerActionNormalizeDirection(dir);
	
	w->isEnemyGraphic = 1;
	if (w->actionState == FigureActionState_150_Attack) {
		if (w->attackGraphicOffset >= 12) {
			w->graphicId = 393 + dir + 8 * ((w->attackGraphicOffset - 12) / 2);
		} else {
			w->graphicId = 393 + dir;
		}
	} else if (w->actionState == FigureActionState_149_Corpse) {
		w->graphicId = 441 + WalkerActionCorpseGraphicOffset(w);
	} else if (w->direction == DirFigure_11_Attack) {
		w->graphicId = 393 + dir + 8 * (w->graphicOffset / 2);
	} else if (w->actionState == FigureActionState_159_NativeAttacking) {
		w->graphicId = 297 + dir + 8 * w->graphicOffset;
	} else {
		w->graphicId = 201 + dir + 8 * w->graphicOffset;
	}
}
