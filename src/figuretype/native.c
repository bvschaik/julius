#include "native.h"

#include "building/building.h"
#include "city/figures.h"
#include "city/military.h"
#include "figure/combat.h"
#include "figure/formation.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "map/terrain.h"

#include "Data/CityInfo.h"

void figure_indigenous_native_action(figure *f)
{
    building *b = building_get(f->buildingId);
    f->terrainUsage = FigureTerrainUsage_Any;
    f->useCrossCountry = 0;
    f->maxRoamLength = 800;
    if (b->state != BUILDING_STATE_IN_USE || b->figureId != f->id) {
        f->state = FigureState_Dead;
    }
    figure_image_increase_offset(f, 12);
    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_156_NATIVE_GOING_TO_MEETING_CENTER:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_157_NATIVE_RETURNING_FROM_MEETING;
                f->destinationX = f->sourceX;
                f->destinationY = f->sourceY;
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            }
            break;
        case FIGURE_ACTION_157_NATIVE_RETURNING_FROM_MEETING:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION ||
                f->direction == DIR_FIGURE_REROUTE ||
                f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            }
            break;
        case FIGURE_ACTION_158_NATIVE_CREATED:
            f->graphicOffset = 0;
            f->waitTicks++;
            if (f->waitTicks > 10 + (f->id & 3)) {
                f->waitTicks = 0;
                if (!city_military_is_native_attack_active()) {
                    int xTile, yTile;
                    building *meeting = building_get(b->subtype.nativeMeetingCenterId);
                    if (map_terrain_get_adjacent_road_or_clear_land(meeting->x, meeting->y, meeting->size, &xTile, &yTile)) {
                        f->actionState = FIGURE_ACTION_156_NATIVE_GOING_TO_MEETING_CENTER;
                        f->destinationX = xTile;
                        f->destinationY = yTile;
                    }
                } else {
                    const formation *m = formation_get(0);
                    f->actionState = FIGURE_ACTION_159_NATIVE_ATTACKING;
                    f->destinationX = m->destination_x;
                    f->destinationY = m->destination_y;
                    f->destinationBuildingId = m->destination_building_id;
                }
                figure_route_remove(f);
            }
            break;
        case FIGURE_ACTION_159_NATIVE_ATTACKING:
            Data_CityInfo.riotersOrAttackingNativesInCity = 10;
            city_figures_add_attacking_native();
            f->terrainUsage = FigureTerrainUsage_Enemy;
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION ||
                f->direction == DIR_FIGURE_REROUTE ||
                f->direction == DIR_FIGURE_LOST) {
                f->actionState = FIGURE_ACTION_158_NATIVE_CREATED;
            }
            break;
    }
    int dir;
    if (f->actionState == FIGURE_ACTION_150_ATTACK || f->direction == DIR_FIGURE_ATTACK) {
        dir = f->attackDirection;
    } else if (f->direction < 8) {
        dir = f->direction;
    } else {
        dir = f->previousTileDirection;
    }
    dir = figure_image_normalize_direction(dir);
    
    f->isEnemyGraphic = 1;
    if (f->actionState == FIGURE_ACTION_150_ATTACK) {
        if (f->attackGraphicOffset >= 12) {
            f->graphicId = 393 + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
        } else {
            f->graphicId = 393 + dir;
        }
    } else if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = 441 + figure_image_corpse_offset(f);
    } else if (f->direction == DIR_FIGURE_ATTACK) {
        f->graphicId = 393 + dir + 8 * (f->graphicOffset / 2);
    } else if (f->actionState == FIGURE_ACTION_159_NATIVE_ATTACKING) {
        f->graphicId = 297 + dir + 8 * f->graphicOffset;
    } else {
        f->graphicId = 201 + dir + 8 * f->graphicOffset;
    }
}
