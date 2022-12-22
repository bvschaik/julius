#include "figure/figure.h"

#include "building/building.h"
#include "core/array.h"
#include "core/log.h"
#include "city/emperor.h"
#include "core/random.h"
#include "game/resource.h"
#include "game/save_version.h"
#include "empire/city.h"
#include "figure/name.h"
#include "figure/route.h"
#include "figure/trader.h"
#include "figure/visited_buildings.h"
#include "map/figure.h"
#include "map/grid.h"

#define FIGURE_ARRAY_SIZE_STEP 1000

#define FIGURE_ORIGINAL_BUFFER_SIZE 128
#define FIGURE_CURRENT_BUFFER_SIZE 130

static struct {
    int created_sequence;
    array(figure) figures;
} data;

figure *figure_get(int id)
{
    return array_item(data.figures, id);
}

int figure_count(void)
{
    return data.figures.size;
}

figure *figure_create(figure_type type, int x, int y, direction_type dir)
{
    figure *f = 0;
    array_new_item(data.figures, 1, f);
    if (!f) {
        return array_first(data.figures);
    }

    f->state = FIGURE_STATE_ALIVE;
    f->faction_id = 1;
    f->type = type;
    f->use_cross_country = 0;
    f->is_friendly = 1;
    f->created_sequence = data.created_sequence++;
    f->direction = dir;
    f->source_x = f->destination_x = f->previous_tile_x = f->x = x;
    f->source_y = f->destination_y = f->previous_tile_y = f->y = y;
    f->grid_offset = map_grid_offset(x, y);
    f->cross_country_x = 15 * x;
    f->cross_country_y = 15 * y;
    f->progress_on_tile = 15;
    f->progress_to_next_tick = 0;
    f->dont_draw_elevated = 0;
    f->disallow_diagonal = 0;
    f->resource_id = RESOURCE_NONE;
    f->destination_building_id = 0;
    f->wait_ticks = 0;
    random_generate_next();
    f->phrase_sequence_city = f->phrase_sequence_exact = random_byte() & 3;
    f->name = figure_name_get(type, 0);
    map_figure_add(f);
    if (type == FIGURE_TRADE_CARAVAN || type == FIGURE_TRADE_SHIP) {
        f->trader_id = trader_create();
    }
    return f;
}

void figure_delete(figure *f)
{
    building *b = building_get(f->building_id);
    switch (f->type) {
        case FIGURE_LABOR_SEEKER:
        case FIGURE_MARKET_SUPPLIER:
        case FIGURE_PRIEST_SUPPLIER:
        case FIGURE_BARKEEP_SUPPLIER:
        case FIGURE_MESS_HALL_SUPPLIER:
        case FIGURE_CARAVANSERAI_SUPPLIER:
        case FIGURE_LIGHTHOUSE_SUPPLIER:
            if (f->building_id && f->id == b->figure_id2) {
                b->figure_id2 = 0;
            } else if (f->building_id && f->id == b->figure_id) {
                b->figure_id = 0;
            } else if (f->building_id && f->id == b->figure_id4) {
                b->figure_id4 = 0;
            }
            break;
        case FIGURE_SURGEON:
        case FIGURE_DOCTOR:
            // if doctor was going to illness plague building, then remove figure_id4 to the dock
            if (f->destination_building_id) {
                building *b_dest = building_get(f->destination_building_id);
                if (b_dest->figure_id4 && f->id == b_dest->figure_id4) {
                    b_dest->figure_id4 = 0;
                }
            }
            break;
        case FIGURE_BALLISTA:
        case FIGURE_WATCHTOWER_ARCHER:
            b->figure_id4 = 0;
            break;
        case FIGURE_DOCKER:
            for (int i = 0; i < 3; i++) {
                if (b->data.dock.docker_ids[i] == f->id) {
                    b->data.dock.docker_ids[i] = 0;
                }
            }
            break;
        case FIGURE_PRIEST:
            if (f->building_id && f->destination_building_id && f->id == b->figure_id2) {
                b->figure_id2 = 0;
            }
            if (f->building_id && f->destination_building_id && f->id == b->figure_id4) {
                b->figure_id4 = 0;
            }
            break;
        case FIGURE_ENEMY_CAESAR_LEGIONARY:
            city_emperor_mark_soldier_killed();
            break;
        case FIGURE_CHARIOTEER:
            if (building_is_neptune_temple(b->type)) {
                b->figure_id2 = 0;
            } else {
                b->figure_id = 0;
            }
            break;
        case FIGURE_EXPLOSION:
        case FIGURE_FORT_STANDARD:
        case FIGURE_ARROW:
        case FIGURE_JAVELIN:
        case FIGURE_FRIENDLY_ARROW:
        case FIGURE_BOLT:
        case FIGURE_SPEAR:
        case FIGURE_FISH_GULLS:
        case FIGURE_SHEEP:
        case FIGURE_WOLF:
        case FIGURE_ZEBRA:
        case FIGURE_DELIVERY_BOY:
        case FIGURE_PATRICIAN:
        case FIGURE_MESS_HALL_COLLECTOR:
        case FIGURE_TRADE_SHIP:
            // nothing to do here
            break;
        case FIGURE_WATCHMAN:
            if (f->id == b->figure_id2) {
                b->figure_id2 = 0;
            } else if (f->id == b->figure_id) {
                b->figure_id = 0;
            }
            break;
        case FIGURE_MESS_HALL_FORT_SUPPLIER:
        {
            building *fort = building_get(f->destination_building_id);
            if (fort->figure_id2 == f->id) {
                fort->figure_id2 = 0;
            }
        }
        default:
            if (f->building_id) {
                b->figure_id = 0;
            }
            break;
    }
    if (f->empire_city_id) {
        empire_city_remove_trader(f->empire_city_id, f->id);
    }
    if (f->immigrant_building_id) {
        building_get(f->immigrant_building_id)->immigrant_figure_id = 0;
    }
    figure_visited_buildings_remove_list(f->last_visited_index);
    figure_route_remove(f);
    map_figure_delete(f);

    int figure_id = f->id;
    memset(f, 0, sizeof(figure));
    f->id = figure_id;

    array_trim(data.figures);
}

int figure_is_dead(const figure *f)
{
    return f->state != FIGURE_STATE_ALIVE || f->action_state == FIGURE_ACTION_149_CORPSE;
}

int figure_is_enemy(const figure *f)
{
    return f->type >= FIGURE_ENEMY43_SPEAR && f->type <= FIGURE_ENEMY_CAESAR_LEGIONARY;
}

int figure_is_legion(const figure *f)
{
    return f->type >= FIGURE_FORT_JAVELIN && f->type <= FIGURE_FORT_LEGIONARY;
}

int figure_is_herd(const figure *f)
{
    return f->type >= FIGURE_SHEEP && f->type <= FIGURE_ZEBRA;
}

static void initialize_new_figure(figure *f, int position)
{
    f->id = position;
}

static int figure_is_active(const figure *f)
{
    return f->state != 0;
}

void figure_init_scenario(void)
{
    if (!array_init(data.figures, FIGURE_ARRAY_SIZE_STEP, initialize_new_figure, figure_is_active) ||
        !array_next(data.figures)) { // Ignore first figure
        log_error("Unable to create figures array. The game will now crash.", 0, 0);
    }
    data.created_sequence = 0;
}

void figure_kill_all(void)
{
    figure *f;
    array_foreach(data.figures, f)
    {
        switch (f->type) {
            default:
                f->state = FIGURE_STATE_DEAD;
                break;
            case FIGURE_EXPLOSION:
            case FIGURE_MAP_FLAG:
            case FIGURE_FISH_GULLS:
            case FIGURE_SHIPWRECK:
            case FIGURE_FORT_STANDARD:
                continue;
        }  
    }
}

int figure_target_is_alive(const figure *f)
{
    if (f->target_figure_id <= 0) {
        return 0;
    }
    figure *target = figure_get(f->target_figure_id);
    if (!figure_is_dead(target) && target->created_sequence == f->target_figure_created_sequence) {
        return 1;
    }
    return 0;
}

static void figure_save(buffer *buf, const figure *f)
{
    buffer_write_u8(buf, f->alternative_location_index);
    buffer_write_u8(buf, f->image_offset);
    buffer_write_u8(buf, f->is_enemy_image);
    buffer_write_u8(buf, f->flotsam_visible);
    buffer_write_i16(buf, f->image_id);
    buffer_write_i16(buf, f->cart_image_id);
    buffer_write_i16(buf, f->next_figure_id_on_same_tile);
    buffer_write_u8(buf, f->type);
    buffer_write_u8(buf, f->resource_id);
    buffer_write_u8(buf, f->use_cross_country);
    buffer_write_u8(buf, f->is_friendly);
    buffer_write_u8(buf, f->state);
    buffer_write_u8(buf, f->faction_id);
    buffer_write_u8(buf, f->action_state_before_attack);
    buffer_write_i8(buf, f->direction);
    buffer_write_i8(buf, f->previous_tile_direction);
    buffer_write_i8(buf, f->attack_direction);
    buffer_write_u8(buf, f->x);
    buffer_write_u8(buf, f->y);
    buffer_write_u8(buf, f->previous_tile_x);
    buffer_write_u8(buf, f->previous_tile_y);
    buffer_write_u8(buf, f->missile_damage);
    buffer_write_u8(buf, f->damage);
    buffer_write_i16(buf, f->grid_offset);
    buffer_write_u8(buf, f->destination_x);
    buffer_write_u8(buf, f->destination_y);
    buffer_write_i16(buf, f->destination_grid_offset);
    buffer_write_u8(buf, f->source_x);
    buffer_write_u8(buf, f->source_y);
    buffer_write_u8(buf, f->formation_position_x.soldier);
    buffer_write_u8(buf, f->formation_position_y.soldier);
    buffer_write_i16(buf, f->disallow_diagonal);
    buffer_write_i16(buf, f->wait_ticks);
    buffer_write_u8(buf, f->action_state);
    buffer_write_u8(buf, f->progress_on_tile);
    buffer_write_i16(buf, f->routing_path_id);
    buffer_write_i16(buf, f->routing_path_current_tile);
    buffer_write_i16(buf, f->routing_path_length);
    buffer_write_u8(buf, f->in_building_wait_ticks);
    buffer_write_u8(buf, f->is_on_road);
    buffer_write_i16(buf, f->max_roam_length);
    buffer_write_i16(buf, f->roam_length);
    buffer_write_u8(buf, f->roam_choose_destination);
    buffer_write_u8(buf, f->roam_random_counter);
    buffer_write_i8(buf, f->roam_turn_direction);
    buffer_write_i8(buf, f->roam_ticks_until_next_turn);
    buffer_write_i16(buf, f->cross_country_x);
    buffer_write_i16(buf, f->cross_country_y);
    buffer_write_i16(buf, f->cc_destination_x);
    buffer_write_i16(buf, f->cc_destination_y);
    buffer_write_i16(buf, f->cc_delta_x);
    buffer_write_i16(buf, f->cc_delta_y);
    buffer_write_i16(buf, f->cc_delta_xy);
    buffer_write_u8(buf, f->cc_direction);
    buffer_write_u8(buf, f->speed_multiplier);
    buffer_write_i16(buf, f->building_id);
    buffer_write_i16(buf, f->immigrant_building_id);
    buffer_write_i16(buf, f->destination_building_id);
    buffer_write_i16(buf, f->formation_id);
    buffer_write_u8(buf, f->index_in_formation);
    buffer_write_u8(buf, f->formation_at_rest);
    buffer_write_u8(buf, f->migrant_num_people);
    buffer_write_u8(buf, f->is_ghost);
    buffer_write_u8(buf, f->min_max_seen);
    buffer_write_i8(buf, f->progress_to_next_tick);
    buffer_write_i16(buf, f->leading_figure_id);
    buffer_write_u8(buf, f->attack_image_offset);
    buffer_write_u8(buf, f->wait_ticks_missile);
    buffer_write_i8(buf, f->x_offset_cart);
    buffer_write_i8(buf, f->y_offset_cart);
    buffer_write_u8(buf, f->empire_city_id);
    buffer_write_u8(buf, f->trader_amount_bought);
    buffer_write_i16(buf, f->name);
    buffer_write_u8(buf, f->terrain_usage);
    buffer_write_u8(buf, f->loads_sold_or_carrying);
    buffer_write_u8(buf, f->is_boat);
    buffer_write_u8(buf, f->height_adjusted_ticks);
    buffer_write_u8(buf, f->current_height);
    buffer_write_u8(buf, f->target_height);
    buffer_write_u8(buf, f->collecting_item_id);
    buffer_write_u8(buf, f->trade_ship_failed_dock_attempts);
    buffer_write_u8(buf, f->phrase_sequence_exact);
    buffer_write_i8(buf, f->phrase_id);
    buffer_write_u8(buf, f->phrase_sequence_city);
    buffer_write_u8(buf, f->trader_id);
    buffer_write_u8(buf, f->wait_ticks_next_target);
    buffer_write_u8(buf, f->dont_draw_elevated);
    buffer_write_i16(buf, f->target_figure_id);
    buffer_write_i16(buf, f->targeted_by_figure_id);
    buffer_write_u16(buf, f->created_sequence);
    buffer_write_u16(buf, f->target_figure_created_sequence);
    buffer_write_u8(buf, f->figures_on_same_tile_index);
    buffer_write_u8(buf, f->num_attackers);
    buffer_write_i16(buf, f->attacker_id1);
    buffer_write_i16(buf, f->attacker_id2);
    buffer_write_i16(buf, f->opponent_id);
    buffer_write_i16(buf, f->last_visited_index);
}

static int get_resource_id(figure_type type, int resource)
{
    
    switch (type) {
        case FIGURE_PRIEST_SUPPLIER:
        case FIGURE_BARKEEP_SUPPLIER:
        case FIGURE_MESS_HALL_SUPPLIER:
        case FIGURE_MARKET_SUPPLIER:
        case FIGURE_CARAVANSERAI_SUPPLIER:
        case FIGURE_PRIEST:
        case FIGURE_DELIVERY_BOY:
        case FIGURE_MESS_HALL_COLLECTOR:
        case FIGURE_CARAVANSERAI_COLLECTOR:
            return resource_map_legacy_inventory(resource);
        default:
            return resource_remap(resource);
    }
}

static void figure_load(buffer *buf, figure *f, int figure_buf_size, int version)
{
    f->alternative_location_index = buffer_read_u8(buf);
    f->image_offset = buffer_read_u8(buf);
    f->is_enemy_image = buffer_read_u8(buf);
    f->flotsam_visible = buffer_read_u8(buf);
    f->image_id = buffer_read_i16(buf);
    f->cart_image_id = buffer_read_i16(buf);
    f->next_figure_id_on_same_tile = buffer_read_i16(buf);
    f->type = buffer_read_u8(buf);
    int resource = buffer_read_u8(buf);
    if (f->type == FIGURE_HIPPODROME_HORSES || f->type == FIGURE_FLOTSAM || resource < RESOURCE_NONE) {
        f->resource_id = resource;
    } else {
        f->resource_id = resource_remap(resource);
    }
    f->use_cross_country = buffer_read_u8(buf);
    f->is_friendly = buffer_read_u8(buf);
    f->state = buffer_read_u8(buf);
    f->faction_id = buffer_read_u8(buf);
    f->action_state_before_attack = buffer_read_u8(buf);
    f->direction = buffer_read_i8(buf);
    f->previous_tile_direction = buffer_read_i8(buf);
    f->attack_direction = buffer_read_i8(buf);
    f->x = buffer_read_u8(buf);
    f->y = buffer_read_u8(buf);
    f->previous_tile_x = buffer_read_u8(buf);
    f->previous_tile_y = buffer_read_u8(buf);
    f->missile_damage = buffer_read_u8(buf);
    f->damage = buffer_read_u8(buf);
    f->grid_offset = buffer_read_i16(buf);
    f->destination_x = buffer_read_u8(buf);
    f->destination_y = buffer_read_u8(buf);
    f->destination_grid_offset = buffer_read_i16(buf);
    f->source_x = buffer_read_u8(buf);
    f->source_y = buffer_read_u8(buf);
    f->formation_position_x.soldier = buffer_read_u8(buf);
    f->formation_position_y.soldier = buffer_read_u8(buf);
    f->disallow_diagonal = buffer_read_i16(buf);
    f->wait_ticks = buffer_read_i16(buf);
    f->action_state = buffer_read_u8(buf);
    f->progress_on_tile = buffer_read_u8(buf);
    f->routing_path_id = buffer_read_i16(buf);
    f->routing_path_current_tile = buffer_read_i16(buf);
    f->routing_path_length = buffer_read_i16(buf);
    f->in_building_wait_ticks = buffer_read_u8(buf);
    f->is_on_road = buffer_read_u8(buf);
    f->max_roam_length = buffer_read_i16(buf);
    f->roam_length = buffer_read_i16(buf);
    f->roam_choose_destination = buffer_read_u8(buf);
    f->roam_random_counter = buffer_read_u8(buf);
    f->roam_turn_direction = buffer_read_i8(buf);
    f->roam_ticks_until_next_turn = buffer_read_i8(buf);
    f->cross_country_x = buffer_read_i16(buf);
    f->cross_country_y = buffer_read_i16(buf);
    f->cc_destination_x = buffer_read_i16(buf);
    f->cc_destination_y = buffer_read_i16(buf);
    f->cc_delta_x = buffer_read_i16(buf);
    f->cc_delta_y = buffer_read_i16(buf);
    f->cc_delta_xy = buffer_read_i16(buf);
    f->cc_direction = buffer_read_u8(buf);
    f->speed_multiplier = buffer_read_u8(buf);
    f->building_id = buffer_read_i16(buf);
    f->immigrant_building_id = buffer_read_i16(buf);
    f->destination_building_id = buffer_read_i16(buf);
    f->formation_id = buffer_read_i16(buf);
    f->index_in_formation = buffer_read_u8(buf);
    f->formation_at_rest = buffer_read_u8(buf);
    f->migrant_num_people = buffer_read_u8(buf);
    f->is_ghost = buffer_read_u8(buf);
    f->min_max_seen = buffer_read_u8(buf);
    f->progress_to_next_tick = buffer_read_i8(buf);
    f->leading_figure_id = buffer_read_i16(buf);
    f->attack_image_offset = buffer_read_u8(buf);
    f->wait_ticks_missile = buffer_read_u8(buf);
    f->x_offset_cart = buffer_read_i8(buf);
    f->y_offset_cart = buffer_read_i8(buf);
    f->empire_city_id = buffer_read_u8(buf);
    f->trader_amount_bought = buffer_read_u8(buf);
    f->name = buffer_read_i16(buf);
    f->terrain_usage = buffer_read_u8(buf);
    f->loads_sold_or_carrying = buffer_read_u8(buf);
    f->is_boat = buffer_read_u8(buf);
    f->height_adjusted_ticks = buffer_read_u8(buf);
    f->current_height = buffer_read_u8(buf);
    f->target_height = buffer_read_u8(buf);
    f->collecting_item_id = (version <= SAVE_GAME_LAST_STATIC_RESOURCES) ?
        get_resource_id(f->type, buffer_read_u8(buf)) : resource_remap(buffer_read_u8(buf));
    f->trade_ship_failed_dock_attempts = buffer_read_u8(buf);
    f->phrase_sequence_exact = buffer_read_u8(buf);
    f->phrase_id = buffer_read_i8(buf);
    f->phrase_sequence_city = buffer_read_u8(buf);
    f->trader_id = buffer_read_u8(buf);
    f->wait_ticks_next_target = buffer_read_u8(buf);
    f->dont_draw_elevated = buffer_read_u8(buf);
    f->target_figure_id = buffer_read_i16(buf);
    f->targeted_by_figure_id = buffer_read_i16(buf);
    f->created_sequence = buffer_read_u16(buf);
    f->target_figure_created_sequence = buffer_read_u16(buf);
    f->figures_on_same_tile_index = buffer_read_u8(buf);
    f->num_attackers = buffer_read_u8(buf);
    f->attacker_id1 = buffer_read_i16(buf);
    f->attacker_id2 = buffer_read_i16(buf);
    f->opponent_id = buffer_read_i16(buf);
    if (version > SAVE_GAME_LAST_GLOBAL_BUILDING_INFO) {
        f->last_visited_index = buffer_read_i16(buf);
    }

    // The following code should only be executed if the savegame includes figure information that is not 
    // supported on this specific version of Augustus. The extra bytes in the buffer must be skipped in order
    // to prevent reading bogus data for the next figure
    if (figure_buf_size > FIGURE_CURRENT_BUFFER_SIZE) {
        buffer_skip(buf, figure_buf_size - FIGURE_CURRENT_BUFFER_SIZE);
    }
}

void figure_save_state(buffer *list, buffer *seq)
{
    buffer_write_i32(seq, data.created_sequence);

    int buf_size = 4 + data.figures.size * FIGURE_CURRENT_BUFFER_SIZE;
    uint8_t *buf_data = malloc(buf_size);
    buffer_init(list, buf_data, buf_size);
    buffer_write_i32(list, FIGURE_CURRENT_BUFFER_SIZE);

    figure *f;
    array_foreach(data.figures, f) {
        figure_save(list, f);
    }
}

void figure_load_state(buffer *list, buffer *seq, int version)
{
    data.created_sequence = buffer_read_i32(seq);

    int figure_buf_size = FIGURE_ORIGINAL_BUFFER_SIZE;
    int buf_size = list->size;

    if (version > SAVE_GAME_LAST_STATIC_VERSION) {
        figure_buf_size = buffer_read_i32(list);
        buf_size -= 4;
    }

    int figures_to_load = buf_size / figure_buf_size;

    if (!array_init(data.figures, FIGURE_ARRAY_SIZE_STEP, initialize_new_figure, figure_is_active) ||
        !array_expand(data.figures, figures_to_load)) {
        log_error("Unable to create figures array. The game will now crash.", 0, 0);
    }

    int highest_id_in_use = 0;

    for (int i = 0; i < figures_to_load; i++) {
        figure *f = array_next(data.figures);
        figure_load(list, f, figure_buf_size, version);
        if (f->state) {
            highest_id_in_use = i;
        }
    }
    data.figures.size = highest_id_in_use + 1;
}
