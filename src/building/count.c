#include "count.h"

#include "building/building.h"
#include "building/monument.h"
#include "core/array.h"
#include "city/buildings.h"
#include "city/health.h"
#include "figure/figure.h"
#include "map/building.h"
#include "map/grid.h"

static const building_type building_set_farms[] = {
    BUILDING_WHEAT_FARM, BUILDING_VEGETABLE_FARM, BUILDING_FRUIT_FARM, BUILDING_OLIVE_FARM,
    BUILDING_VINES_FARM, BUILDING_PIG_FARM
};

#define BUILDING_SET_SIZE_FARMS (sizeof(building_set_farms) / sizeof(building_type))

static const building_type building_set_raw_materials[] = {
    BUILDING_MARBLE_QUARRY, BUILDING_IRON_MINE, BUILDING_TIMBER_YARD, BUILDING_CLAY_PIT,
    BUILDING_GOLD_MINE, BUILDING_STONE_QUARRY, BUILDING_SAND_PIT
};

#define BUILDING_SET_SIZE_RAW_MATERIALS (sizeof(building_set_raw_materials) / sizeof(building_type))

static const building_type building_set_workshops[] = {
    BUILDING_WINE_WORKSHOP, BUILDING_OIL_WORKSHOP, BUILDING_WEAPONS_WORKSHOP, BUILDING_FURNITURE_WORKSHOP,
    BUILDING_POTTERY_WORKSHOP, BUILDING_CONCRETE_MAKER, BUILDING_BRICKWORKS, BUILDING_CITY_MINT
};

#define BUILDING_SET_SIZE_WORKSHOPS (sizeof(building_set_workshops) / sizeof(building_type))

static const building_type building_set_small_temples[] = {
    BUILDING_SMALL_TEMPLE_CERES, BUILDING_SMALL_TEMPLE_NEPTUNE, BUILDING_SMALL_TEMPLE_MERCURY, BUILDING_SMALL_TEMPLE_MARS,
    BUILDING_SMALL_TEMPLE_VENUS
};

#define BUILDING_SET_SIZE_SMALL_TEMPLES (sizeof(building_set_small_temples) / sizeof(building_type))

static const building_type building_set_large_temples[] = {
    BUILDING_LARGE_TEMPLE_CERES, BUILDING_LARGE_TEMPLE_NEPTUNE, BUILDING_LARGE_TEMPLE_MERCURY, BUILDING_LARGE_TEMPLE_MARS,
    BUILDING_LARGE_TEMPLE_VENUS
};

#define BUILDING_SET_SIZE_LARGE_TEMPLES (sizeof(building_set_large_temples) / sizeof(building_type))

static const building_type building_set_grand_temples[] = {
    BUILDING_GRAND_TEMPLE_CERES, BUILDING_GRAND_TEMPLE_NEPTUNE, BUILDING_GRAND_TEMPLE_MERCURY, BUILDING_GRAND_TEMPLE_MARS,
    BUILDING_GRAND_TEMPLE_VENUS, BUILDING_PANTHEON
};

#define BUILDING_SET_SIZE_GRAND_TEMPLES (sizeof(building_set_grand_temples) / sizeof(building_type))

static const building_type building_set_deco_trees[] = {
    BUILDING_PINE_TREE, BUILDING_FIR_TREE, BUILDING_OAK_TREE, BUILDING_ELM_TREE,
    BUILDING_FIG_TREE, BUILDING_PLUM_TREE, BUILDING_PALM_TREE, BUILDING_DATE_TREE
};

#define BUILDING_SET_SIZE_DECO_TREES (sizeof(building_set_deco_trees) / sizeof(building_type))

static const building_type building_set_deco_paths[] = {
    BUILDING_PINE_PATH, BUILDING_FIR_PATH, BUILDING_OAK_PATH, BUILDING_ELM_PATH,
    BUILDING_FIG_PATH, BUILDING_PLUM_PATH, BUILDING_PALM_PATH, BUILDING_DATE_PATH,
    BUILDING_GARDEN_PATH
};

#define BUILDING_SET_SIZE_DECO_PATHS (sizeof(building_set_deco_paths) / sizeof(building_type))

static const building_type building_set_deco_statues[] = {
    BUILDING_GARDENS, BUILDING_GRAND_GARDEN, BUILDING_SMALL_STATUE, BUILDING_MEDIUM_STATUE,
    BUILDING_LARGE_STATUE, BUILDING_GODDESS_STATUE, BUILDING_SENATOR_STATUE, BUILDING_LEGION_STATUE,
    BUILDING_GLADIATOR_STATUE, BUILDING_SMALL_POND, BUILDING_LARGE_POND, BUILDING_DOLPHIN_FOUNTAIN
};

#define BUILDING_SET_SIZE_DECO_STATUES (sizeof(building_set_deco_statues) / sizeof(building_type))

int building_count_grand_temples(void)
{
    return building_count_total(BUILDING_GRAND_TEMPLE_CERES) +
        building_count_total(BUILDING_GRAND_TEMPLE_NEPTUNE) +
        building_count_total(BUILDING_GRAND_TEMPLE_MERCURY) +
        building_count_total(BUILDING_GRAND_TEMPLE_MARS) +
        building_count_total(BUILDING_GRAND_TEMPLE_VENUS) +
        building_count_total(BUILDING_PANTHEON);
}

int building_count_grand_temples_active(void)
{
    return building_count_active(BUILDING_GRAND_TEMPLE_CERES) +
        building_count_active(BUILDING_GRAND_TEMPLE_NEPTUNE) +
        building_count_active(BUILDING_GRAND_TEMPLE_MERCURY) +
        building_count_active(BUILDING_GRAND_TEMPLE_MARS) +
        building_count_active(BUILDING_GRAND_TEMPLE_VENUS) +
        building_count_active(BUILDING_PANTHEON);
}

int building_count_active(building_type type)
{
    int active = 0;
    for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
        if (building_is_active(b) && b == building_main(b)) {
            active++;
        }
    }
    return active;
}

int building_count_total(building_type type)
{
    int total = 0;
    for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
        if ((b->state == BUILDING_STATE_IN_USE || b->state == BUILDING_STATE_CREATED) && b == building_main(b)) {
            total++;
        }
    }
    return total;
}

int building_count_any_total(int active_only)
{
    int total = 0;
    for (int id = 1; id < building_count(); id++) {
        building *b = building_get(id);
        if (b == building_main(b)) {
            if (active_only) {
                if (building_is_active(b)) {
                    total++;
                }
            } else {
                if (b->state == BUILDING_STATE_IN_USE || b->state == BUILDING_STATE_CREATED) {
                    total++;
                }
            }
        }
    }
    return total;
}

int building_count_upgraded(building_type type)
{
    int upgraded = 0;
    for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
        if ((b->state == BUILDING_STATE_IN_USE || b->state == BUILDING_STATE_CREATED) && b->upgrade_level > 0 && b == building_main(b)) {
            upgraded++;
        }
    }
    return upgraded;
}

int building_count_in_area(building_type type, int minx, int miny, int maxx, int maxy)
{
    int grid_area = (abs(maxx - minx) + 1) * (abs(maxy - miny) + 1);
    int array_size = grid_area < building_count() ? grid_area : building_count();
    unsigned int *found_buildings = (unsigned int *) malloc(array_size * sizeof(unsigned int));
    memset(found_buildings, 0, array_size * sizeof(unsigned int));

    int total = 0;
    for (int x = minx; x <= maxx; x++) {
        for (int y = miny; y <= maxy; y++) {
            int grid_offset = map_grid_offset(x, y);
            int building_id = map_building_at(grid_offset);
            if (building_id) {
                building *b = building_main(building_get(building_id));
                if (type != BUILDING_ANY && b->type != type) {
                    continue;
                }
                if (b->state != BUILDING_STATE_IN_USE && b->state != BUILDING_STATE_CREATED) {
                    continue;
                }

                int building_already_counted = 0;

                for (int i = 0; i < total; i++) {
                    if (found_buildings[i] == b->id) {
                        building_already_counted = 1;
                        break;
                    }
                }

                if (building_already_counted) {
                    continue;
                }

                found_buildings[total] = b->id;
                total++;
            }
        }
    }

    free(found_buildings);
    return total;
}

int building_count_fort_type_in_area(int minx, int miny, int maxx, int maxy, figure_type type)
{
    int grid_area = abs((maxx - minx) * (maxy - miny));
    int array_size = grid_area < building_count() ? grid_area : building_count();
    int *found_buildings = (int *) malloc(array_size * sizeof(int));

    int total = 0;
    for (int x = minx; x <= maxx; x++) {
        for (int y = miny; y <= maxy; y++) {
            int grid_offset = map_grid_offset(x, y);
            int building_id = map_building_at(grid_offset);
            if (building_id) {
                building *b = building_main(building_get(building_id));
                if (b->type != BUILDING_FORT || b->subtype.fort_figure_type != type) {
                    continue;
                }
                if (b->state != BUILDING_STATE_IN_USE && b->state != BUILDING_STATE_CREATED) {
                    continue;
                }

                int building_already_counted = 0;

                for (int i = 0; i < total; i++) {
                    if (found_buildings[i] == b->id) {
                        building_already_counted = 1;
                        break;
                    }
                }

                if (building_already_counted) {
                    continue;
                }

                found_buildings[total] = b->id;
                total++;
            }
        }
    }

    free(found_buildings);
    return total;
}

static int building_count_with_active_check(building_type type, int active_only)
{
    if (active_only) {
        return building_count_active(type);
    } else {
        return building_count_total(type);
    }
}

static int count_all_types_in_set(int active_only, const building_type *set, int count)
{
    int total = 0;
    for (int i = 0; i < count; i++) {
        building_type type = set[i];
        total += building_count_with_active_check(type, active_only);
    }
    return total;
}

static int count_all_types_in_set_in_area(const building_type *set, int count, int minx, int miny, int maxx, int maxy)
{
    int total = 0;
    for (int i = 0; i < count; i++) {
        building_type type = set[i];
        total += building_count_in_area(type, minx, miny, maxx, maxy);
    }
    return total;
}

int building_set_count_farms(int active_only)
{
    return count_all_types_in_set(active_only, building_set_farms, BUILDING_SET_SIZE_FARMS);
}

int building_set_count_raw_materials(int active_only)
{
    return count_all_types_in_set(active_only, building_set_raw_materials, BUILDING_SET_SIZE_RAW_MATERIALS);
}

int building_set_count_workshops(int active_only)
{
    return count_all_types_in_set(active_only, building_set_workshops, BUILDING_SET_SIZE_WORKSHOPS);
}

int building_set_count_small_temples(int active_only)
{
    return count_all_types_in_set(active_only, building_set_small_temples, BUILDING_SET_SIZE_SMALL_TEMPLES);
}

int building_set_count_large_temples(int active_only)
{
    return count_all_types_in_set(active_only, building_set_large_temples, BUILDING_SET_SIZE_LARGE_TEMPLES);
}

int building_set_count_deco_trees(void)
{
    return count_all_types_in_set(0, building_set_deco_trees, BUILDING_SET_SIZE_DECO_TREES);
}

int building_set_count_deco_paths(void)
{
    return count_all_types_in_set(0, building_set_deco_paths, BUILDING_SET_SIZE_DECO_PATHS);
}

int building_set_count_deco_statues(void)
{
    return count_all_types_in_set(0, building_set_deco_statues, BUILDING_SET_SIZE_DECO_STATUES);
}

int building_set_area_count_farms(int minx, int miny, int maxx, int maxy)
{
    return count_all_types_in_set_in_area(building_set_farms, BUILDING_SET_SIZE_FARMS, minx, miny, maxx, maxy);
}

int building_set_area_count_raw_materials(int minx, int miny, int maxx, int maxy)
{
    return count_all_types_in_set_in_area(building_set_raw_materials, BUILDING_SET_SIZE_RAW_MATERIALS, minx, miny, maxx, maxy);
}

int building_set_area_count_workshops(int minx, int miny, int maxx, int maxy)
{
    return count_all_types_in_set_in_area(building_set_workshops, BUILDING_SET_SIZE_WORKSHOPS, minx, miny, maxx, maxy);
}

int building_set_area_count_small_temples(int minx, int miny, int maxx, int maxy)
{
    return count_all_types_in_set_in_area(building_set_small_temples, BUILDING_SET_SIZE_SMALL_TEMPLES, minx, miny, maxx, maxy);
}

int building_set_area_count_large_temples(int minx, int miny, int maxx, int maxy)
{
    return count_all_types_in_set_in_area(building_set_large_temples, BUILDING_SET_SIZE_LARGE_TEMPLES, minx, miny, maxx, maxy);
}

int building_set_area_count_grand_temples(int minx, int miny, int maxx, int maxy)
{
    return count_all_types_in_set_in_area(building_set_grand_temples, BUILDING_SET_SIZE_GRAND_TEMPLES, minx, miny, maxx, maxy);
}

int building_set_area_count_deco_trees(int minx, int miny, int maxx, int maxy)
{
    return count_all_types_in_set_in_area(building_set_deco_trees, BUILDING_SET_SIZE_DECO_TREES, minx, miny, maxx, maxy);
}

int building_set_area_count_deco_paths(int minx, int miny, int maxx, int maxy)
{
    return count_all_types_in_set_in_area(building_set_deco_paths, BUILDING_SET_SIZE_DECO_PATHS, minx, miny, maxx, maxy);
}

int building_set_area_count_deco_statues(int minx, int miny, int maxx, int maxy)
{
    return count_all_types_in_set_in_area(building_set_deco_statues, BUILDING_SET_SIZE_DECO_STATUES, minx, miny, maxx, maxy);
}


int building_count_active_fort_type(figure_type type)
{
    int active = 0;
    for (building *b = building_first_of_type(BUILDING_FORT); b; b = b->next_of_type) {
        if (building_is_active(b) && b == building_main(b)) {
            if (b->subtype.fort_figure_type == type) {
                active++;
            }
        }
    }
    return active;
}

int building_count_fort_type_total(figure_type type)
{
    int total = 0;
    for (building *b = building_first_of_type(BUILDING_FORT); b; b = b->next_of_type) {
        if ((b->state == BUILDING_STATE_IN_USE || b->state == BUILDING_STATE_CREATED) && b == building_main(b)) {
            if (b->subtype.fort_figure_type == type) {
                total++;
            }
        }
    }
    return total;
}
