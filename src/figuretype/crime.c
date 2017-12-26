#include "crime.h"

#include "building/building.h"
#include "city/message.h"
#include "figure/image.h"
#include "figure/route.h"
#include "graphics/image.h"
#include "map/building.h"
#include "map/grid.h"

#include "Data/CityInfo.h"
#include "../Building.h"
#include "../Formation.h"
#include "FigureAction.h"
#include "FigureMovement.h"

static const int CRIMINAL_OFFSETS[] = {
    0, 0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1
};

void figure_protestor_action(figure *f)
{
    f->terrainUsage = FigureTerrainUsage_Roads;
    figure_image_increase_offset(f, 64);
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
        f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) + figure_image_corpse_offset(f) + 96;
    } else {
        f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) + CRIMINAL_OFFSETS[f->graphicOffset / 4] + 104;
    }
}

void figure_criminal_action(figure *f)
{
    f->terrainUsage = FigureTerrainUsage_Roads;
    figure_image_increase_offset(f, 32);
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
        f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) + figure_image_corpse_offset(f) + 96;
    } else {
        f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) + CRIMINAL_OFFSETS[f->graphicOffset / 2] + 104;
    }
}

void figure_rioter_action(figure *f)
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
        case FIGURE_ACTION_120_RIOTER_CREATED:
            figure_image_increase_offset(f, 32);
            f->waitTicks++;
            if (f->waitTicks >= 160) {
                f->actionState = FIGURE_ACTION_121_RIOTER_MOVING;
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
        case FIGURE_ACTION_121_RIOTER_MOVING:
            figure_image_increase_offset(f, 12);
            FigureMovement_walkTicks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
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
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->actionState = FIGURE_ACTION_120_RIOTER_CREATED;
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_ATTACK) {
                if (f->graphicOffset > 12) {
                    f->graphicOffset = 0;
                }
            }
            break;
    }
    int dir;
    if (f->direction == DIR_FIGURE_ATTACK) {
        dir = f->attackDirection;
    } else if (f->direction < 8) {
        dir = f->direction;
    } else {
        dir = f->previousTileDirection;
    }
    dir = figure_image_normalize_direction(dir);
    
    if (f->actionState == FigureActionState_149_Corpse) {
        f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) + 96 + figure_image_corpse_offset(f);
    } else if (f->direction == DIR_FIGURE_ATTACK) {
        f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) + 104 + CRIMINAL_OFFSETS[f->graphicOffset];
    } else if (f->actionState == FIGURE_ACTION_121_RIOTER_MOVING) {
        f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) + dir + 8 * f->graphicOffset;
    } else {
        f->graphicId = image_group(GROUP_FIGURE_CRIMINAL) + 104 + CRIMINAL_OFFSETS[f->graphicOffset / 2];
    }
}

int figure_rioter_collapse_building(figure *f)
{
    for (int dir = 0; dir < 8; dir += 2) {
        int gridOffset = f->gridOffset + map_grid_direction_delta(dir);
        if (!map_building_at(gridOffset)) {
            continue;
        }
        building *b = building_get(map_building_at(gridOffset));
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
        city_message_apply_sound_interval(MESSAGE_CAT_RIOT_COLLAPSE);
        city_message_post(0, MESSAGE_DESTROYED_BUILDING, b->type, f->gridOffset);
        city_message_increase_category_count(MESSAGE_CAT_RIOT_COLLAPSE);
        Building_collapseOnFire(b->id, 0);
        f->actionState = FIGURE_ACTION_120_RIOTER_CREATED;
        f->waitTicks = 0;
        f->direction = dir;
        return 1;
    }
    return 0;
}
