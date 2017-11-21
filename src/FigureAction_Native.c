#include "FigureAction_private.h"

#include "Terrain.h"

#include "Data/CityInfo.h"

#include "figure/formation.h"
#include "figure/route.h"

void FigureAction_indigenousNative(int figureId)
{
	struct Data_Figure *f = &Data_Figures[figureId];
	struct Data_Building *b = &Data_Buildings[f->buildingId];
	f->terrainUsage = FigureTerrainUsage_Any;
	f->useCrossCountry = 0;
	f->maxRoamLength = 800;
	if (!BuildingIsInUse(f->buildingId) || b->figureId != figureId) {
		f->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(f, 12);
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(figureId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(figureId);
			break;
		case FigureActionState_156_NativeGoingToMeetingCenter:
			FigureMovement_walkTicks(figureId, 1);
			if (f->direction == DirFigure_8_AtDestination) {
				f->actionState = FigureActionState_157_NativeReturningFromMeetingCenter;
				f->destinationX = f->sourceX;
				f->destinationY = f->sourceY;
			} else if (f->direction == DirFigure_9_Reroute || f->direction == DirFigure_10_Lost) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_157_NativeReturningFromMeetingCenter:
			FigureMovement_walkTicks(figureId, 1);
			if (f->direction == DirFigure_8_AtDestination ||
				f->direction == DirFigure_9_Reroute ||
				f->direction == DirFigure_10_Lost) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_158_NativeCreated:
			f->graphicOffset = 0;
			f->waitTicks++;
			if (f->waitTicks > 10 + (figureId & 3)) {
				f->waitTicks = 0;
				if (Data_CityInfo.nativeAttackDuration == 0) {
					int xTile, yTile;
					struct Data_Building *meeting = &Data_Buildings[b->subtype.nativeMeetingCenterId];
					if (Terrain_getAdjacentRoadOrClearLand(meeting->x, meeting->y, meeting->size, &xTile, &yTile)) {
						f->actionState = FigureActionState_156_NativeGoingToMeetingCenter;
						f->destinationX = xTile;
						f->destinationY = yTile;
					}
				} else {
                    const formation *m = formation_get(0);
					f->actionState = FigureActionState_159_NativeAttacking;
					f->destinationX = m->destination_x;
					f->destinationY = m->destination_y;
					f->destinationBuildingId = m->destination_building_id;
				}
				figure_route_remove(figureId);
			}
			break;
		case FigureActionState_159_NativeAttacking:
			Data_CityInfo.riotersOrAttackingNativesInCity = 10;
			Data_CityInfo.numAttackingNativesInCity++;
			f->terrainUsage = FigureTerrainUsage_Enemy;
			FigureMovement_walkTicks(figureId, 1);
			if (f->direction == DirFigure_8_AtDestination ||
				f->direction == DirFigure_9_Reroute ||
				f->direction == DirFigure_10_Lost) {
				f->actionState = FigureActionState_158_NativeCreated;
			}
			break;
	}
	int dir;
	if (f->actionState == FigureActionState_150_Attack || f->direction == DirFigure_11_Attack) {
		dir = f->attackDirection;
	} else if (f->direction < 8) {
		dir = f->direction;
	} else {
		dir = f->previousTileDirection;
	}
	FigureActionNormalizeDirection(dir);
	
	f->isEnemyGraphic = 1;
	if (f->actionState == FigureActionState_150_Attack) {
		if (f->attackGraphicOffset >= 12) {
			f->graphicId = 393 + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
		} else {
			f->graphicId = 393 + dir;
		}
	} else if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = 441 + FigureActionCorpseGraphicOffset(f);
	} else if (f->direction == DirFigure_11_Attack) {
		f->graphicId = 393 + dir + 8 * (f->graphicOffset / 2);
	} else if (f->actionState == FigureActionState_159_NativeAttacking) {
		f->graphicId = 297 + dir + 8 * f->graphicOffset;
	} else {
		f->graphicId = 201 + dir + 8 * f->graphicOffset;
	}
}
