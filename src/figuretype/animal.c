#include "animal.h"

#include "building/building.h"
#include "core/calc.h"
#include "core/random.h"
#include "figure/formation.h"
#include "figure/formation_layout.h"
#include "figure/image.h"
#include "figure/route.h"
#include "graphics/image.h"
#include "map/figure.h"
#include "map/grid.h"
#include "scenario/map.h"
#include "scenario/property.h"

#include "Data/CityInfo.h"
#include "Data/State.h"
#include "FigureAction.h"
#include "FigureMovement.h"

static const int SEAGULL_OFFSETS_X[] = {0, 0, -2, 1, 2, -3, 4, -2, 0};
static const int SEAGULL_OFFSETS_Y[] = {0, -2, 0, 2, 0, 1, -3, 4, 0};

static const int HORSE_DESTINATION_X1[] = {
    2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2
};
static const int HORSE_DESTINATION_Y1[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2
};
static const int HORSE_DESTINATION_X2[] = {
    12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};
static const int HORSE_DESTINATION_Y2[] = {
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2
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
    FigureMovement_crossCountrySetDirection(fish,
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
            f->actionState = FigureActionState_196_HerdAnimalAtRest;
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
    if (!(f->graphicOffset & 3) && FigureMovement_crossCountryWalkTicks(f, 1)) {
        f->progressOnTile++;
        if (f->progressOnTile > 8) {
            f->progressOnTile = 0;
        }
        FigureAction_Common_setCrossCountryDestination(f,
            f->sourceX + SEAGULL_OFFSETS_X[f->progressOnTile],
            f->sourceY + SEAGULL_OFFSETS_Y[f->progressOnTile]);
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
        case FigureActionState_150_Attack:
            FigureAction_Common_handleAttack(f);
            break;
        case FigureActionState_149_Corpse:
            FigureAction_Common_handleCorpse(f);
            break;
        case FigureActionState_196_HerdAnimalAtRest:
            f->waitTicks++;
            if (f->waitTicks > 400) {
                f->waitTicks = f->id & 0x1f;
                f->actionState = FigureActionState_197_HerdAnimalMoving;
                f->destinationX = m->destination_x + formation_layout_position_x(FORMATION_HERD, f->indexInFormation);
                f->destinationY = m->destination_y + formation_layout_position_y(FORMATION_HERD, f->indexInFormation);
                f->roamLength = 0;
            }
            break;
        case FigureActionState_197_HerdAnimalMoving:
            FigureMovement_walkTicks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION || f->direction == DIR_FIGURE_LOST) {
                f->direction = f->previousTileDirection;
                f->actionState = FigureActionState_196_HerdAnimalAtRest;
                f->waitTicks = f->id & 0x1f;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            }
            break;
    }
    int dir = figure_image_direction(f);
    if (f->actionState == FigureActionState_149_Corpse) {
        f->graphicId = image_group(GROUP_FIGURE_SHEEP) + 104 +
            figure_image_corpse_offset(f);
    } else if (f->actionState == FigureActionState_196_HerdAnimalAtRest) {
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
        case FigureActionState_150_Attack:
            FigureAction_Common_handleAttack(f);
            break;
        case FigureActionState_149_Corpse:
            FigureAction_Common_handleCorpse(f);
            break;
        case FigureActionState_196_HerdAnimalAtRest:
            f->waitTicks++;
            if (f->waitTicks > 400) {
                f->waitTicks = f->id & 0x1f;
                f->actionState = FigureActionState_197_HerdAnimalMoving;
                f->destinationX = m->destination_x + formation_layout_position_x(FORMATION_HERD, f->indexInFormation);
                f->destinationY = m->destination_y + formation_layout_position_y(FORMATION_HERD, f->indexInFormation);
                f->roamLength = 0;
            }
            break;
        case FigureActionState_197_HerdAnimalMoving:
            FigureMovement_walkTicks(f, 2);
            if (f->direction == DIR_FIGURE_AT_DESTINATION || f->direction == DIR_FIGURE_LOST) {
                f->direction = f->previousTileDirection;
                f->actionState = FigureActionState_196_HerdAnimalAtRest;
                f->waitTicks = f->id & 0x1f;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            }
            break;
        case FigureActionState_199_WolfAttacking:
            FigureMovement_walkTicks(f, 2);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                int targetId = FigureAction_CombatWolf_getTarget(f->x, f->y, 6);
                if (targetId) {
                    figure *target = figure_get(targetId);
                    f->destinationX = target->x;
                    f->destinationY = target->y;
                    f->targetFigureId = targetId;
                    target->targetedByFigureId = f->id;
                    f->targetFigureCreatedSequence = target->createdSequence;
                    figure_route_remove(f);
                } else {
                    f->direction = f->previousTileDirection;
                    f->actionState = FigureActionState_196_HerdAnimalAtRest;
                    f->waitTicks = f->id & 0x1f;
                }
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->direction = f->previousTileDirection;
                f->actionState = FigureActionState_196_HerdAnimalAtRest;
                f->waitTicks = f->id & 0x1f;
            }
            break;
    }
    int dir = figure_image_direction(f);
    if (f->actionState == FigureActionState_149_Corpse) {
        f->graphicId = image_group(GROUP_FIGURE_WOLF) + 96 + figure_image_corpse_offset(f);
    } else if (f->actionState == FigureActionState_150_Attack) {
        f->graphicId = image_group(GROUP_FIGURE_WOLF) + 104 +
            dir + 8 * (f->attackGraphicOffset / 4);
    } else if (f->actionState == FigureActionState_196_HerdAnimalAtRest) {
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
        case FigureActionState_150_Attack:
            FigureAction_Common_handleAttack(f);
            break;
        case FigureActionState_149_Corpse:
            FigureAction_Common_handleCorpse(f);
            break;
        case FigureActionState_196_HerdAnimalAtRest:
            f->waitTicks++;
            if (f->waitTicks > 200) {
                f->waitTicks = f->id & 0x1f;
                f->actionState = FigureActionState_197_HerdAnimalMoving;
                f->destinationX = m->destination_x + formation_layout_position_x(FORMATION_HERD, f->indexInFormation);
                f->destinationY = m->destination_y + formation_layout_position_y(FORMATION_HERD, f->indexInFormation);
                f->roamLength = 0;
            }
            break;
        case FigureActionState_197_HerdAnimalMoving:
            FigureMovement_walkTicks(f, 2);
            if (f->direction == DIR_FIGURE_AT_DESTINATION || f->direction == DIR_FIGURE_LOST) {
                f->direction = f->previousTileDirection;
                f->actionState = FigureActionState_196_HerdAnimalAtRest;
                f->waitTicks = f->id & 0x1f;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            }
            break;
    }
    int dir = figure_image_direction(f);
    if (f->actionState == FigureActionState_149_Corpse) {
        f->graphicId = image_group(GROUP_FIGURE_ZEBRA) + 96 + figure_image_corpse_offset(f);
    } else if (f->actionState == FigureActionState_196_HerdAnimalAtRest) {
        f->graphicId = image_group(GROUP_FIGURE_ZEBRA) + dir;
    } else {
        f->graphicId = image_group(GROUP_FIGURE_ZEBRA) + dir + 8 * f->graphicOffset;
    }
}

static void set_horse_destination(figure *f, int state)
{
    building *b = building_get(f->buildingId);
    if (state == HORSE_CREATED) {
        map_figure_delete(f);
        if (Data_State.map.orientation == DIR_0_TOP || Data_State.map.orientation == DIR_6_LEFT) {
            f->destinationX = b->x + HORSE_DESTINATION_X1[f->waitTicksMissile];
            f->destinationY = b->y + HORSE_DESTINATION_Y1[f->waitTicksMissile];
        } else {
            f->destinationX = b->x + HORSE_DESTINATION_X2[f->waitTicksMissile];
            f->destinationY = b->y + HORSE_DESTINATION_Y2[f->waitTicksMissile];
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
        if (Data_State.map.orientation == DIR_0_TOP || Data_State.map.orientation == DIR_6_LEFT) {
            f->destinationX = b->x + HORSE_DESTINATION_X1[f->waitTicksMissile];
            f->destinationY = b->y + HORSE_DESTINATION_Y1[f->waitTicksMissile];
        } else {
            f->destinationX = b->x + HORSE_DESTINATION_X2[f->waitTicksMissile];
            f->destinationY = b->y + HORSE_DESTINATION_Y2[f->waitTicksMissile];
        }
    } else if (state == HORSE_FINISHED) {
        if (Data_State.map.orientation == DIR_0_TOP || Data_State.map.orientation == DIR_6_LEFT) {
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
        case FigureActionState_200_HippodromeMiniHorseCreated:
            f->graphicOffset = 0;
            f->waitTicksMissile = 0;
            set_horse_destination(f, HORSE_CREATED);
            f->waitTicks++;
            if (f->waitTicks > 60 && f->resourceId == 0) {
                f->actionState = FigureActionState_201_HippodromeMiniHorseRacing;
                f->waitTicks = 0;
            }
            f->waitTicks++;
            if (f->waitTicks > 20 && f->resourceId == 1) {
                f->actionState = FigureActionState_201_HippodromeMiniHorseRacing;
                f->waitTicks = 0;
            }
            break;
        case FigureActionState_201_HippodromeMiniHorseRacing:
            f->direction = calc_general_direction(f->x, f->y, f->destinationX, f->destinationY);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->waitTicksMissile++;
                if (f->waitTicksMissile >= 22) {
                    f->waitTicksMissile = 0;
                    f->inFrontFigureId++;
                    if (f->inFrontFigureId >= 6) {
                        f->waitTicks = 0;
                        f->actionState = FigureActionState_202_HippodromeMiniHorseDone;
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
                FigureMovement_crossCountrySetDirection(f,
                    f->crossCountryX, f->crossCountryY, 15 * f->destinationX, 15 * f->destinationY, 0);
            }
            if (f->actionState != FigureActionState_202_HippodromeMiniHorseDone) {
                FigureMovement_crossCountryWalkTicks(f, f->speedMultiplier);
            }
            break;
        case FigureActionState_202_HippodromeMiniHorseDone:
            if (!f->waitTicks) {
                set_horse_destination(f, HORSE_FINISHED);
                f->direction = calc_general_direction(f->x, f->y, f->destinationX, f->destinationY);
                FigureMovement_crossCountrySetDirection(f,
                    f->crossCountryX, f->crossCountryY, 15 * f->destinationX, 15 * f->destinationY, 0);
            }
            if (f->direction != DIR_FIGURE_AT_DESTINATION) {
                FigureMovement_crossCountryWalkTicks(f, 1);
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
