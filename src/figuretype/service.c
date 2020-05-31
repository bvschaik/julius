#include "service.h"

#include "building/building.h"
#include "building/market.h"
#include "core/config.h"
#include "core/image.h"
#include "figure/combat.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "map/building.h"
#include "map/road_access.h"

static void roamer_action(figure *f, int num_ticks)
{
    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_125_ROAMING:
            f->is_ghost = 0;
            f->roam_length++;
            if (f->roam_length >= f->max_roam_length) {
                int x, y;
                building *b = building_get(f->building_id);
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x, &y)) {
                    f->action_state = FIGURE_ACTION_126_ROAMER_RETURNING;
                    f->destination_x = x;
                    f->destination_y = y;
                    figure_route_remove(f);
                    f->roam_length = 0;
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            figure_movement_roam_ticks(f, num_ticks);
            break;
        case FIGURE_ACTION_126_ROAMER_RETURNING:
            figure_movement_move_ticks(f, num_ticks);
            if (f->direction == DIR_FIGURE_AT_DESTINATION ||
                f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
    }
}

static void culture_action(figure *f, int group)
{
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    f->max_roam_length = 384;
    building *b = building_get(f->building_id);
    if (b->state != BUILDING_STATE_IN_USE || b->figure_id != f->id) {
        f->state = FIGURE_STATE_DEAD;
    }
    figure_image_increase_offset(f, 12);
    roamer_action(f, 1);
    figure_image_update(f, image_group(group));
}

void figure_priest_action(figure *f)
{
    culture_action(f, GROUP_FIGURE_PRIEST);
}

void figure_school_child_action(figure *f)
{
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    if (config_get(CONFIG_GP_CH_SCHOOL_WALKERS)) {
        f->max_roam_length = 192;
    }
    else {
        f->max_roam_length = 96;
    }
    building *b = building_get(f->building_id);
    if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_SCHOOL) {
        f->state = FIGURE_STATE_DEAD;
    }
    figure_image_increase_offset(f, 12);
    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_125_ROAMING:
            f->is_ghost = 0;
            f->roam_length++;
            if (f->roam_length >= f->max_roam_length) {
                f->state = FIGURE_STATE_DEAD;
            }
            figure_movement_roam_ticks(f, 2);
            break;
    }
    figure_image_update(f, image_group(GROUP_FIGURE_SCHOOL_CHILD));
}

void figure_teacher_action(figure *f)
{
    culture_action(f, GROUP_FIGURE_TEACHER_LIBRARIAN);
}

void figure_librarian_action(figure *f)
{
    culture_action(f, GROUP_FIGURE_TEACHER_LIBRARIAN);
}

void figure_barber_action(figure *f)
{
    culture_action(f, GROUP_FIGURE_BARBER);
}

void figure_bathhouse_worker_action(figure *f)
{
    culture_action(f, GROUP_FIGURE_BATHHOUSE_WORKER);
}

void figure_doctor_action(figure *f)
{
    culture_action(f, GROUP_FIGURE_DOCTOR_SURGEON);
}

void figure_missionary_action(figure *f)
{
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    f->max_roam_length = 192;
    building *b = building_get(f->building_id);
    if (b->state != BUILDING_STATE_IN_USE || b->figure_id != f->id) {
        f->state = FIGURE_STATE_DEAD;
    }
    figure_image_increase_offset(f, 12);
    roamer_action(f, 1);
    figure_image_update(f, image_group(GROUP_FIGURE_MISSIONARY));
}

void figure_patrician_action(figure *f)
{
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    f->max_roam_length = 128;
    if (building_get(f->building_id)->state != BUILDING_STATE_IN_USE) {
        f->state = FIGURE_STATE_DEAD;
    }
    figure_image_increase_offset(f, 12);
    roamer_action(f, 1);
    figure_image_update(f, image_group(GROUP_FIGURE_PATRICIAN));
}

void figure_labor_seeker_action(figure *f)
{
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    f->max_roam_length = 384;
    building *b = building_get(f->building_id);
    if (b->state != BUILDING_STATE_IN_USE || b->figure_id2 != f->id) {
        f->state = FIGURE_STATE_DEAD;
    }
    figure_image_increase_offset(f, 12);
    roamer_action(f, 1);
    figure_image_update(f, image_group(GROUP_FIGURE_LABOR_SEEKER));
}

void figure_market_trader_action(figure *f)
{
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    f->max_roam_length = 384;
    building *market = building_get(f->building_id);
    if (market->state != BUILDING_STATE_IN_USE || market->figure_id != f->id) {
        f->state = FIGURE_STATE_DEAD;
    }
    figure_image_increase_offset(f, 12);
    if (f->action_state == FIGURE_ACTION_125_ROAMING) {
        // force return on out of stock
        int stock = building_market_get_max_food_stock(market) +
            building_market_get_max_goods_stock(market);
        if (f->roam_length >= 96 && stock <= 0) {
            f->roam_length = f->max_roam_length;
        }
    }
    roamer_action(f, 1);
    figure_image_update(f, image_group(GROUP_FIGURE_MARKET_LADY));
}

void figure_tax_collector_action(figure *f)
{
    building *b = building_get(f->building_id);

    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    f->max_roam_length = 512;
    if (b->state != BUILDING_STATE_IN_USE || b->figure_id != f->id) {
        f->state = FIGURE_STATE_DEAD;
    }
    figure_image_increase_offset(f, 12);

    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_40_TAX_COLLECTOR_CREATED:
            f->is_ghost = 1;
            f->image_offset = 0;
            f->wait_ticks--;
            if (f->wait_ticks <= 0) {
                int x_road, y_road;
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                    f->action_state = FIGURE_ACTION_41_TAX_COLLECTOR_ENTERING_EXITING;
                    figure_movement_set_cross_country_destination(f, x_road, y_road);
                    f->roam_length = 0;
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            break;
        case FIGURE_ACTION_41_TAX_COLLECTOR_ENTERING_EXITING:
            f->use_cross_country = 1;
            f->is_ghost = 1;
            if (figure_movement_move_ticks_cross_country(f, 1) == 1) {
                if (map_building_at(f->grid_offset) == f->building_id) {
                    // returned to own building
                    f->state = FIGURE_STATE_DEAD;
                } else {
                    f->action_state = FIGURE_ACTION_42_TAX_COLLECTOR_ROAMING;
                    figure_movement_init_roaming(f);
                    f->roam_length = 0;
                }
            }
            break;
        case FIGURE_ACTION_42_TAX_COLLECTOR_ROAMING:
            f->is_ghost = 0;
            f->roam_length++;
            if (f->roam_length >= f->max_roam_length) {
                int x_road, y_road;
                if (map_closest_road_within_radius(b->x, b->y, b->size, 2, &x_road, &y_road)) {
                    f->action_state = FIGURE_ACTION_43_TAX_COLLECTOR_RETURNING;
                    f->destination_x = x_road;
                    f->destination_y = y_road;
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            figure_movement_roam_ticks(f, 1);
            break;
        case FIGURE_ACTION_43_TAX_COLLECTOR_RETURNING:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_41_TAX_COLLECTOR_ENTERING_EXITING;
                figure_movement_set_cross_country_destination(f, b->x, b->y);
                f->roam_length = 0;
            } else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
    }
    figure_image_update(f, image_group(GROUP_FIGURE_TAX_COLLECTOR));
}
