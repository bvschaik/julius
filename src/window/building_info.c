#include "building_info.h"

#include "building/barracks.h"
#include "building/culture.h"
#include "building/house_evolution.h"
#include "building/model.h"
#include "building/monument.h"
#include "building/warehouse.h"
#include "city/map.h"
#include "city/view.h"
#include "core/calc.h"
#include "core/image_group.h"
#include "figure/figure.h"
#include "figure/formation_legion.h"
#include "figure/phrase.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "map/aqueduct.h"
#include "map/building.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/property.h"
#include "map/sprite.h"
#include "map/terrain.h"
#include "window/advisors.h"
#include "window/city.h"
#include "window/message_dialog.h"
#include "window/building/distribution.h"
#include "window/building/figures.h"
#include "window/building/government.h"
#include "window/building/industry.h"
#include "window/building/house.h"
#include "window/building/military.h"
#include "window/building/terrain.h"
#include "window/building/utility.h"

#define OFFSET(x,y) (x + GRID_SIZE * y)

static void button_help(int param1, int param2);
static void button_close(int param1, int param2);
static void button_advisor(int advisor, int param2);
static void button_mothball(int mothball, int param2);
static void button_monument_construction(int param1, int param2);

static image_button image_buttons_help_close[] = {
    {14, 0, 27, 27, IB_NORMAL, GROUP_CONTEXT_ICONS, 0, button_help, button_none, 0, 0, 1},
    {424, 3, 24, 24, IB_NORMAL, GROUP_CONTEXT_ICONS, 4, button_close, button_none, 0, 0, 1}
};

static image_button image_buttons_advisor[] = {
    {350, -38, 28, 28, IB_NORMAL, GROUP_MESSAGE_ADVISOR_BUTTONS, 9, button_advisor, button_none, ADVISOR_RATINGS, 0, 1}
};

static image_button image_button_mothball[] = {
    {400, 3, 24, 24, IB_NORMAL, 0, 0, button_mothball, button_none, 0, 0, 1, "UI", "Mothball_1"},
    {400, 3, 24, 24, IB_NORMAL, 0, 0, button_mothball, button_none, 0, 0, 1, "UI", "Unmothball_1"}
};

static generic_button generic_button_monument_construction[] = {
    {80, 3, 304, 24, button_monument_construction, button_none, 0, 0}
};

static building_info_context context;
static int focus_image_button_id;
static int focus_mothball_image_button_id;
static int focus_monument_construction_button_id;

static int get_height_id(void)
{
    if (context.type == BUILDING_INFO_TERRAIN) {
        switch (context.terrain_type) {
            case TERRAIN_INFO_AQUEDUCT:
                return 4;
            case TERRAIN_INFO_RUBBLE:
            case TERRAIN_INFO_WALL:
            case TERRAIN_INFO_GARDEN:
                return 1;
            default:
                return 5;
        }
    } else if (context.type == BUILDING_INFO_LEGION) {
        return 7;
    } else if (context.type == BUILDING_INFO_BUILDING) {
        const building *b = building_get(context.building_id);
        if (building_is_house(b->type) && b->house_population <= 0) {
            return 5;
        }
        if (building_is_house(b->type) && b->house_population > 0) {
            return 7;
        }

        if (b->type >= BUILDING_SMALL_POND && b->type <= BUILDING_SMALL_STATUE_ALT_B) {
            return 1;
        }

        switch (b->type) {
            case BUILDING_SMALL_TEMPLE_CERES:
            case BUILDING_SMALL_TEMPLE_NEPTUNE:
            case BUILDING_SMALL_TEMPLE_MERCURY:
            case BUILDING_SMALL_TEMPLE_MARS:
            case BUILDING_SMALL_TEMPLE_VENUS:
            case BUILDING_SMALL_STATUE:
            case BUILDING_MEDIUM_STATUE:
            case BUILDING_LARGE_STATUE:
            case BUILDING_LEGION_STATUE:
            case BUILDING_DECORATIVE_COLUMN:
            case BUILDING_HORSE_STATUE:
            case BUILDING_GLADIATOR_SCHOOL:
            case BUILDING_LION_HOUSE:
            case BUILDING_ACTOR_COLONY:
            case BUILDING_CHARIOT_MAKER:
            case BUILDING_BURNING_RUIN:
            case BUILDING_RESERVOIR:
            case BUILDING_NATIVE_HUT:
            case BUILDING_NATIVE_MEETING:
            case BUILDING_NATIVE_CROPS:
            case BUILDING_MISSION_POST:
            case BUILDING_PREFECTURE:
            case BUILDING_ENGINEERS_POST:
            case BUILDING_GATEHOUSE:
            case BUILDING_TOWER:
            case BUILDING_FORT:
            case BUILDING_MILITARY_ACADEMY:
            case BUILDING_MARKET:
            case BUILDING_SHIPYARD:
            case BUILDING_WHARF:
            case BUILDING_GOVERNORS_HOUSE:
            case BUILDING_GOVERNORS_VILLA:
            case BUILDING_GOVERNORS_PALACE:
            case BUILDING_FORUM:
            case BUILDING_ROADBLOCK:
            case BUILDING_FORUM_UPGRADED:
            case BUILDING_WORKCAMP:
            case BUILDING_ARCHITECT_GUILD:
            case BUILDING_OBELISK:
            case BUILDING_HEDGE_DARK:
            case BUILDING_HEDGE_LIGHT:
            case BUILDING_LARARIUM:
            case BUILDING_COLONNADE:
            case BUILDING_GARDEN_PATH:
            case BUILDING_WATCHTOWER:
            case BUILDING_GARDEN_WALL:
            case BUILDING_ROOFED_GARDEN_WALL:
            case BUILDING_GARDEN_WALL_GATE:
            case BUILDING_PALISADE:
            case BUILDING_HEDGE_GATE_DARK:
            case BUILDING_HEDGE_GATE_LIGHT:
            case BUILDING_PALISADE_GATE:
            case BUILDING_GLADIATOR_STATUE:
                return 1;

            case BUILDING_SENATE:
            case BUILDING_SENATE_UPGRADED:
            case BUILDING_FOUNTAIN:
                return 2;

            case BUILDING_BARRACKS:
            case BUILDING_LARGE_TEMPLE_CERES:
            case BUILDING_LARGE_TEMPLE_NEPTUNE:
            case BUILDING_LARGE_TEMPLE_MERCURY:
            case BUILDING_LARGE_TEMPLE_MARS:
            case BUILDING_LARGE_TEMPLE_VENUS:
            case BUILDING_ORACLE:
            case BUILDING_NYMPHAEUM:
            case BUILDING_SMALL_MAUSOLEUM:
            case BUILDING_LARGE_MAUSOLEUM:
                return 3;

            case BUILDING_WELL:
                return 4;

            case BUILDING_DOCK:
            case BUILDING_LIGHTHOUSE:
            case BUILDING_CARAVANSERAI:
                return 6;

            case BUILDING_MESS_HALL:
                return 7;

            case BUILDING_GRAND_TEMPLE_CERES:
            case BUILDING_GRAND_TEMPLE_NEPTUNE:
            case BUILDING_GRAND_TEMPLE_MERCURY:
            case BUILDING_GRAND_TEMPLE_MARS:
            case BUILDING_GRAND_TEMPLE_VENUS:
            case BUILDING_PANTHEON:
            case BUILDING_HIPPODROME:
            case BUILDING_COLOSSEUM:
                return 8;

            case BUILDING_GRANARY:
                return 9;

            default:
                return 0;
        }
    }
    return 0;
}

static int has_mothball_button(void)
{
    return model_get_building(building_get(context.building_id)->type)->laborers;
}

static void draw_mothball_button(int x, int y)
{
    building *b = building_get(context.building_id);
    if (b->state == BUILDING_STATE_MOTHBALLED) {
        image_buttons_draw(x, y, image_button_mothball, 2);
    } else {
        image_buttons_draw(x, y, image_button_mothball, 1);
    }
}

static void draw_halt_monument_construction_button(int x, int y, int focused, building *monument)
{
    int width = BLOCK_SIZE * (context.width_blocks - 10);
    button_border_draw(x, y, width, 20, focused ? 1 : 0);
    if (monument->state != BUILDING_STATE_MOTHBALLED) {
        text_draw_centered(translation_for(TR_BUTTON_HALT_MONUMENT_CONSTRUCTION), x, y + 4, width, FONT_NORMAL_BLACK, 0);
    } else {
        text_draw_centered(translation_for(TR_BUTTON_RESUME_MONUMENT_CONSTRUCTION), x, y + 4, width, FONT_NORMAL_BLACK, 0);
    }
}

static int center_in_city(int element_width_pixels)
{
    int x, y, width, height;
    city_view_get_viewport(&x, &y, &width, &height);
    int margin = (width - element_width_pixels) / 2;
    return x + margin;
}

void highlight_waypoints(building *b) // highlight the 4 routing tiles for roams from this building
{
    map_clear_highlights();
    if (b->type == BUILDING_FORT || b->house_size) { // building doesn't send roamers
        return;
    }
    int hx, hy, roadx, roady;
    hx = b->x; hy = b->y - 8;
    map_grid_bound(&hx, &hy);
    if (map_closest_road_within_radius(hx, hy, 1, 6, &roadx, &roady)) {
        map_highlight_set(map_grid_offset(roadx, roady));
    }
    hx = b->x + 8; hy = b->y;
    map_grid_bound(&hx, &hy);
    if (map_closest_road_within_radius(hx, hy, 1, 6, &roadx, &roady)) {
        map_highlight_set(map_grid_offset(roadx, roady));
    }
    hx = b->x; hy = b->y + 8;
    map_grid_bound(&hx, &hy);
    if (map_closest_road_within_radius(hx, hy, 1, 6, &roadx, &roady)) {
        map_highlight_set(map_grid_offset(roadx, roady));
    }
    hx = b->x - 8; hy = b->y;
    map_grid_bound(&hx, &hy);
    if (map_closest_road_within_radius(hx, hy, 1, 6, &roadx, &roady)) {
        map_highlight_set(map_grid_offset(roadx, roady));
    }
    window_invalidate();
}

static void init(int grid_offset)
{
    context.can_play_sound = 1;
    context.storage_show_special_orders = 0;
    context.can_go_to_advisor = 0;
    context.building_id = map_building_at(grid_offset);
    context.rubble_building_type = map_rubble_building_type(grid_offset);
    context.has_reservoir_pipes = map_terrain_is(grid_offset, TERRAIN_RESERVOIR_RANGE);
    context.aqueduct_has_water = map_aqueduct_at(grid_offset)
        && map_image_at(grid_offset) - image_group(GROUP_BUILDING_AQUEDUCT) < 15;

    city_resource_determine_available();
    context.type = BUILDING_INFO_TERRAIN;
    context.figure.drawn = 0;
    if (!context.building_id && map_sprite_bridge_at(grid_offset) > 0) {
        if (map_terrain_is(grid_offset, TERRAIN_WATER)) {
            context.terrain_type = TERRAIN_INFO_BRIDGE;
        } else {
            context.terrain_type = TERRAIN_INFO_EMPTY;
        }
    } else if (map_property_is_plaza_or_earthquake(grid_offset)) {
        if (map_terrain_is(grid_offset, TERRAIN_ROAD)) {
            context.terrain_type = TERRAIN_INFO_PLAZA;
        }
        if (map_terrain_is(grid_offset, TERRAIN_ROCK)) {
            context.terrain_type = TERRAIN_INFO_EARTHQUAKE;
        }
    } else if (map_terrain_is(grid_offset, TERRAIN_TREE)) {
        context.terrain_type = TERRAIN_INFO_TREE;
    } else if (map_terrain_is(grid_offset, TERRAIN_ROCK)) {
        if (grid_offset == city_map_entry_flag()->grid_offset) {
            context.terrain_type = TERRAIN_INFO_ENTRY_FLAG;
        } else if (grid_offset == city_map_exit_flag()->grid_offset) {
            context.terrain_type = TERRAIN_INFO_EXIT_FLAG;
        } else {
            context.terrain_type = TERRAIN_INFO_ROCK;
        }
    } else if ((map_terrain_get(grid_offset) & (TERRAIN_WATER | TERRAIN_BUILDING)) == TERRAIN_WATER) {
        context.terrain_type = TERRAIN_INFO_WATER;
    } else if (map_terrain_is(grid_offset, TERRAIN_SHRUB)) {
        context.terrain_type = TERRAIN_INFO_SHRUB;
    } else if (map_terrain_is(grid_offset, TERRAIN_GARDEN)) {
        context.terrain_type = TERRAIN_INFO_GARDEN;
    } else if ((map_terrain_get(grid_offset) & (TERRAIN_ROAD | TERRAIN_BUILDING)) == TERRAIN_ROAD) {
        context.terrain_type = TERRAIN_INFO_ROAD;
    } else if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT)) {
        context.terrain_type = TERRAIN_INFO_AQUEDUCT;
    } else if (map_terrain_is(grid_offset, TERRAIN_RUBBLE)) {
        context.terrain_type = TERRAIN_INFO_RUBBLE;
    } else if (map_terrain_is(grid_offset, TERRAIN_WALL)) {
        context.terrain_type = TERRAIN_INFO_WALL;
    } else if (!context.building_id) {
        context.terrain_type = TERRAIN_INFO_EMPTY;
    } else {
        building *b = building_get(context.building_id);
        context.type = BUILDING_INFO_BUILDING;
        context.worker_percentage = calc_percentage(b->num_workers, model_get_building(b->type)->laborers);
        highlight_waypoints(b);

        switch (b->type) {
            case BUILDING_FORT_GROUND:
                context.building_id = b->prev_part_building_id;
                // fallthrough
            case BUILDING_FORT:
                context.formation_id = b->formation_id;
                break;
            case BUILDING_WAREHOUSE_SPACE:
            case BUILDING_HIPPODROME:
                b = building_main(b);
                context.building_id = b->id;
                break;
            case BUILDING_BARRACKS:
                context.barracks_soldiers_requested = formation_legion_recruits_needed();
                if (building_barracks_get_unmanned_tower(b, 0)) {
                    context.barracks_soldiers_requested++;
                }
                break;
            default:
                if (b->house_size) {
                    context.worst_desirability_building_id = building_house_determine_worst_desirability_building(b);
                    building_house_determine_evolve_text(b, context.worst_desirability_building_id);
                }
                break;
        }
        context.has_road_access = 0;
        switch (b->type) {
            case BUILDING_GRANARY:
                if (map_has_road_access_granary(b->x, b->y, 0)) {
                    context.has_road_access = 1;
                }
                break;
            case BUILDING_HIPPODROME:
                if (map_has_road_access_hippodrome_rotation(b->x, b->y, 0, b->subtype.orientation)) {
                    context.has_road_access = 1;
                }
                break;
            case BUILDING_WAREHOUSE:
                if (map_has_road_access_rotation(b->subtype.orientation, b->x, b->y, 3, 0)) {
                    context.has_road_access = 1;
                }
                context.warehouse_space_text = building_warehouse_get_space_info(b);
                break;
            case BUILDING_GRAND_TEMPLE_CERES:
            case BUILDING_GRAND_TEMPLE_NEPTUNE:
            case BUILDING_GRAND_TEMPLE_MERCURY:
            case BUILDING_GRAND_TEMPLE_MARS:
            case BUILDING_GRAND_TEMPLE_VENUS:
            case BUILDING_PANTHEON:
                if (map_has_road_access_monument_size7(b->x, b->y, 0)) {
                    context.has_road_access = 1;
                }
                break;
            case BUILDING_COLOSSEUM:
                if (map_has_road_access_monument_size5(b->x, b->y, 0)) {
                    context.has_road_access = 1;
                }
                break;
            case BUILDING_LIGHTHOUSE:
            case BUILDING_LARGE_TEMPLE_CERES:
            case BUILDING_LARGE_TEMPLE_MARS:
            case BUILDING_LARGE_TEMPLE_MERCURY:
            case BUILDING_LARGE_TEMPLE_NEPTUNE:
            case BUILDING_LARGE_TEMPLE_VENUS:
                if (map_has_road_access_monument_size3(b->x, b->y, 0)) {
                    context.has_road_access = 1;
                }
                break;
            case BUILDING_CARAVANSERAI:
                if (map_has_road_access_monument_size4(b->x, b->y, 0)) {
                    context.has_road_access = 1;
                }
                break;
            default:
                if (map_has_road_access(b->x, b->y, b->size, 0)) {
                    context.has_road_access = 1;
                }
                break;
        }
    }
    // figures
    context.figure.selected_index = 0;
    context.figure.count = 0;
    for (int i = 0; i < 7; i++) {
        context.figure.figure_ids[i] = 0;
    }
    static const int FIGURE_OFFSETS[] = {
        OFFSET(0,0), OFFSET(0,-1), OFFSET(0,1), OFFSET(1,0), OFFSET(-1,0),
        OFFSET(-1,-1), OFFSET(1,-1), OFFSET(-1,1), OFFSET(1,1)
    };
    for (int i = 0; i < 9 && context.figure.count < 7; i++) {
        int figure_id = map_figure_at(grid_offset + FIGURE_OFFSETS[i]);
        while (figure_id > 0 && context.figure.count < 7) {
            figure *f = figure_get(figure_id);
            if (f->state != FIGURE_STATE_DEAD &&
                f->action_state != FIGURE_ACTION_149_CORPSE) {
                switch (f->type) {
                    case FIGURE_NONE:
                    case FIGURE_EXPLOSION:
                    case FIGURE_MAP_FLAG:
                    case FIGURE_FLOTSAM:
                    case FIGURE_ARROW:
                    case FIGURE_JAVELIN:
                    case FIGURE_BOLT:
                    case FIGURE_BALLISTA:
                    case FIGURE_CREATURE:
                    case FIGURE_FISH_GULLS:
                    case FIGURE_SPEAR:
                    case FIGURE_HIPPODROME_HORSES:
                    case FIGURE_FRIENDLY_ARROW:
                    case FIGURE_WATCHTOWER_ARCHER:
                        break;
                    default:
                        context.figure.figure_ids[context.figure.count++] = figure_id;
                        figure_phrase_determine(f);
                        break;
                }
            }
            figure_id = f->next_figure_id_on_same_tile;
        }
    }
    // check for legion figures
    for (int i = 0; i < 7; i++) {
        int figure_id = context.figure.figure_ids[i];
        if (figure_id <= 0) {
            continue;
        }
        figure *f = figure_get(figure_id);
        if (f->type == FIGURE_FORT_STANDARD || figure_is_legion(f)) {
            context.type = BUILDING_INFO_LEGION;
            context.formation_id = f->formation_id;
            const formation *m = formation_get(context.formation_id);
            if (m->figure_type != FIGURE_FORT_LEGIONARY) {
                context.formation_types = 5;
            } else if (m->has_military_training) {
                context.formation_types = 4;
            } else {
                context.formation_types = 3;
            }
            break;
        }
    }
    // dialog size
    context.width_blocks = 29;

    switch (get_height_id()) {
        case 1: context.height_blocks = 16; break;
        case 2: context.height_blocks = 18; break;
        case 3: context.height_blocks = 19; break;
        case 4: context.height_blocks = 14; break;
        case 5: context.height_blocks = 24; break;
        case 6: context.height_blocks = 38; break;
        case 7: context.height_blocks = 26; break;
        case 8: context.height_blocks = 40; context.width_blocks = 30; break;
        case 9: context.height_blocks = 20; break;
        default: context.height_blocks = 22; break;
    }
    if (screen_height() <= 600) {
        context.height_blocks = calc_bound(context.height_blocks, 0, 26);
    }
    // dialog placement
    int s_width = screen_width();
    int s_height = screen_height();
    context.x_offset = center_in_city(BLOCK_SIZE * context.width_blocks);
    if (s_width >= 1024 && s_height >= 768) {
        context.x_offset = mouse_get()->x;
        context.y_offset = mouse_get()->y;
        window_building_set_possible_position(&context.x_offset, &context.y_offset,
            context.width_blocks, context.height_blocks);
    } else if (s_height >= 600 && mouse_get()->y <= (s_height - 24) / 2 + 24) {
        context.y_offset = s_height - BLOCK_SIZE * context.height_blocks - MARGIN_POSITION;
    } else {
        context.y_offset = MIN_Y_POSITION;
    }
}

static void draw_background(void)
{
    window_city_draw_panels();
    window_city_draw();
    if (context.type == BUILDING_INFO_NONE) {
        window_building_draw_no_people(&context);
    } else if (context.type == BUILDING_INFO_TERRAIN) {
        window_building_draw_terrain(&context);
    } else if (context.type == BUILDING_INFO_BUILDING) {
        int btype = building_get(context.building_id)->type;
        if (building_is_house(btype)) {
            window_building_draw_house(&context);
        } else if (btype == BUILDING_WHEAT_FARM) {
            window_building_draw_wheat_farm(&context);
        } else if (btype == BUILDING_VEGETABLE_FARM) {
            window_building_draw_vegetable_farm(&context);
        } else if (btype == BUILDING_FRUIT_FARM) {
            window_building_draw_fruit_farm(&context);
        } else if (btype == BUILDING_OLIVE_FARM) {
            window_building_draw_olive_farm(&context);
        } else if (btype == BUILDING_VINES_FARM) {
            window_building_draw_vines_farm(&context);
        } else if (btype == BUILDING_PIG_FARM) {
            window_building_draw_pig_farm(&context);
        } else if (btype == BUILDING_MARBLE_QUARRY) {
            window_building_draw_marble_quarry(&context);
        } else if (btype == BUILDING_IRON_MINE) {
            window_building_draw_iron_mine(&context);
        } else if (btype == BUILDING_TIMBER_YARD) {
            window_building_draw_timber_yard(&context);
        } else if (btype == BUILDING_CLAY_PIT) {
            window_building_draw_clay_pit(&context);
        } else if (btype == BUILDING_WINE_WORKSHOP) {
            window_building_draw_wine_workshop(&context);
        } else if (btype == BUILDING_OIL_WORKSHOP) {
            window_building_draw_oil_workshop(&context);
        } else if (btype == BUILDING_WEAPONS_WORKSHOP) {
            window_building_draw_weapons_workshop(&context);
        } else if (btype == BUILDING_FURNITURE_WORKSHOP) {
            window_building_draw_furniture_workshop(&context);
        } else if (btype == BUILDING_POTTERY_WORKSHOP) {
            window_building_draw_pottery_workshop(&context);
        } else if (btype == BUILDING_MARKET) {
            if (context.storage_show_special_orders) {
                window_building_draw_supplier_orders(&context, translation_for(TR_MARKET_SPECIAL_ORDERS_HEADER));
            } else {
                window_building_draw_market(&context);
            }
        } else if (btype == BUILDING_MESS_HALL) {
            if (context.storage_show_special_orders) {
                window_building_draw_supplier_orders(&context, translation_for(TR_MESS_HALL_SPECIAL_ORDERS_HEADER));
            } else {
                window_building_draw_mess_hall(&context);
            }
        } else if (btype == BUILDING_GRANARY) {
            if (context.storage_show_special_orders) {
                window_building_draw_granary_orders(&context);
            } else {
                window_building_draw_granary(&context);
            }
        } else if (btype == BUILDING_WAREHOUSE) {
            if (context.storage_show_special_orders) {
                window_building_draw_warehouse_orders(&context);
            } else {
                window_building_draw_warehouse(&context);
            }
        } else if (btype == BUILDING_AMPHITHEATER) {
            window_building_draw_amphitheater(&context);
        } else if (btype == BUILDING_THEATER) {
            window_building_draw_theater(&context);
        } else if (btype == BUILDING_HIPPODROME) {
            window_building_draw_hippodrome_background(&context);
        } else if (btype == BUILDING_COLOSSEUM) {
            window_building_draw_colosseum_background(&context);
        } else if (btype == BUILDING_ARENA) {
            window_building_draw_arena(&context);
        } else if (btype == BUILDING_GLADIATOR_SCHOOL) {
            window_building_draw_gladiator_school(&context);
        } else if (btype == BUILDING_LION_HOUSE) {
            window_building_draw_lion_house(&context);
        } else if (btype == BUILDING_ACTOR_COLONY) {
            window_building_draw_actor_colony(&context);
        } else if (btype == BUILDING_CHARIOT_MAKER) {
            window_building_draw_chariot_maker(&context);
        } else if (btype == BUILDING_DOCTOR) {
            window_building_draw_clinic(&context);
        } else if (btype == BUILDING_HOSPITAL) {
            window_building_draw_hospital(&context);
        } else if (btype == BUILDING_BATHHOUSE) {
            window_building_draw_bathhouse(&context);
        } else if (btype == BUILDING_BARBER) {
            window_building_draw_barber(&context);
        } else if (btype == BUILDING_SCHOOL) {
            window_building_draw_school(&context);
        } else if (btype == BUILDING_ACADEMY) {
            window_building_draw_academy(&context);
        } else if (btype == BUILDING_LIBRARY) {
            window_building_draw_library(&context);
        } else if (btype == BUILDING_SMALL_TEMPLE_CERES || btype == BUILDING_LARGE_TEMPLE_CERES) {
            if (context.storage_show_special_orders) {
                window_building_draw_supplier_orders(&context, translation_for(TR_TEMPLE_SPECIAL_ORDERS_HEADER));
            } else {
                window_building_draw_temple_ceres(&context);
            }
        } else if (btype == BUILDING_SMALL_TEMPLE_NEPTUNE || btype == BUILDING_LARGE_TEMPLE_NEPTUNE) {
            window_building_draw_temple_neptune(&context);
        } else if (btype == BUILDING_SMALL_TEMPLE_MERCURY || btype == BUILDING_LARGE_TEMPLE_MERCURY) {
            window_building_draw_temple_mercury(&context);
        } else if (btype == BUILDING_SMALL_TEMPLE_MARS || btype == BUILDING_LARGE_TEMPLE_MARS) {
            window_building_draw_temple_mars(&context);
        } else if (btype == BUILDING_SMALL_TEMPLE_VENUS || btype == BUILDING_LARGE_TEMPLE_VENUS) {
            if (context.storage_show_special_orders) {
                window_building_draw_supplier_orders(&context, translation_for(TR_TEMPLE_SPECIAL_ORDERS_HEADER));
            } else {
                window_building_draw_temple_venus(&context);
            }
        } else if (btype == BUILDING_ORACLE) {
            window_building_draw_oracle(&context);
        } else if (btype == BUILDING_LARARIUM) {
            window_building_draw_lararium(&context);
        } else if (btype == BUILDING_NYMPHAEUM) {
            window_building_draw_nymphaeum(&context);
        } else if (btype == BUILDING_SMALL_MAUSOLEUM) {
            window_building_draw_small_mausoleum(&context);
        } else if (btype == BUILDING_LARGE_MAUSOLEUM) {
            window_building_draw_large_mausoleum(&context);
        } else if (btype == BUILDING_WORKCAMP) {
            window_building_draw_work_camp(&context);
        } else if (btype == BUILDING_ARCHITECT_GUILD) {
            window_building_draw_architect_guild(&context);
        } else if (btype == BUILDING_MESS_HALL) {
            window_building_draw_mess_hall(&context);
        } else if (btype == BUILDING_TAVERN) {
            if (context.storage_show_special_orders) {
                window_building_draw_supplier_orders(&context, translation_for(TR_TAVERN_SPECIAL_ORDERS_HEADER));
            } else {
                window_building_draw_tavern(&context);
            }
        } else if (btype == BUILDING_GRAND_TEMPLE_CERES) {
            window_building_draw_grand_temple_ceres(&context);
        } else if (btype == BUILDING_GRAND_TEMPLE_NEPTUNE) {
            window_building_draw_grand_temple_neptune(&context);
        } else if (btype == BUILDING_GRAND_TEMPLE_MERCURY) {
            window_building_draw_grand_temple_mercury(&context);
        } else if (btype == BUILDING_GRAND_TEMPLE_MARS) {
            window_building_draw_grand_temple_mars(&context);
        } else if (btype == BUILDING_GRAND_TEMPLE_VENUS) {
            window_building_draw_grand_temple_venus(&context);
        } else if (btype == BUILDING_PANTHEON) {
            window_building_draw_pantheon(&context);
        } else if (btype == BUILDING_LIGHTHOUSE) {
            window_building_draw_lighthouse(&context);
        } else if (btype == BUILDING_GOVERNORS_HOUSE || btype == BUILDING_GOVERNORS_VILLA ||
            btype == BUILDING_GOVERNORS_PALACE) {
            window_building_draw_governor_home(&context);
        } else if (btype == BUILDING_FORUM || btype == BUILDING_FORUM_UPGRADED) {
            window_building_draw_forum(&context);
        } else if (btype == BUILDING_SENATE || btype == BUILDING_SENATE_UPGRADED) {
            window_building_draw_senate(&context);
        } else if (btype == BUILDING_ENGINEERS_POST) {
            window_building_draw_engineers_post(&context);
        } else if (btype == BUILDING_SHIPYARD) {
            window_building_draw_shipyard(&context);
        } else if (btype == BUILDING_DOCK) {
            if (context.storage_show_special_orders) {
                window_building_draw_dock_orders(&context);
            } else {
                window_building_draw_dock(&context);
            }
        } else if (btype == BUILDING_WHARF) {
            window_building_draw_wharf(&context);
        } else if (btype == BUILDING_RESERVOIR) {
            window_building_draw_reservoir(&context);
        } else if (btype == BUILDING_FOUNTAIN) {
            window_building_draw_fountain(&context);
        } else if (btype == BUILDING_WELL) {
            window_building_draw_well(&context);
        } else if (btype == BUILDING_SMALL_STATUE ||
            btype == BUILDING_MEDIUM_STATUE ||
            btype == BUILDING_SMALL_STATUE_ALT ||
            btype == BUILDING_SMALL_STATUE_ALT_B ||
            btype == BUILDING_LEGION_STATUE ||
            btype == BUILDING_DECORATIVE_COLUMN ||
            btype == BUILDING_HORSE_STATUE ||
            btype == BUILDING_GLADIATOR_STATUE) {
            window_building_draw_statue(&context);
        } else if (btype == BUILDING_LARGE_STATUE) {
            window_building_draw_large_statue(&context);
        } else if (btype == BUILDING_SMALL_POND || btype == BUILDING_LARGE_POND) {
            window_building_draw_pond(&context);
        } else if ((btype >= BUILDING_PINE_TREE && btype <= BUILDING_PAVILION_GREEN) ||
            (btype >= BUILDING_HEDGE_DARK && btype <= BUILDING_HEDGE_LIGHT) ||
            btype == BUILDING_COLONNADE || btype == BUILDING_GARDEN_PATH || btype == BUILDING_GARDEN_WALL ||
            btype == BUILDING_ROOFED_GARDEN_WALL) {
            window_building_draw_garden(&context);
        } else if (btype == BUILDING_TRIUMPHAL_ARCH) {
            window_building_draw_triumphal_arch(&context);
        } else if (btype == BUILDING_PREFECTURE) {
            window_building_draw_prefect(&context);
        } else if (btype == BUILDING_OBELISK) {
            window_building_draw_obelisk(&context);
        } else if (btype == BUILDING_ROADBLOCK) {
            if (context.storage_show_special_orders) {
                window_building_draw_roadblock_orders(&context);
            } else {
                window_building_draw_roadblock(&context);
            }
        } else if (btype == BUILDING_GATEHOUSE) {
            window_building_draw_gatehouse(&context);
        } else if (btype == BUILDING_TOWER) {
            window_building_draw_tower(&context);
        } else if (btype == BUILDING_MILITARY_ACADEMY) {
            window_building_draw_military_academy(&context);
        } else if (btype == BUILDING_BARRACKS) {
            window_building_draw_barracks(&context);
        } else if (btype == BUILDING_FORT) {
            window_building_draw_fort(&context);
        } else if (btype == BUILDING_BURNING_RUIN) {
            window_building_draw_burning_ruin(&context);
        } else if (btype == BUILDING_NATIVE_HUT) {
            window_building_draw_native_hut(&context);
        } else if (btype == BUILDING_NATIVE_MEETING) {
            window_building_draw_native_meeting(&context);
        } else if (btype == BUILDING_NATIVE_CROPS) {
            window_building_draw_native_crops(&context);
        } else if (btype == BUILDING_MISSION_POST) {
            window_building_draw_mission_post(&context);
        } else if (btype == BUILDING_WATCHTOWER) {
            window_building_draw_watchtower(&context);
        } else if (btype == BUILDING_CARAVANSERAI) {
            if (context.storage_show_special_orders) {
                window_building_draw_supplier_orders(&context, translation_for(TR_CARAVANSERAI_SPECIAL_ORDERS_HEADER));
            } else {
                window_building_draw_caravanserai(&context);
            }
        } else if (btype == BUILDING_GARDEN_WALL_GATE || btype == BUILDING_HEDGE_GATE_DARK || btype == BUILDING_HEDGE_GATE_LIGHT) {
            if (context.storage_show_special_orders) {
                window_building_draw_roadblock_orders(&context);
            } else {
                window_building_draw_garden_gate(&context);
            }
        } else if (btype == BUILDING_PALISADE) {
            window_building_draw_palisade(&context);
        } else if (btype == BUILDING_PALISADE_GATE) {
            if (context.storage_show_special_orders) {
                window_building_draw_roadblock_orders(&context);
            } else {
                window_building_draw_palisade_gate(&context);
            }
        }
    } else if (context.type == BUILDING_INFO_LEGION) {
        window_building_draw_legion_info(&context);
    }
}

static void draw_foreground(void)
{
    building *b = building_get(context.building_id);
    // building-specific buttons
    if (context.type == BUILDING_INFO_BUILDING) {
        int btype = building_get(context.building_id)->type;

        if (building_is_primary_product_producer(btype)) {
            window_building_draw_primary_product_stockpiling(&context);
        }

        if (btype == BUILDING_LIGHTHOUSE && b->data.monument.phase == MONUMENT_FINISHED) {
            window_building_draw_lighthouse_foreground(&context);
        } else if (btype == BUILDING_GRANARY) {
            if (context.storage_show_special_orders) {
                window_building_draw_granary_orders_foreground(&context);
            } else {
                window_building_draw_granary_foreground(&context);
            }
        } else if (btype == BUILDING_WAREHOUSE) {
            if (context.storage_show_special_orders) {
                window_building_draw_warehouse_orders_foreground(&context);
            } else {
                window_building_draw_warehouse_foreground(&context);
            }
        } else if (btype == BUILDING_MARKET) {
            if (context.storage_show_special_orders) {
                window_building_draw_supplier_orders_foreground(&context);
            } else {
                window_building_supplier_draw_foreground(&context);
            }
        } else if (btype == BUILDING_TAVERN) {
            if (context.storage_show_special_orders) {
                window_building_draw_supplier_orders_foreground(&context);
            } else {
                window_building_supplier_draw_foreground(&context);
            }
        } else if (btype == BUILDING_MESS_HALL) {
            if (context.storage_show_special_orders) {
                window_building_draw_supplier_orders_foreground(&context);
            } else {
                window_building_supplier_draw_foreground(&context);
            }
        } else if (building_is_venus_temple(btype) && b->data.monument.phase <= 0 &&
            building_monument_gt_module_is_active(VENUS_MODULE_1_DISTRIBUTE_WINE)) {
            if (context.storage_show_special_orders) {
                window_building_draw_supplier_orders_foreground(&context);
            } else {
                window_building_supplier_draw_foreground(&context);
            }
        } else if (building_is_ceres_temple(btype) && b->data.monument.phase <= 0 &&
            building_monument_gt_module_is_active(CERES_MODULE_2_DISTRIBUTE_FOOD)) {
            if (context.storage_show_special_orders) {
                window_building_draw_supplier_orders_foreground(&context);
            } else {
                window_building_supplier_draw_foreground(&context);
            }
        } else if (btype == BUILDING_ROADBLOCK) {
            if (context.storage_show_special_orders) {
                window_building_draw_roadblock_orders_foreground(&context);
            } else {
                window_building_draw_roadblock_foreground(&context);
            }
        } else if (btype == BUILDING_DOCK) {
            if (context.storage_show_special_orders) {
                window_building_draw_dock_orders_foreground(&context);
            } else {
                window_building_draw_dock_foreground(&context);
            }
        } else if (btype == BUILDING_BARRACKS) {
            window_building_draw_barracks_foreground(&context);
        } else if ((btype >= BUILDING_GRAND_TEMPLE_CERES && btype <= BUILDING_GRAND_TEMPLE_VENUS) ||
            btype == BUILDING_PANTHEON) {
            window_building_draw_grand_temple_foreground(&context);
        } else if (btype == BUILDING_CARAVANSERAI &&
            b->data.monument.phase == MONUMENT_FINISHED) {
            if (context.storage_show_special_orders) {
                window_building_draw_supplier_orders_foreground(&context);
            } else {
                window_building_supplier_draw_foreground(&context);
                window_building_draw_caravanserai_foreground(&context);
            }
        } else if (btype == BUILDING_COLOSSEUM) {
            window_building_draw_colosseum_foreground(&context);
        } else if (btype == BUILDING_HIPPODROME) {
            window_building_draw_hippodrome_foreground(&context);
        } else if (btype == BUILDING_GARDEN_WALL_GATE || btype == BUILDING_HEDGE_GATE_DARK || btype == BUILDING_HEDGE_GATE_LIGHT || btype == BUILDING_PALISADE_GATE) {
            if (context.storage_show_special_orders) {
                window_building_draw_roadblock_orders_foreground(&context);
            } else {
                window_building_draw_garden_gate_foreground(&context);
            }
        }

        if (building_monument_is_unfinished_monument(b)) {
            draw_halt_monument_construction_button(context.x_offset + 80,
                context.y_offset + 3 + BLOCK_SIZE * context.height_blocks - 40,
                focus_monument_construction_button_id, b);
        }
    } else if (context.type == BUILDING_INFO_LEGION) {
        window_building_draw_legion_info_foreground(&context);
    }
    // general buttons
    if (context.storage_show_special_orders) {
        int y_offset = window_building_get_vertical_offset(&context, 28);
        image_buttons_draw(context.x_offset, y_offset + 400, image_buttons_help_close, 2);
    } else {
        image_buttons_draw(context.x_offset, context.y_offset + BLOCK_SIZE * context.height_blocks - 40,
            image_buttons_help_close, 2);
    }
    if (context.can_go_to_advisor) {
        image_buttons_draw(context.x_offset, context.y_offset + BLOCK_SIZE * context.height_blocks - 40,
            image_buttons_advisor, 1);
    }
    if (!context.storage_show_special_orders && !building_monument_is_unfinished_monument(b) && has_mothball_button()) {
            draw_mothball_button(context.x_offset, context.y_offset + BLOCK_SIZE * context.height_blocks - 40);
    }
}

static int handle_specific_building_info_mouse(const mouse *m)
{
    // building-specific buttons
    if (context.type == BUILDING_INFO_NONE) {
        return 0;
    }

    if (context.type == BUILDING_INFO_LEGION) {
        return window_building_handle_mouse_legion_info(m, &context);
    } else if (context.figure.drawn) {
        return window_building_handle_mouse_figure_list(m, &context);
    } else if (context.type == BUILDING_INFO_BUILDING) {
        int btype = building_get(context.building_id)->type;

        if (building_has_supplier_inventory(btype)) {
            if (context.storage_show_special_orders) {
                window_building_handle_mouse_supplier_orders(m, &context);
            } else {
                if (btype == BUILDING_CARAVANSERAI) {
                    window_building_handle_mouse_caravanserai(m, &context);
                }
                window_building_handle_mouse_supplier(m, &context);
            }
        } else if (btype == BUILDING_ROADBLOCK) {
            if (context.storage_show_special_orders) {
                return window_building_handle_mouse_roadblock_orders(m, &context);
            } else {
                return window_building_handle_mouse_roadblock(m, &context);
            }
        } else if (btype == BUILDING_DOCK) {
            if (context.storage_show_special_orders) {
                return window_building_handle_mouse_dock_orders(m, &context);
            } else {
                return window_building_handle_mouse_dock(m, &context);
            }
        } else if (btype == BUILDING_BARRACKS) {
            return window_building_handle_mouse_barracks(m, &context);
        } else if (btype == BUILDING_GRAND_TEMPLE_MARS) {
            window_building_handle_mouse_grand_temple_mars(m, &context);
        } else if (btype == BUILDING_GRANARY) {
            if (context.storage_show_special_orders) {
                return window_building_handle_mouse_granary_orders(m, &context);
            } else {
                return window_building_handle_mouse_granary(m, &context);
            }
        } else if (btype == BUILDING_WAREHOUSE) {
            if (context.storage_show_special_orders) {
                window_building_handle_mouse_warehouse_orders(m, &context);
            } else {
                window_building_handle_mouse_warehouse(m, &context);
            }
        } else if ((btype >= BUILDING_GRAND_TEMPLE_CERES && btype <= BUILDING_GRAND_TEMPLE_VENUS) ||
            btype == BUILDING_PANTHEON) {
            window_building_handle_mouse_grand_temple(m, &context);
        } else if (btype == BUILDING_LIGHTHOUSE) {
            window_building_handle_mouse_lighthouse(m, &context);
        } else if (btype == BUILDING_COLOSSEUM) {
            window_building_handle_mouse_colosseum(m, &context);
        } else if (btype == BUILDING_HIPPODROME) {
            window_building_handle_mouse_hippodrome(m, &context);
        } else if (btype == BUILDING_GARDEN_WALL_GATE || btype == BUILDING_HEDGE_GATE_DARK || btype == BUILDING_HEDGE_GATE_LIGHT || btype == BUILDING_PALISADE_GATE) {
            if (context.storage_show_special_orders) {
                return window_building_handle_mouse_roadblock_orders(m, &context);
            } else {
                return window_building_handle_mouse_garden_gate(m, &context);
            }
        } else if (building_is_primary_product_producer(btype)) {
            window_building_handle_mouse_primary_product_producer(m, &context);
        }
    }
    return 0;
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    int handled = 0;
    // general buttons
    if (context.storage_show_special_orders) {
        int y_offset = window_building_get_vertical_offset(&context, 28);
        handled |= image_buttons_handle_mouse(m, context.x_offset, y_offset + 400,
            image_buttons_help_close, 2, &focus_image_button_id);
    } else {
        handled |= image_buttons_handle_mouse(
            m, context.x_offset, context.y_offset + BLOCK_SIZE * context.height_blocks - 40,
            image_buttons_help_close, 2, &focus_image_button_id);
        building *b = building_get(context.building_id);
        if (building_monument_is_unfinished_monument(b)) {
            handled = generic_buttons_handle_mouse(
                m, context.x_offset, context.y_offset + BLOCK_SIZE * context.height_blocks - 40,
                generic_button_monument_construction, 1, &focus_monument_construction_button_id);
        } else {
            if (has_mothball_button()) {
                if (b->state == BUILDING_STATE_MOTHBALLED) {
                    handled |= image_buttons_handle_mouse(m, context.x_offset, context.y_offset + BLOCK_SIZE * context.height_blocks - 40,
                        image_button_mothball, 2, &focus_mothball_image_button_id);
                } else {
                    handled |= image_buttons_handle_mouse(m, context.x_offset, context.y_offset + BLOCK_SIZE * context.height_blocks - 40,
                        image_button_mothball, 1, &focus_mothball_image_button_id);
                }
            }
        }
    }
    if (context.can_go_to_advisor) {
        handled |= image_buttons_handle_mouse(
            m, context.x_offset, context.y_offset + BLOCK_SIZE * context.height_blocks - 40,
            image_buttons_advisor, 1, 0);
    }

    if (!handled) {
        handled |= handle_specific_building_info_mouse(m);
    }
    if (!handled && input_go_back_requested(m, h)) {
        window_city_show();
    }
}

static void get_tooltip(tooltip_context *c)
{
    int text_id = 0, group_id = 0, translation = 0;
    const uint8_t *precomposed_text = 0;
    building *b = building_get(context.building_id);
    int btype = b->type;
    if (focus_image_button_id) {
        text_id = focus_image_button_id;
    } else if (focus_mothball_image_button_id && has_mothball_button()) {
        if (!building_monument_is_unfinished_monument(b)) {
            if (building_get(context.building_id)->state == BUILDING_STATE_MOTHBALLED) {
                translation = TR_TOOLTIP_BUTTON_MOTHBALL_OFF;
            } else {
                translation = TR_TOOLTIP_BUTTON_MOTHBALL_ON;
            }
        }
    } else if (building_is_primary_product_producer(btype)) {
        window_building_primary_product_producer_stockpiling_tooltip(&translation);
    } else if (context.type == BUILDING_INFO_LEGION) {
        text_id = window_building_get_legion_info_tooltip_text(&context);
    } else if (context.type == BUILDING_INFO_BUILDING && context.storage_show_special_orders) {
        if (btype == BUILDING_GRANARY) {
            window_building_get_tooltip_granary_orders(&group_id, &text_id, &translation);
        } else if (btype == BUILDING_WAREHOUSE) {
            window_building_get_tooltip_warehouse_orders(&group_id, &text_id, &translation);
        } else if (btype == BUILDING_ROADBLOCK || btype == BUILDING_GARDEN_WALL_GATE || btype == BUILDING_HEDGE_GATE_DARK || btype == BUILDING_HEDGE_GATE_LIGHT) {
            window_building_roadblock_get_tooltip_walker_permissions(&translation);
        }
    } else if (btype == BUILDING_GRANARY) {
        window_building_granary_get_tooltip_distribution_permissions(&translation);
    } else if (btype == BUILDING_WAREHOUSE) {
        window_building_warehouse_get_tooltip_distribution_permissions(&translation);
    } else if (btype == BUILDING_DOCK) {
        precomposed_text = window_building_dock_get_tooltip(&context);        
    }
    if (!text_id && !group_id && !translation) {
        if (btype >= BUILDING_WHEAT_FARM && btype <= BUILDING_POTTERY_WORKSHOP) {
            window_building_industry_get_tooltip(&context, &translation);
        }
    }
    if (text_id || group_id || translation || precomposed_text) {
        c->type = TOOLTIP_BUTTON;
        c->text_id = text_id;
        c->translation_key = translation;
        if (group_id) {
            c->text_group = group_id;
        }
        c->precomposed_text = precomposed_text;
    }
}

static void button_help(int param1, int param2)
{
    if (context.help_id > 0) {
        window_message_dialog_show(context.help_id, window_city_draw_all);
    } else {
        window_message_dialog_show(MESSAGE_DIALOG_HELP, window_city_draw_all);
    }
    window_invalidate();
}

static void button_close(int param1, int param2)
{
    if (context.storage_show_special_orders) {
        context.storage_show_special_orders = 0;
        window_invalidate();
    } else {
        window_city_show();
    }
}

static void button_advisor(int advisor, int param2)
{
    window_advisors_show_advisor(advisor);
}

static void button_mothball(int mothball, int param2)
{
    building *b = building_get(context.building_id);
    int workers_needed = model_get_building(b->type)->laborers;
    if (workers_needed) {
        building_mothball_toggle(b);
        window_invalidate();
    }
}

static void button_monument_construction(int param1, int param2)
{
    building *b = building_get(context.building_id);
    building_monument_toggle_construction_halted(b);
    window_invalidate();
}

void window_building_info_show(int grid_offset)
{
    window_type window = {
        WINDOW_BUILDING_INFO,
        draw_background,
        draw_foreground,
        handle_input,
        get_tooltip
    };
    init(grid_offset);
    window_show(&window);
}

int window_building_info_get_building_type(void)
{
    if (context.type == BUILDING_INFO_BUILDING) {
        return building_get(context.building_id)->type;
    }
    return BUILDING_NONE;
}

void window_building_info_show_storage_orders(void)
{
    context.storage_show_special_orders = 1;
    window_invalidate();
}
