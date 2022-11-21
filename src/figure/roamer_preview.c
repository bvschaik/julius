#include "roamer_preview.h"

#include "building/industry.h"
#include "building/properties.h"
#include "core/config.h"
#include "figure/figure.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/road_access.h"

#include <string.h>

#define TOTAL_ROAMERS 4
#define MAX_STORED_BUILDING_TYPES 4
#define SHOWN_BUILDING_OFFSET 12

static struct {
    grid_u8 travelled_tiles;
    building_type types[MAX_STORED_BUILDING_TYPES];
    int stored_building_types;
} data;

static figure_type building_type_to_figure_type(building_type type)
{
    switch (type) {
        case BUILDING_MARKET:
            return FIGURE_MARKET_TRADER;
        case BUILDING_SENATE:
        case BUILDING_SENATE_UPGRADED:
        case BUILDING_FORUM:
        case BUILDING_FORUM_UPGRADED:
            return FIGURE_TAX_COLLECTOR;
        case BUILDING_SCHOOL:
            return FIGURE_SCHOOL_CHILD;
        case BUILDING_LIBRARY:
            return FIGURE_LIBRARIAN;
        case BUILDING_ACADEMY:
            return FIGURE_TEACHER;
        case BUILDING_ENGINEERS_POST:
            return FIGURE_ENGINEER;
        case BUILDING_PREFECTURE:
            return FIGURE_PREFECT;
        case BUILDING_TAVERN:
            return FIGURE_BARKEEP;
        case BUILDING_THEATER:
            return FIGURE_ACTOR;
        case BUILDING_AMPHITHEATER:
            return FIGURE_GLADIATOR;
        case BUILDING_ARENA:
        case BUILDING_COLOSSEUM:
            return FIGURE_LION_TAMER;
        case BUILDING_HIPPODROME:
            return FIGURE_CHARIOTEER;
        case BUILDING_BATHHOUSE:
            return FIGURE_BATHHOUSE_WORKER;
        case BUILDING_BARBER:
            return FIGURE_BARBER;
        case BUILDING_DOCTOR:
            return FIGURE_DOCTOR;
        case BUILDING_HOSPITAL:
            return FIGURE_SURGEON;
        case BUILDING_PANTHEON:
        case BUILDING_GRAND_TEMPLE_CERES:
        case BUILDING_GRAND_TEMPLE_MARS:
        case BUILDING_GRAND_TEMPLE_MERCURY:
        case BUILDING_GRAND_TEMPLE_NEPTUNE:
        case BUILDING_GRAND_TEMPLE_VENUS:
        case BUILDING_LARGE_TEMPLE_CERES:
        case BUILDING_LARGE_TEMPLE_MARS:
        case BUILDING_LARGE_TEMPLE_MERCURY:
        case BUILDING_LARGE_TEMPLE_NEPTUNE:
        case BUILDING_LARGE_TEMPLE_VENUS:
        case BUILDING_SMALL_TEMPLE_CERES:
        case BUILDING_SMALL_TEMPLE_MARS:
        case BUILDING_SMALL_TEMPLE_MERCURY:
        case BUILDING_SMALL_TEMPLE_NEPTUNE:
        case BUILDING_SMALL_TEMPLE_VENUS:
            return FIGURE_PRIEST;
        case BUILDING_MISSION_POST:
            return FIGURE_MISSIONARY;
        case BUILDING_BARRACKS:
        case BUILDING_MILITARY_ACADEMY:
        case BUILDING_MESS_HALL:
        case BUILDING_WAREHOUSE:
        case BUILDING_GRANARY:
        case BUILDING_TOWER:
        case BUILDING_ACTOR_COLONY:
        case BUILDING_GLADIATOR_SCHOOL:
        case BUILDING_LION_HOUSE:
        case BUILDING_CHARIOT_MAKER:
        case BUILDING_WHEAT_FARM:
        case BUILDING_PIG_FARM:
        case BUILDING_FRUIT_FARM:
        case BUILDING_OLIVE_FARM:
        case BUILDING_VINES_FARM:
        case BUILDING_VEGETABLE_FARM:
        case BUILDING_CLAY_PIT:
        case BUILDING_TIMBER_YARD:
        case BUILDING_IRON_MINE:
        case BUILDING_MARBLE_QUARRY:
        case BUILDING_POTTERY_WORKSHOP:
        case BUILDING_OIL_WORKSHOP:
        case BUILDING_WINE_WORKSHOP:
        case BUILDING_FURNITURE_WORKSHOP:
        case BUILDING_WEAPONS_WORKSHOP:
        case BUILDING_WORKCAMP:
        case BUILDING_ARCHITECT_GUILD:
        case BUILDING_WHARF:
        case BUILDING_DOCK:
        case BUILDING_SHIPYARD:
        case BUILDING_CARAVANSERAI:
        case BUILDING_LIGHTHOUSE:
        case BUILDING_WATCHTOWER:
            return FIGURE_LABOR_SEEKER;
        default:
            return FIGURE_NONE;
    }
}

static int roam_length_for_figure_type(figure_type type)
{
    switch (type) {
        case FIGURE_TAX_COLLECTOR:
        case FIGURE_ACTOR:
        case FIGURE_GLADIATOR:
        case FIGURE_LION_TAMER:
            return 34;
        case FIGURE_CHARIOTEER:
            return 68;
        case FIGURE_MISSIONARY:
            return 12;
        case FIGURE_ENGINEER:
        case FIGURE_PREFECT:
            return 42;
        default:
            return 25;
    }
}

static int figure_enters_exits_building(figure_type type)
{
    switch (type) {
        case FIGURE_TAX_COLLECTOR:
        case FIGURE_ENGINEER:
        case FIGURE_PREFECT:
            return 1;
        default:
            return 0;
    }
}

static void init_roaming(figure *f, int roam_dir, int x, int y)
{
    f->progress_on_tile = 15;
    f->roam_choose_destination = 0;
    f->roam_ticks_until_next_turn = -1;
    f->roam_turn_direction = 2;
    f->roam_length = 0;

    if (config_get(CONFIG_GP_CH_ROAMERS_DONT_SKIP_CORNERS)) {
        f->disallow_diagonal = 1;
    }
    switch (roam_dir) {
        case DIR_0_TOP: y -= 8; break;
        case DIR_2_RIGHT: x += 8; break;
        case DIR_4_BOTTOM: y += 8; break;
        case DIR_6_LEFT: x -= 8; break;
    }
    map_grid_bound(&x, &y);
    int x_road, y_road;
    if (map_closest_road_within_radius(x, y, 1, 6, &x_road, &y_road)) {
        f->destination_x = x_road;
        f->destination_y = y_road;
    } else {
        f->roam_choose_destination = 1;
    }
}

void figure_roamer_preview_create(building_type b_type, int grid_offset, int x, int y)
{
    if (!config_get(CONFIG_UI_SHOW_ROAMING_PATH)) {
        figure_roamer_preview_reset(BUILDING_NONE);
        return;
    }

    figure_type fig_type = building_type_to_figure_type(b_type);
    if (fig_type == FIGURE_NONE) {
        return;
    }

    if (fig_type == FIGURE_LABOR_SEEKER && config_get(CONFIG_GP_CH_GLOBAL_LABOUR)) {
        return;
    }

    if (data.travelled_tiles.items[grid_offset] == SHOWN_BUILDING_OFFSET) {
        return;
    }

    data.travelled_tiles.items[grid_offset] = SHOWN_BUILDING_OFFSET;

    int b_size = building_is_farm(b_type) ? 3 : building_properties_for_type(b_type)->size;

    map_point road;
    if (!map_has_road_access(x, y, b_size, &road)) {
        return;
    }

    int figure_walks_into_building = figure_enters_exits_building(fig_type);

    int x_road, y_road;
    int has_closest_road = map_closest_road_within_radius(x, y, b_size, 2, &x_road, &y_road);

    if (figure_walks_into_building && !has_closest_road) {
        return;
    }

    int roam_length = roam_length_for_figure_type(fig_type);

    int should_return = fig_type != FIGURE_SCHOOL_CHILD;

    for (int i = 0; i < TOTAL_ROAMERS; i++) {
        figure roamer;
        
        memset(&roamer, 0, sizeof(figure));
        
        roamer.source_x = roamer.destination_x = roamer.previous_tile_x = road.x;
        roamer.source_y = roamer.destination_y = roamer.previous_tile_y = road.y;
        roamer.terrain_usage = TERRAIN_USAGE_ROADS;
        roamer.direction = DIR_0_TOP;
        roamer.faction_id = FIGURE_FACTION_ROAMER_PREVIEW;
        roamer.type = fig_type;
        roamer.max_roam_length = roam_length;

        if (figure_walks_into_building) {
            roamer.x = x_road;
            roamer.y = y_road;
        } else {
            roamer.x = road.x;
            roamer.y = road.y;
        }
        roamer.grid_offset = map_grid_offset(roamer.x, roamer.y);
        if (map_grid_is_valid_offset(roamer.grid_offset)) {
            data.travelled_tiles.items[roamer.grid_offset] = FIGURE_ROAMER_PREVIEW_EXIT_TILE;
        }
        init_roaming(&roamer, i * 2, x, y);
        while (++roamer.roam_length <= roamer.max_roam_length) {
            if (data.travelled_tiles.items[roamer.grid_offset] < FIGURE_ROAMER_PREVIEW_MAX_PASSAGES) {
                data.travelled_tiles.items[roamer.grid_offset]++;
            }
            roamer.progress_on_tile = 15;
            figure_movement_roam_ticks(&roamer, 1);
        }
        figure_route_remove(&roamer);
        if (!should_return || !has_closest_road) {
            continue;
        }
        roamer.destination_x = x_road;
        roamer.destination_y = y_road;
        while (roamer.direction != DIR_FIGURE_AT_DESTINATION &&
            roamer.direction != DIR_FIGURE_REROUTE && roamer.direction != DIR_FIGURE_LOST) {
            if (data.travelled_tiles.items[roamer.grid_offset] < FIGURE_ROAMER_PREVIEW_MAX_PASSAGES) {
                data.travelled_tiles.items[roamer.grid_offset]++;
            }
            roamer.progress_on_tile = 15;
            figure_movement_move_ticks(&roamer, 1);
        }
        figure_route_remove(&roamer);
        if (roamer.direction == DIR_FIGURE_AT_DESTINATION) {
            int tile_type = data.travelled_tiles.items[roamer.grid_offset];
            data.travelled_tiles.items[roamer.grid_offset] = tile_type < FIGURE_ROAMER_PREVIEW_EXIT_TILE ?
                FIGURE_ROAMER_PREVIEW_ENTRY_TILE : FIGURE_ROAMER_PREVIEW_ENTRY_EXIT_TILE;
        }
    }
}

void figure_roamer_preview_create_all_for_building_type(building_type type)
{
    if (type == BUILDING_NONE) {
        return;
    }
    if (!config_get(CONFIG_UI_SHOW_ROAMING_PATH)) {
        figure_roamer_preview_reset_building_types();
        return;
    }
    for (int i = 0; i < data.stored_building_types; i++) {
        if (data.types[i] == type) {
            return;
        }
    }
    if (data.stored_building_types == MAX_STORED_BUILDING_TYPES) {
        return;
    }
    for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
        figure_roamer_preview_create(type, b->grid_offset, b->x, b->y);
    }
    data.types[data.stored_building_types] = type;
    data.stored_building_types++;
}

void figure_roamer_preview_reset(building_type type)
{
    map_grid_clear_u8(data.travelled_tiles.items);
    int show_other_roamers = 0;
    figure_type fig_type = building_type_to_figure_type(type);
    if (fig_type == FIGURE_LABOR_SEEKER && config_get(CONFIG_GP_CH_GLOBAL_LABOUR)) {
        fig_type = FIGURE_NONE;
    }
    if (fig_type == FIGURE_NONE) {
        show_other_roamers = 1;
    } else {
        for (int i = 0; i < data.stored_building_types; i++) {
            if (building_type_to_figure_type(data.types[i]) == fig_type) {
                show_other_roamers = 1;
                break;
            }
        }
    }
    if (show_other_roamers) {
        for (int i = 0; i < data.stored_building_types; i++) {
            for (building *b = building_first_of_type(data.types[i]); b; b = b->next_of_type) {
                figure_roamer_preview_create(b->type, b->grid_offset, b->x, b->y);
            }
        }
    }
}

void figure_roamer_preview_reset_building_types(void)
{
    data.stored_building_types = 0;
    figure_roamer_preview_reset(BUILDING_NONE);
}

int figure_roamer_preview_get_frequency(int grid_offset)
{
    return map_grid_is_valid_offset(grid_offset) ? data.travelled_tiles.items[grid_offset] : 0;
}
