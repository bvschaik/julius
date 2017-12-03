#include "FigureAction_private.h"

#include "Building.h"
#include "Formation.h"

#include "Data/CityInfo.h"

#include "city/message.h"
#include "figure/route.h"
#include "map/building.h"
#include "map/grid.h"

static const int criminalOffsets[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1
};

void FigureAction_protestor(figure *f)
{
	f->terrainUsage = FigureTerrainUsage_Roads;
	FigureActionIncreaseGraphicOffset(f, 64);
	f->cartGraphicId = 0;
	if (f->actionState == FigureActionState_149_Corpse) {
		f->state = FigureState_Dead;
	}
	f->waitTicks++;
	if (f->waitTicks > 200) {
		f->state = FigureState_Dead;
		f->graphicOffset = 0;
	}
	if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) +
			FigureActionCorpseGraphicOffset(f) + 96;
	} else {
		f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) +
			criminalOffsets[f->graphicOffset / 4] + 104;
	}
}

void FigureAction_criminal(figure *f)
{
	f->terrainUsage = FigureTerrainUsage_Roads;
	FigureActionIncreaseGraphicOffset(f, 32);
	f->cartGraphicId = 0;
	if (f->actionState == FigureActionState_149_Corpse) {
		f->state = FigureState_Dead;
	}
	f->waitTicks++;
	if (f->waitTicks > 200) {
		f->state = FigureState_Dead;
		f->graphicOffset = 0;
	}
	if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) +
			FigureActionCorpseGraphicOffset(f) + 96;
	} else {
		f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) +
			criminalOffsets[f->graphicOffset / 2] + 104;
	}
}

void FigureAction_rioter(figure *f)
{
	Data_CityInfo.numRiotersInCity++;
	if (!f->targetedByFigureId) {
		Data_CityInfo.riotersOrAttackingNativesInCity = 10;
	}
	f->terrainUsage = FigureTerrainUsage_Enemy;
	f->maxRoamLength = 480;
	f->cartGraphicId = 0;
	f->isGhost = 0;
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(f);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(f);
			break;
		case FigureActionState_120_RioterCreated:
			FigureActionIncreaseGraphicOffset(f, 32);
			f->waitTicks++;
			if (f->waitTicks >= 160) {
				f->actionState = FigureActionState_121_RioterMoving;
				int xTile, yTile;
				int buildingId = Formation_Rioter_getTargetBuilding(&xTile, &yTile);
				if (buildingId) {
					f->destinationX = xTile;
					f->destinationY = yTile;
					f->destinationBuildingId = buildingId;
					figure_route_remove(f);
				} else {
					f->state = FigureState_Dead;
				}
			}
			break;
		case FigureActionState_121_RioterMoving:
			FigureActionIncreaseGraphicOffset(f, 12);
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DirFigure_8_AtDestination) {
				int xTile, yTile;
				int buildingId = Formation_Rioter_getTargetBuilding(&xTile, &yTile);
				if (buildingId) {
					f->destinationX = xTile;
					f->destinationY = yTile;
					f->destinationBuildingId = buildingId;
					figure_route_remove(f);
				} else {
					f->state = FigureState_Dead;
				}
			} else if (f->direction == DirFigure_9_Reroute || f->direction == DirFigure_10_Lost) {
				f->actionState = FigureActionState_120_RioterCreated;
				figure_route_remove(f);
			} else if (f->direction == DirFigure_11_Attack) {
				if (f->graphicOffset > 12) {
					f->graphicOffset = 0;
				}
			}
			break;
	}
	int dir;
	if (f->direction == DirFigure_11_Attack) {
		dir = f->attackDirection;
	} else if (f->direction < 8) {
		dir = f->direction;
	} else {
		dir = f->previousTileDirection;
	}
	FigureActionNormalizeDirection(dir);
	
	if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) +
			96 + FigureActionCorpseGraphicOffset(f);
	} else if (f->direction == DirFigure_11_Attack) {
		f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) +
			104 + criminalOffsets[f->graphicOffset];
	} else if (f->actionState == FigureActionState_121_RioterMoving) {
		f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) +
			dir + 8 * f->graphicOffset;
	} else {
		f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) +
			104 + criminalOffsets[f->graphicOffset / 2];
	}
}

int FigureAction_Rioter_collapseBuilding(figure *f)
{
	for (int dir = 0; dir < 8; dir += 2) {
		int gridOffset = f->gridOffset + map_grid_direction_delta(dir);
		if (!map_building_at(gridOffset)) {
			continue;
		}
		int buildingId = map_building_at(gridOffset);
		struct Data_Building *b = &Data_Buildings[buildingId];
		switch (b->type) {
			case BUILDING_WAREHOUSE_SPACE:
			case BUILDING_WAREHOUSE:
			case BUILDING_FORT_GROUND:
			case BUILDING_FORT:
			case BUILDING_BURNING_RUIN:
				continue;
		}
		if (b->houseSize && b->subtype.houseLevel < HOUSE_SMALL_CASA) {
			continue;
		}
		game.messages.apply_sound_interval(MESSAGE_CAT_RIOT_COLLAPSE);
		game.messages.post(0, MESSAGE_DESTROYED_BUILDING, b->type, f->gridOffset);
		game.messages.increase_category_count(MESSAGE_CAT_RIOT_COLLAPSE);
		Building_collapseOnFire(buildingId, 0);
		f->actionState = FigureActionState_120_RioterCreated;
		f->waitTicks = 0;
		f->direction = dir;
		return 1;
	}
	return 0;
}
