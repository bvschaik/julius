#ifndef FIGURE_FIGURE_H
#define FIGURE_FIGURE_H

#include "core/buffer.h"
#include "core/direction.h"
#include "figure/action.h"
#include "figure/type.h"

#define MAX_FIGURES 5000

typedef struct {
    int id;

    short image_id;
    short cart_image_id;
    unsigned char image_offset;
    unsigned char is_enemy_image;

    unsigned char alternative_location_index;
    unsigned char flotsam_visible;
    short next_figure_id_on_same_tile;
    unsigned char type;
    unsigned char resource_id;
    unsigned char use_cross_country;
    unsigned char is_friendly;
    unsigned char state;
    unsigned char faction_id; // 1 = city, 0 = enemy
    unsigned char action_state_before_attack;
    signed char direction;
    signed char previous_tile_direction;
    signed char attack_direction;
    unsigned char x;
    unsigned char y;
    unsigned char previous_tile_x;
    unsigned char previous_tile_y;
    unsigned char missile_damage;
    unsigned char damage;
    short grid_offset;
    unsigned char destination_x;
    unsigned char destination_y;
    short destination_grid_offset; // only used for soldiers
    unsigned char source_x;
    unsigned char source_y;
    union {
        unsigned char soldier;
        signed char enemy;
    } formation_position_x;
    union {
        unsigned char soldier;
        signed char enemy;
    } formation_position_y;
    short __unused_24;
    short wait_ticks;
    unsigned char action_state;
    unsigned char progress_on_tile;
    short routing_path_id;
    short routing_path_current_tile;
    short routing_path_length;
    unsigned char in_building_wait_ticks;
    unsigned char is_on_road;
    short max_roam_length;
    short roam_length;
    unsigned char roam_choose_destination;
    unsigned char roam_random_counter;
    signed char roam_turn_direction;
    signed char roam_ticks_until_next_turn;
    short cross_country_x; // position = 15 * x + offset on tile
    short cross_country_y; // position = 15 * y + offset on tile
    short cc_destination_x;
    short cc_destination_y;
    short cc_delta_x;
    short cc_delta_y;
    short cc_delta_xy;
    unsigned char cc_direction; // 1 = x, 2 = y
    unsigned char speed_multiplier;
    short building_id;
    short immigrant_building_id;
    short destination_building_id;
    short formation_id;
    unsigned char index_in_formation;
    unsigned char formation_at_rest;
    unsigned char migrant_num_people;
    unsigned char is_ghost;
    unsigned char min_max_seen;
    unsigned char __unused_57;
    short leading_figure_id;
    unsigned char attack_image_offset;
    unsigned char wait_ticks_missile;
    signed char x_offset_cart;
    signed char y_offset_cart;
    unsigned char empire_city_id;
    unsigned char trader_amount_bought;
    short name;
    unsigned char terrain_usage;
    unsigned char loads_sold_or_carrying;
    unsigned char is_boat; // 1 for boat, 2 for flotsam
    unsigned char height_adjusted_ticks;
    unsigned char current_height;
    unsigned char target_height;
    unsigned char collecting_item_id; // NOT a resource ID for cartpushers! IS a resource ID for warehousemen
    unsigned char trade_ship_failed_dock_attempts;
    unsigned char phrase_sequence_exact;
    signed char phrase_id;
    unsigned char phrase_sequence_city;
    unsigned char trader_id;
    unsigned char wait_ticks_next_target;
    unsigned char __unused_6f;
    short target_figure_id;
    short targeted_by_figure_id;
    unsigned short created_sequence;
    unsigned short target_figure_created_sequence;
    unsigned char figures_on_same_tile_index;
    unsigned char num_attackers;
    short attacker_id1;
    short attacker_id2;
    short opponent_id;
} figure;

figure *figure_get(int id);

/**
 * Creates a figure
 * @param type Figure type
 * @param x X position
 * @param y Y position
 * @param dir Direction the figure faces
 * @return Always a figure. If figure->id is zero, it is an invalid one.
 */
figure *figure_create(figure_type type, int x, int y, direction_type dir);

void figure_delete(figure *f);

int figure_is_dead(const figure *f);

int figure_is_enemy(const figure *f);

int figure_is_legion(const figure *f);

int figure_is_herd(const figure *f);

void figure_init_scenario(void);

void figure_kill_all();

void figure_save_state(buffer *list, buffer *seq);

void figure_load_state(buffer *list, buffer *seq);

#endif // FIGURE_FIGURE_H
