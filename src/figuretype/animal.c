#include "animal.h"

#include "building/building.h"
#include "city/view.h"
#include "core/calc.h"
#include "core/image.h"
#include "core/random.h"
#include "figure/combat.h"
#include "figure/formation.h"
#include "figure/formation_layout.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/point.h"
#include "scenario/map.h"
#include "scenario/property.h"

#include "Data/CityInfo.h"

static const map_point SEAGULL_OFFSETS[] = {
    {0, 0}, {0, -2}, {-2, 0}, {1, 2}, {2, 0}, {-3, 1}, {4, -3}, {-2, 4}, {0, 0}
};

static const map_point HORSE_DESTINATION_1[] = {
    {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}, {7, 1}, {8, 1}, {9, 1}, {10, 1}, {11, 1}, {12, 2},
    {12, 3}, {11, 3}, {10, 3}, {9, 3}, {8, 3}, {7, 3}, {6, 3}, {5, 3}, {4, 3}, {3, 3}, {2, 2}
};
static const map_point HORSE_DESTINATION_2[] = {
    {12, 3}, {11, 3}, {10, 3}, {9, 3}, {8, 3}, {7, 3}, {6, 3}, {5, 3}, {4, 3}, {3, 3}, {2, 2},
    {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}, {7, 1}, {8, 1}, {9, 1}, {10, 1}, {11, 1}, {12, 2}
};

static const int SHEEP_IMAGE_OFFSETS[] = {
    0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 4, 4, 5, 5, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 5, 5
};

enum {
    HORSE_CREATED = 0,
    HORSE_RACING = 1,
    HORSE_FINISHED = 2
};

static void create_fishing_point(int x, int y)
{
    random_generate_next();
    figure *fish = figure_create(FIGURE_FISH_GULLS, x, y, DIR_0_TOP);
    fish->graphicOffset = random_byte() & 0x1f;
    fish->progressOnTile = random_byte() & 7;
    figure_movement_set_cross_country_direction(fish,
        fish->crossCountryX, fish->crossCountryY,
        15 * fish->destinationX, 15 * fish->destinationY, 0);
}

void figure_create_fishing_points()
{
    scenario_map_foreach_fishing_point(create_fishing_point);
}

static void create_herd(int x, int y)
{
    figure_type herd_type;
    int num_animals;
    switch (scenario_property_climate()) {
        case CLIMATE_CENTRAL:
            herd_type = FIGURE_SHEEP;
            num_animals = 10;
            break;
        case CLIMATE_NORTHERN:
            herd_type = FIGURE_WOLF;
            num_animals = 8;
            break;
        case CLIMATE_DESERT:
            herd_type = FIGURE_ZEBRA;
            num_animals = 12;
            break;
        default:
            return;
    }
    int formation_id = formation_create_herd(herd_type, x, y, num_animals);
    if (formation_id > 0) {
        for (int fig = 0; fig < num_animals; fig++) {
            random_generate_next();
            figure *f = figure_create(herd_type, x, y, DIR_0_TOP);
            f->actionState = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
            f->formationId = formation_id;
            f->waitTicks = f->id & 0x1f;
        }
    }
}

void figure_create_herds()
{
    scenario_map_foreach_herd_point(create_herd);
}

void figure_seagulls_action(figure *f)
{
    f->terrainUsage = FigureTerrainUsage_Any;
    f->isGhost = 0;
    f->useCrossCountry = 1;
    if (!(f->graphicOffset & 3) && figure_movement_move_ticks_cross_country(f, 1)) {
        f->progressOnTile++;
        if (f->progressOnTile > 8) {
            f->progressOnTile = 0;
        }
        figure_movement_set_cross_country_destination(f,
            f->sourceX + SEAGULL_OFFSETS[f->progressOnTile].x,
            f->sourceY + SEAGULL_OFFSETS[f->progressOnTile].y);
    }
    if (f->id & 1) {
        figure_image_increase_offset(f, 54);
        f->graphicId = image_group(GROUP_FIGURE_SEAGULLS) + f->graphicOffset / 3;
    } else {
        figure_image_increase_offset(f, 72);
        f->graphicId = image_group(GROUP_FIGURE_SEAGULLS) + 18 + f->graphicOffset / 3;
    }
}

void figure_sheep_action(figure *f)
{
    const formation *m = formation_get(f->formationId);
    f->terrainUsage = FigureTerrainUsage_Animal;
    f->useCrossCountry = 0;
    f->isGhost = 0;
    Data_CityInfo.numAnimalsInCity++;
    figure_image_increase_offset(f, 6);

    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_196_HERD_ANIMAL_AT_REST:
            f->waitTicks++;
            if (f->waitTicks > 400) {
                f->waitTicks = f->id & 0x1f;
                f->actionState = FIGURE_ACTION_197_HERD_ANIMAL_MOVING;
                f->destinationX = m->destination_x + formation_layout_position_x(FORMATION_HERD, f->indexInFormation);
                f->destinationY = m->destination_y + formation_layout_position_y(FORMATION_HERD, f->indexInFormation);
                f->roamLength = 0;
            }
            break;
        case FIGURE_ACTION_197_HERD_ANIMAL_MOVING:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION || f->direction == DIR_FIGURE_LOST) {
                f->direction = f->previousTileDirection;
                f->actionState = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
                f->waitTicks = f->id & 0x1f;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            }
            break;
    }
    int dir = figure_image_direction(f);
    if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = image_group(GROUP_FIGURE_SHEEP) + 104 +
            figure_image_corpse_offset(f);
    } else if (f->actionState == FIGURE_ACTION_196_HERD_ANIMAL_AT_REST) {
        if (f->id & 3) {
            f->graphicId = image_group(GROUP_FIGURE_SHEEP) + 48 + dir +
                8 * SHEEP_IMAGE_OFFSETS[f->waitTicks & 0x3f];
        } else {
            f->graphicId = image_group(GROUP_FIGURE_SHEEP) + 96 + dir;
        }
    } else {
        f->graphicId = image_group(GROUP_FIGURE_SHEEP) + dir + 8 * f->graphicOffset;
    }
}

void figure_wolf_action(figure *f)
{
    const formation *m = formation_get(f->formationId);
    f->terrainUsage = FigureTerrainUsage_Animal;
    f->useCrossCountry = 0;
    f->isGhost = 0;
    Data_CityInfo.numAnimalsInCity++;
    figure_image_increase_offset(f, 12);

    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_196_HERD_ANIMAL_AT_REST:
            f->waitTicks++;
            if (f->waitTicks > 400) {
                f->waitTicks = f->id & 0x1f;
                f->actionState = FIGURE_ACTION_197_HERD_ANIMAL_MOVING;
                f->destinationX = m->destination_x + formation_layout_position_x(FORMATION_HERD, f->indexInFormation);
                f->destinationY = m->destination_y + formation_layout_position_y(FORMATION_HERD, f->indexInFormation);
                f->roamLength = 0;
            }
            break;
        case FIGURE_ACTION_197_HERD_ANIMAL_MOVING:
            figure_movement_move_ticks(f, 2);
            if (f->direction == DIR_FIGURE_AT_DESTINATION || f->direction == DIR_FIGURE_LOST) {
                f->direction = f->previousTileDirection;
                f->actionState = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
                f->waitTicks = f->id & 0x1f;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            }
            break;
        case FIGURE_ACTION_199_WOLF_ATTACKING:
            figure_movement_move_ticks(f, 2);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                int target_id = figure_combat_get_target_for_wolf(f->x, f->y, 6);
                if (target_id) {
                    figure *target = figure_get(target_id);
                    f->destinationX = target->x;
                    f->destinationY = target->y;
                    f->targetFigureId = target_id;
                    target->targetedByFigureId = f->id;
                    f->targetFigureCreatedSequence = target->createdSequence;
                    figure_route_remove(f);
                } else {
                    f->direction = f->previousTileDirection;
                    f->actionState = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
                    f->waitTicks = f->id & 0x1f;
                }
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->direction = f->previousTileDirection;
                f->actionState = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
                f->waitTicks = f->id & 0x1f;
            }
            break;
    }
    int dir = figure_image_direction(f);
    if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = image_group(GROUP_FIGURE_WOLF) + 96 + figure_image_corpse_offset(f);
    } else if (f->actionState == FIGURE_ACTION_150_ATTACK) {
        f->graphicId = image_group(GROUP_FIGURE_WOLF) + 104 +
            dir + 8 * (f->attackGraphicOffset / 4);
    } else if (f->actionState == FIGURE_ACTION_196_HERD_ANIMAL_AT_REST) {
        f->graphicId = image_group(GROUP_FIGURE_WOLF) + 152 + dir;
    } else {
        f->graphicId = image_group(GROUP_FIGURE_WOLF) + dir + 8 * f->graphicOffset;
    }
}

void figure_zebra_action(figure *f)
{
    const formation *m = formation_get(f->formationId);
    f->terrainUsage = FigureTerrainUsage_Animal;
    f->useCrossCountry = 0;
    f->isGhost = 0;
    Data_CityInfo.numAnimalsInCity++;
    figure_image_increase_offset(f, 12);

    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_196_HERD_ANIMAL_AT_REST:
            f->waitTicks++;
            if (f->waitTicks > 200) {
                f->waitTicks = f->id & 0x1f;
                f->actionState = FIGURE_ACTION_197_HERD_ANIMAL_MOVING;
                f->destinationX = m->destination_x + formation_layout_position_x(FORMATION_HERD, f->indexInFormation);
                f->destinationY = m->destination_y + formation_layout_position_y(FORMATION_HERD, f->indexInFormation);
                f->roamLength = 0;
            }
            break;
        case FIGURE_ACTION_197_HERD_ANIMAL_MOVING:
            figure_movement_move_ticks(f, 2);
            if (f->direction == DIR_FIGURE_AT_DESTINATION || f->direction == DIR_FIGURE_LOST) {
                f->direction = f->previousTileDirection;
                f->actionState = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
                f->waitTicks = f->id & 0x1f;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            }
            break;
    }
    int dir = figure_image_direction(f);
    if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = image_group(GROUP_FIGURE_ZEBRA) + 96 + figure_image_corpse_offset(f);
    } else if (f->actionState == FIGURE_ACTION_196_HERD_ANIMAL_AT_REST) {
        f->graphicId = image_group(GROUP_FIGURE_ZEBRA) + dir;
    } else {
        f->graphicId = image_group(GROUP_FIGURE_ZEBRA) + dir + 8 * f->graphicOffset;
    }
}

static void set_horse_destination(figure *f, int state)
{
    building *b = building_get(f->buildingId);
    int orientation = city_view_orientation();
    if (state == HORSE_CREATED) {
        map_figure_delete(f);
        if (orientation == DIR_0_TOP || orientation == DIR_6_LEFT) {
            f->destinationX = b->x + HORSE_DESTINATION_1[f->waitTicksMissile].x;
            f->destinationY = b->y + HORSE_DESTINATION_1[f->waitTicksMissile].y;
        } else {
            f->destinationX = b->x + HORSE_DESTINATION_2[f->waitTicksMissile].x;
            f->destinationY = b->y + HORSE_DESTINATION_2[f->waitTicksMissile].y;
        }
        if (f->resourceId == 1) {
            f->destinationY++;
        }
        f->x = f->destinationX;
        f->y = f->destinationY;
        f->crossCountryX = 15 * f->x;
        f->crossCountryY = 15 * f->y;
        f->gridOffset = map_grid_offset(f->x, f->y);
        map_figure_add(f);
    } else if (state == HORSE_RACING) {
        if (orientation == DIR_0_TOP || orientation == DIR_6_LEFT) {
            f->destinationX = b->x + HORSE_DESTINATION_1[f->waitTicksMissile].x;
            f->destinationY = b->y + HORSE_DESTINATION_1[f->waitTicksMissile].y;
        } else {
            f->destinationX = b->x + HORSE_DESTINATION_2[f->waitTicksMissile].x;
            f->destinationY = b->y + HORSE_DESTINATION_2[f->waitTicksMissile].y;
        }
    } else if (state == HORSE_FINISHED) {
        if (orientation == DIR_0_TOP || orientation == DIR_6_LEFT) {
            if (f->resourceId) {
                f->destinationX = b->x + 1;
                f->destinationY = b->y + 2;
            } else {
                f->destinationX = b->x + 1;
                f->destinationY = b->y + 1;
            }
        } else {
            if (f->resourceId) {
                f->destinationX = b->x + 12;
                f->destinationY = b->y + 3;
            } else {
                f->destinationX = b->x + 12;
                f->destinationY = b->y + 2;
            }
        }
    }
}

void figure_hippodrome_horse_action(figure *f)
{
    Data_CityInfo.entertainmentHippodromeHasShow = 1;
    f->useCrossCountry = 1;
    f->isGhost = 0;
    figure_image_increase_offset(f, 8);

    switch (f->actionState) {
        case FIGURE_ACTION_200_HIPPODROME_HORSE_CREATED:
            f->graphicOffset = 0;
            f->waitTicksMissile = 0;
            set_horse_destination(f, HORSE_CREATED);
            f->waitTicks++;
            if (f->waitTicks > 60 && f->resourceId == 0) {
                f->actionState = FIGURE_ACTION_201_HIPPODROME_HORSE_RACING;
                f->waitTicks = 0;
            }
            f->waitTicks++;
            if (f->waitTicks > 20 && f->resourceId == 1) {
                f->actionState = FIGURE_ACTION_201_HIPPODROME_HORSE_RACING;
                f->waitTicks = 0;
            }
            break;
        case FIGURE_ACTION_201_HIPPODROME_HORSE_RACING:
            f->direction = calc_general_direction(f->x, f->y, f->destinationX, f->destinationY);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->waitTicksMissile++;
                if (f->waitTicksMissile >= 22) {
                    f->waitTicksMissile = 0;
                    f->inFrontFigureId++;
                    if (f->inFrontFigureId >= 6) {
                        f->waitTicks = 0;
                        f->actionState = FIGURE_ACTION_202_HIPPODROME_HORSE_DONE;
                    }
                    if ((f->id + random_byte()) & 1) {
                        f->speedMultiplier = 3;
                    } else {
                        f->speedMultiplier = 4;
                    }
                } else if (f->waitTicksMissile == 11) {
                    if ((f->id + random_byte()) & 1) {
                        f->speedMultiplier = 3;
                    } else {
                        f->speedMultiplier = 4;
                    }
                }
                set_horse_destination(f, HORSE_RACING);
                f->direction = calc_general_direction(f->x, f->y, f->destinationX, f->destinationY);
                figure_movement_set_cross_country_direction(f,
                    f->crossCountryX, f->crossCountryY, 15 * f->destinationX, 15 * f->destinationY, 0);
            }
            if (f->actionState != FIGURE_ACTION_202_HIPPODROME_HORSE_DONE) {
                figure_movement_move_ticks_cross_country(f, f->speedMultiplier);
            }
            break;
        case FIGURE_ACTION_202_HIPPODROME_HORSE_DONE:
            if (!f->waitTicks) {
                set_horse_destination(f, HORSE_FINISHED);
                f->direction = calc_general_direction(f->x, f->y, f->destinationX, f->destinationY);
                figure_movement_set_cross_country_direction(f,
                    f->crossCountryX, f->crossCountryY, 15 * f->destinationX, 15 * f->destinationY, 0);
            }
            if (f->direction != DIR_FIGURE_AT_DESTINATION) {
                figure_movement_move_ticks_cross_country(f, 1);
            }
            f->waitTicks++;
            if (f->waitTicks > 30) {
                f->graphicOffset = 0;
            }
            f->waitTicks++;
            if (f->waitTicks > 150) {
                f->state = FigureState_Dead;
            }
            break;
    }

    int dir = figure_image_direction(f);
    if (f->resourceId == 0) {
        f->graphicId = image_group(GROUP_FIGURE_HIPPODROME_HORSE_1) +
            dir + 8 * f->graphicOffset;
        f->cartGraphicId = image_group(GROUP_FIGURE_HIPPODROME_CART_1) + dir;
    } else {
        f->graphicId = image_group(GROUP_FIGURE_HIPPODROME_HORSE_2) +
            dir + 8 * f->graphicOffset;
        f->cartGraphicId = image_group(GROUP_FIGURE_HIPPODROME_CART_2) + dir;
    }
    int cart_dir = (dir + 4) % 8;
    figure_image_set_cart_offset(f, cart_dir);
}

void figure_hippodrome_horse_reroute()
{
    if (!Data_CityInfo.entertainmentHippodromeHasShow) {
        return;
    }
    for (int i = 1; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
        if (f->state == FigureState_Alive && f->type == FIGURE_HIPPODROME_HORSES) {
            f->waitTicksMissile = 0;
            set_horse_destination(f, HORSE_CREATED);
        }
    }
}
