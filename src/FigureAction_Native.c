#include "FigureAction_private.h"

#include "Terrain.h"

#include "Data/CityInfo.h"

#include "building/building.h"
#include "figure/formation.h"
#include "figure/route.h"

void FigureAction_indigenousNative(figure *f)
{
	building *b = building_get(f->buildingId);
	f->terrainUsage = FigureTerrainUsage_Any;
	f->useCrossCountry = 0;
	f->maxRoamLength = 800;
	if (!BuildingIsInUse(f->buildingId) || b->figureId != f->id) {
		f->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(f, 12);
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(f);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(f);
			break;
		case FigureActionState_156_NativeGoingToMeetingCenter:
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DIR_FIGURE_AT_DESTINATION) {
				f->actionState = FigureActionState_157_NativeReturningFromMeetingCenter;
				f->destinationX = f->sourceX;
				f->destinationY = f->sourceY;
			} else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_157_NativeReturningFromMeetingCenter:
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DIR_FIGURE_AT_DESTINATION ||
				f->direction == DIR_FIGURE_REROUTE ||
				f->direction == DIR_FIGURE_LOST) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_158_NativeCreated:
			f->graphicOffset = 0;
			f->waitTicks++;
			if (f->waitTicks > 10 + (f->id & 3)) {
				f->waitTicks = 0;
				if (Data_CityInfo.nativeAttackDuration == 0) {
					int xTile, yTile;
					building *meeting = building_get(b->subtype.nativeMeetingCenterId);
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
				figure_route_remove(f);
			}
			break;
		case FigureActionState_159_NativeAttacking:
			Data_CityInfo.riotersOrAttackingNativesInCity = 10;
			Data_CityInfo.numAttackingNativesInCity++;
			f->terrainUsage = FigureTerrainUsage_Enemy;
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DIR_FIGURE_AT_DESTINATION ||
				f->direction == DIR_FIGURE_REROUTE ||
				f->direction == DIR_FIGURE_LOST) {
				f->actionState = FigureActionState_158_NativeCreated;
			}
			break;
	}
	int dir;
	if (f->actionState == FigureActionState_150_Attack || f->direction == DIR_FIGURE_ATTACK) {
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
	} else if (f->direction == DIR_FIGURE_ATTACK) {
		f->graphicId = 393 + dir + 8 * (f->graphicOffset / 2);
	} else if (f->actionState == FigureActionState_159_NativeAttacking) {
		f->graphicId = 297 + dir + 8 * f->graphicOffset;
	} else {
		f->graphicId = 201 + dir + 8 * f->graphicOffset;
	}
}
