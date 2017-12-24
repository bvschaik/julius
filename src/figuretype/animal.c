#include "animal.h"

#include "core/random.h"
#include "figure/formation.h"
#include "scenario/map.h"
#include "scenario/property.h"

#include "FigureMovement.h"

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
