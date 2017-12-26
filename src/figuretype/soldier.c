#include "soldier.h"

#include "core/calc.h"
#include "figure/combat.h"
#include "figure/formation.h"
#include "figure/formation_layout.h"
#include "figure/image.h"
#include "figure/properties.h"
#include "figure/route.h"
#include "figuretype/missile.h"
#include "graphics/image.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/point.h"

#include "Data/CityInfo.h"
#include "FigureMovement.h"

static const map_point ALTERNATIVE_POINTS[] = {{-1, -6},
    {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1},
    {0, -2}, {1, -2}, {2, -2}, {2, -1}, {2, 0}, {2, 1}, {2, 2}, {1, 2},
    {0, 2}, {-1, 2}, {-2, 2}, {-2, 1}, {-2, 0}, {-2, -1}, {-2, -2}, {-1, -2},
    {0, -3}, {1, -3}, {2, -3}, {3, -3}, {3, -2}, {3, -1}, {3, 0}, {3, 1},
    {3, 2}, {3, 3}, {2, 3}, {1, 3}, {0, 3}, {-1, 3}, {-2, 3}, {-3, 3},
    {-3, 2}, {-3, 1}, {-3, 0}, {-3, -1}, {-3, -2}, {-3, -3}, {-2, -3}, {-1, -3},
    {0, -4}, {1, -4}, {2, -4}, {3, -4}, {4, -4}, {4, -3}, {4, -2}, {4, -1},
    {4, 0}, {4, 1}, {4, 2}, {4, 3}, {4, 4}, {3, 4}, {2, 4}, {1, 4},
    {0, 4}, {-1, 4}, {-2, 4}, {-3, 4}, {-4, 4}, {-4, 3}, {-4, 2}, {-4, 1},
    {-4, 0}, {-4, -1}, {-4, -2}, {-4, -3}, {-4, -4}, {-3, -4}, {-2, -4}, {-1, -4},
    {0, -5}, {1, -5}, {2, -5}, {3, -5}, {4, -5}, {5, -5}, {5, -4}, {5, -3},
    {5, -2}, {5, -1}, {5, 0}, {5, 1}, {5, 2}, {5, 3}, {5, 4}, {5, 5},
    {4, 5}, {3, 5}, {2, 5}, {1, 5}, {0, 5}, {-1, 5}, {-2, 5}, {-3, 5},
    {-4, 5}, {-5, 5}, {-5, 4}, {-5, 3}, {-5, 2}, {-5, 1}, {-5, 0}, {-5, -1},
    {-5, -2}, {-5, -3}, {-5, -4}, {-5, -5}, {-4, -5}, {-3, -5}, {-2, -5}, {-1, -5},
    {0, -6}, {1, -6}, {2, -6}, {3, -6}, {4, -6}, {5, -6}, {6, -6}, {6, -5},
    {6, -4}, {6, -3}, {6, -2}, {6, -1}, {6, 0}, {6, 1}, {6, 2}, {6, 3},
    {6, 4}, {6, 5}, {6, 6}, {5, 6}, {4, 6}, {3, 6}, {2, 6}, {1, 6},
    {0, 6}, {-1, 6}, {-2, 6}, {-3, 6}, {-4, 6}, {-5, 6}, {-6, 6}, {-6, 5},
    {-6, 4}, {-6, 3}, {-6, 2}, {-6, 1}, {-6, 0}, {-6, -1}, {-6, -2}, {-6, -3},
    {-6, -4}, {-6, -5}, {-6, -6}, {-5, -6}, {-4, -6}, {-3, -6}, {-2, -6}, {-1, -6},
};

void figure_military_standard_action(figure *f)
{
    const formation *m = formation_get(f->formationId);

    f->terrainUsage = FigureTerrainUsage_Any;
    figure_image_increase_offset(f, 16);
    map_figure_delete(f);
    if (m->is_at_fort) {
        f->x = m->x;
        f->y = m->y;
    } else {
        f->x = m->standard_x;
        f->y = m->standard_y;
    }
    f->gridOffset = map_grid_offset(f->x, f->y);
    f->crossCountryX = 15 * f->x + 7;
    f->crossCountryY = 15 * f->y + 7;
    map_figure_add(f);

    f->graphicId = image_group(GROUP_FIGURE_FORT_STANDARD_POLE) + 20 - m->morale / 5;
    if (m->figure_type == FIGURE_FORT_LEGIONARY) {
        if (m->is_halted) {
            f->cartGraphicId = image_group(GROUP_FIGURE_FORT_FLAGS) + 8;
        } else {
            f->cartGraphicId = image_group(GROUP_FIGURE_FORT_FLAGS) + f->graphicOffset / 2;
        }
    } else if (m->figure_type == FIGURE_FORT_MOUNTED) {
        if (m->is_halted) {
            f->cartGraphicId = image_group(GROUP_FIGURE_FORT_FLAGS) + 26;
        } else {
            f->cartGraphicId = image_group(GROUP_FIGURE_FORT_FLAGS) + 18 + f->graphicOffset / 2;
        }
    } else {
        if (m->is_halted) {
            f->cartGraphicId = image_group(GROUP_FIGURE_FORT_FLAGS) + 17;
        } else {
            f->cartGraphicId = image_group(GROUP_FIGURE_FORT_FLAGS) + 9 + f->graphicOffset / 2;
        }
    }
}

static void javelin_launch_missile(figure *f)
{
    int x_tile, y_tile;
    f->waitTicksMissile++;
    if (f->waitTicksMissile > figure_properties_for_type(f->type)->missile_delay) {
        f->waitTicksMissile = 0;
        if (figure_combat_get_missile_target_for_soldier(f, 10, &x_tile, &y_tile)) {
            f->attackGraphicOffset = 1;
            f->direction = calc_missile_shooter_direction(f->x, f->y, x_tile, y_tile);
        } else {
            f->attackGraphicOffset = 0;
        }
    }
    if (f->attackGraphicOffset) {
        if (f->attackGraphicOffset == 1) {
            figure_create_missile(f->id, f->x, f->y, x_tile, y_tile, FIGURE_JAVELIN);
            formation_record_missile_fired(f->formationId);
        }
        f->attackGraphicOffset++;
        if (f->attackGraphicOffset > 100) {
            f->attackGraphicOffset = 0;
        }
    }
}

static void legionary_attack_adjacent_enemy(figure *f)
{
    for (int i = 0; i < 8 && f->actionState != FIGURE_ACTION_150_ATTACK; i++) {
        figure_combat_attack_figure_at(f, f->gridOffset + map_grid_direction_delta(i));
    }
}

static int find_mop_up_target(figure *f)
{
    int target_id = f->targetFigureId;
    if (figure_is_dead(figure_get(target_id))) {
        f->targetFigureId = 0;
        target_id = 0;
    }
    if (target_id <= 0) {
        target_id = figure_combat_get_target_for_soldier(f->x, f->y, 20);
        if (target_id) {
            figure *target = figure_get(target_id);
            f->destinationX = target->x;
            f->destinationY = target->y;
            f->targetFigureId = target_id;
            target->targetedByFigureId = f->id;
            f->targetFigureCreatedSequence = target->createdSequence;
        } else {
            f->actionState = FIGURE_ACTION_84_SOLDIER_AT_STANDARD;
            f->graphicOffset = 0;
        }
        figure_route_remove(f);
    }
    return target_id;
}

static void update_image_javelin(figure *f, int dir)
{
    int image_id = image_group(GROUP_BUILDING_FORT_JAVELIN);
    if (f->actionState == FIGURE_ACTION_150_ATTACK) {
        if (f->attackGraphicOffset < 12) {
            f->graphicId = image_id + 96 + dir;
        } else {
            f->graphicId = image_id + 96 + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
        }
    } else if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = image_id + 144 + figure_image_corpse_offset(f);
    } else if (f->actionState == FIGURE_ACTION_84_SOLDIER_AT_STANDARD) {
        f->graphicId = image_id + 96 + dir +
            8 * figure_image_missile_launcher_offset(f);
    } else {
        f->graphicId = image_id + dir + 8 * f->graphicOffset;
    }
}

static void update_image_mounted(figure *f, int dir)
{
    int image_id = image_group(GROUP_FIGURE_FORT_MOUNTED);
    if (f->actionState == FIGURE_ACTION_150_ATTACK) {
        if (f->attackGraphicOffset < 12) {
            f->graphicId = image_id + 96 + dir;
        } else {
            f->graphicId = image_id + 96 + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
        }
    } else if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = image_id + 144 + figure_image_corpse_offset(f);
    } else {
        f->graphicId = image_id + dir + 8 * f->graphicOffset;
    }
}

static void update_image_legionary(figure *f, const formation *m, int dir)
{
    int image_id = image_group(GROUP_BUILDING_FORT_LEGIONARY);
    if (f->actionState == FIGURE_ACTION_150_ATTACK) {
        if (f->attackGraphicOffset < 12) {
            f->graphicId = image_id + 96 + dir;
        } else {
            f->graphicId = image_id + 96 + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
        }
    } else if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = image_id + 152 + figure_image_corpse_offset(f);
    } else if (f->actionState == FIGURE_ACTION_84_SOLDIER_AT_STANDARD) {
        if (m->is_halted && m->layout == FORMATION_COLUMN && m->missile_attack_timeout) {
            f->graphicId = image_id + dir + 144;
        } else {
            f->graphicId = image_id + dir;
        }
    } else {
        f->graphicId = image_id + dir + 8 * f->graphicOffset;
    }
}

static void update_image(figure *f, const formation *m)
{
    int dir;
    if (f->actionState == FIGURE_ACTION_150_ATTACK) {
        dir = f->attackDirection;
    } else if (m->missile_fired) {
        dir = f->direction;
    } else if (f->actionState == FIGURE_ACTION_84_SOLDIER_AT_STANDARD) {
        dir = m->direction;
    } else if (f->direction < 8) {
        dir = f->direction;
    } else {
        dir = f->previousTileDirection;
    }
    dir = figure_image_normalize_direction(dir);
    if (f->type == FIGURE_FORT_JAVELIN) {
        update_image_javelin(f, dir);
    } else if (f->type == FIGURE_FORT_MOUNTED) {
        update_image_mounted(f, dir);
    } else if (f->type == FIGURE_FORT_LEGIONARY) {
        update_image_legionary(f, m, dir);
    }
}

void figure_soldier_action(figure *f)
{
    const formation *m = formation_get(f->formationId);
    Data_CityInfo.numSoldiersInCity++;
    f->terrainUsage = FigureTerrainUsage_Any;
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;
    if (m->in_use != 1) {
        f->actionState = FIGURE_ACTION_149_CORPSE;
    }
    int speed_factor;
    if (f->type == FIGURE_FORT_MOUNTED) {
        speed_factor = 3;
    } else if (f->type == FIGURE_FORT_JAVELIN) {
        speed_factor = 2;
    } else {
        speed_factor = 1;
    }
    int layout = m->layout;
    if (f->formationAtRest || f->actionState == FIGURE_ACTION_81_SOLDIER_GOING_TO_FORT) {
        layout = FORMATION_AT_REST;
    }
    f->formationPositionX = m->x + formation_layout_position_x(layout, f->indexInFormation);
    f->formationPositionY = m->y + formation_layout_position_y(layout, f->indexInFormation);
    
    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_80_SOLDIER_AT_REST:
            map_figure_update(f);
            f->waitTicks = 0;
            f->formationAtRest = 1;
            f->graphicOffset = 0;
            if (f->x != f->formationPositionX || f->y != f->formationPositionY) {
                f->actionState = FIGURE_ACTION_81_SOLDIER_GOING_TO_FORT;
            }
            break;
        case FIGURE_ACTION_81_SOLDIER_GOING_TO_FORT:
        case FIGURE_ACTION_148_FLEEING:
            f->waitTicks = 0;
            f->formationAtRest = 1;
            f->destinationX = f->formationPositionX;
            f->destinationY = f->formationPositionY;
            f->destinationGridOffsetSoldier = map_grid_offset(f->destinationX, f->destinationY);
            FigureMovement_walkTicks(f, speed_factor);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_80_SOLDIER_AT_REST;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            }
            break;
        case FIGURE_ACTION_82_SOLDIER_RETURNING_TO_BARRACKS:
            f->formationAtRest = 1;
            f->destinationX = f->sourceX;
            f->destinationY = f->sourceY;
            FigureMovement_walkTicks(f, speed_factor);
            if (f->direction == DIR_FIGURE_AT_DESTINATION || f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            }
            break;
        case FIGURE_ACTION_83_SOLDIER_GOING_TO_STANDARD:
            f->formationAtRest = 0;
            f->destinationX = m->standard_x + formation_layout_position_x(m->layout, f->indexInFormation);
            f->destinationY = m->standard_y + formation_layout_position_y(m->layout, f->indexInFormation);
            if (f->alternativeLocationIndex) {
                f->destinationX += ALTERNATIVE_POINTS[f->alternativeLocationIndex].x;
                f->destinationY += ALTERNATIVE_POINTS[f->alternativeLocationIndex].y;
            }
            f->destinationGridOffsetSoldier = map_grid_offset(f->destinationX, f->destinationY);
            FigureMovement_walkTicks(f, speed_factor);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_84_SOLDIER_AT_STANDARD;
                f->graphicOffset = 0;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->alternativeLocationIndex++;
                if (f->alternativeLocationIndex > 168) {
                    f->state = FigureState_Dead;
                }
                f->graphicOffset = 0;
            }
            break;
        case FIGURE_ACTION_84_SOLDIER_AT_STANDARD:
            f->formationAtRest = 0;
            f->graphicOffset = 0;
            map_figure_update(f);
            f->destinationX = m->standard_x + formation_layout_position_x(m->layout, f->indexInFormation);
            f->destinationY = m->standard_y + formation_layout_position_y(m->layout, f->indexInFormation);
            if (f->alternativeLocationIndex) {
                f->destinationX += ALTERNATIVE_POINTS[f->alternativeLocationIndex].x;
                f->destinationY += ALTERNATIVE_POINTS[f->alternativeLocationIndex].y;
            }
            if (f->x != f->destinationX || f->y != f->destinationY) {
                if (m->missile_fired <= 0 && m->recent_fight <= 0 && m->missile_attack_timeout <= 0) {
                    f->actionState = FIGURE_ACTION_83_SOLDIER_GOING_TO_STANDARD;
                    f->alternativeLocationIndex = 0;
                }
            }
            if (f->actionState != FIGURE_ACTION_83_SOLDIER_GOING_TO_STANDARD) {
                if (f->type == FIGURE_FORT_JAVELIN) {
                    javelin_launch_missile(f);
                } else if (f->type == FIGURE_FORT_LEGIONARY) {
                    legionary_attack_adjacent_enemy(f);
                }
            }
            break;
        case FIGURE_ACTION_85_SOLDIER_GOING_TO_MILITARY_ACADEMY:
            formation_legion_set_trained(m->id);
            f->formationAtRest = 1;
            FigureMovement_walkTicks(f, speed_factor);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_81_SOLDIER_GOING_TO_FORT;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            }
            break;
        case FIGURE_ACTION_86_SOLDIER_MOPPING_UP:
            f->formationAtRest = 0;
            if (find_mop_up_target(f)) {
                FigureMovement_walkTicks(f, speed_factor);
                if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                    figure *target = figure_get(f->targetFigureId);
                    f->destinationX = target->x;
                    f->destinationY = target->y;
                    figure_route_remove(f);
                } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                    f->actionState = FIGURE_ACTION_84_SOLDIER_AT_STANDARD;
                    f->targetFigureId = 0;
                    f->graphicOffset = 0;
                }
            }
            break;
        case FIGURE_ACTION_87_SOLDIER_GOING_TO_DISTANT_BATTLE:
            f->formationAtRest = 0;
            f->destinationX = Data_CityInfo.exitPointX;
            f->destinationY = Data_CityInfo.exitPointY;
            FigureMovement_walkTicks(f, speed_factor);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_89_SOLDIER_AT_DISTANT_BATTLE;
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            }
            break;
        case FIGURE_ACTION_88_SOLDIER_RETURNING_FROM_DISTANT_BATTLE:
            f->isGhost = 0;
            f->waitTicks = 0;
            f->formationAtRest = 1;
            f->destinationX = f->formationPositionX;
            f->destinationY = f->formationPositionY;
            f->destinationGridOffsetSoldier = map_grid_offset(f->destinationX, f->destinationY);
            FigureMovement_walkTicks(f, speed_factor);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_80_SOLDIER_AT_REST;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            }
            break;
        case FIGURE_ACTION_89_SOLDIER_AT_DISTANT_BATTLE:
            f->isGhost = 1;
            f->formationAtRest = 1;
            break;
    }

    update_image(f, m);
}
