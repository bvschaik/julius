#ifndef FIGURE_FORMATION_H
#define FIGURE_FORMATION_H

#include "core/buffer.h"
#include "figure/type.h"

#define MAX_FORMATIONS 50

#define MAX_LEGIONS 6
#define MAX_FORMATION_FIGURES 16

enum {
    LEGION_RECRUIT_NONE = 0,
    LEGION_RECRUIT_MOUNTED = 1,
    LEGION_RECRUIT_JAVELIN = 2,
    LEGION_RECRUIT_LEGIONARY = 3
};

enum {
    FORMATION_ATTACK_FOOD_CHAIN= 0,
    FORMATION_ATTACK_GOLD_STORES = 1,
    FORMATION_ATTACK_BEST_BUILDINGS = 2,
    FORMATION_ATTACK_TROOPS = 3,
    FORMATION_ATTACK_RANDOM = 4
};

enum {
    FORMATION_COLUMN = 0,
    FORMATION_DOUBLE_LINE_1 = 1,
    FORMATION_DOUBLE_LINE_2 = 2,
    FORMATION_SINGLE_LINE_1 = 3,
    FORMATION_SINGLE_LINE_2 = 4,
    FORMATION_TORTOISE = 5,
    FORMATION_MOP_UP = 6,
    FORMATION_AT_REST = 7,
    FORMATION_ENEMY8 = 8,
    FORMATION_HERD = 9,
    FORMATION_ENEMY10 = 10,
    FORMATION_ENEMY12 = 12,
};

typedef struct {
    int duration_halt;
    int duration_advance;
    int duration_regroup;
} formation_state;

/**
 * Formation data
 */
typedef struct {
    int id; /**< ID of the formation */
    int faction_id; /**< 1 = player, 0 = everyone else */

    /* General variables */
    int in_use; /**< Flag whether this entry is in use */
    int is_herd; /**< Flag to indicate herd */
    int is_legion; /**< Flag to indicate (own) legion */
    int legion_id; /**< Legion ID (0-5 for own troops) */
    int layout;
    int direction;
    int orientation;

    int morale;
    int months_from_home;
    int months_low_morale;
    int months_very_low_morale;
    
    /* Figures */
    int figure_type; /**< Type of figure in this formation */
    int num_figures; /**< Current number of figures in the formation */
    int max_figures; /**< Maximum number of figures */
    int figures[MAX_FORMATION_FIGURES]; /**< Figure IDs */
    int total_damage; /**< Total damage of all figures added */
    int max_total_damage; /**< Maximum total damage of all figures added */
    
    /* Position */
    int x;
    int y;
    int x_home;
    int y_home;
    int building_id;
    int standard_x;
    int standard_y;
    int standard_figure_id;
    int destination_x;
    int destination_y;
    int destination_building_id;
    
    /* Movement */
    int wait_ticks;
    int is_halted;
    int recent_fight;
    int unknown_fired;
    int missile_fired;
    int missile_attack_timeout;
    int missile_attack_formation_id;

    /* Legion-related */
    int empire_service; /**< Flag to indicate this legion is selected for empire service */
    int in_distant_battle; /**< Flag to indicate this legion is away in a distant battle */
    int cursed_by_mars; /**< Flag to indicate this legion is cursed */
    int has_military_training; /**< Flag to indicate this legion has had military training */
    int legion_recruit_type; /**< Recruit type: none if this legion is fully occupied */
    int is_at_fort; /**< Flag to indicate this legion is resting at the fort */
    
    /* Enemy-related */
    int enemy_type;
    int enemy_legion_index;
    int attack_type;
    int invasion_id;
    int invasion_sequence;
    formation_state enemy_state;
    
    /* Herd-related */
    int herd_direction;
    int herd_wolf_spawn_delay;
    
    struct {
        int layout;
        int x_home;
        int y_home;
    } prev;
} formation;

void formations_clear();

void formation_clear(int formation_id);

formation *formation_create_legion(int building_id, int x, int y, figure_type figure_type);
int formation_create_herd(int figure_type, int x, int y, int num_animals);
int formation_create_enemy(int figure_type, int x, int y, int layout, int orientation,
                           int enemy_type, int attack_type, int invasion_id, int invasion_sequence);

formation *formation_get(int formation_id);
formation_state *formation_get_state(int formation_id);

void formation_set_halted(int formation_id, int halted);
void formation_set_distant_battle(int formation_id, int distant_battle);
void formation_set_at_fort(int formation_id, int at_fort);
void formation_set_cursed(int formation_id);

void formation_change_layout(int formation_id, int new_layout);

void formation_restore_layout(formation *m);

void formation_toggle_empire_service(int formation_id);

void formation_record_missile_fired(int formation_id);
void formation_record_missile_attack(int formation_id, int from_formation_id);
void formation_record_fight(int formation_id);

int formation_for_invasion(int invasion_sequence);

void formation_caesar_pause();

void formation_caesar_retreat();

void formation_foreach(void (*callback)(const formation*));
void formation_foreach_herd(void (*callback)(const formation*));
void formation_foreach_non_herd(void (*callback)(const formation*, void*), void *data);
void formation_foreach_legion(void (*callback)(const formation*, void*), void *data);

void formation_legion_set_trained(int formation_id);
void formation_legion_set_max_figures();

int formation_totals_get_num_legions();
void formation_totals_clear_legions();
void formation_totals_add_legion(int formation_id);

int formation_get_num_legions();

int formation_for_legion(int legion_index);

void formation_change_morale(int formation_id, int amount);
int formation_has_low_morale(int formation_id);

void formation_update_monthly_morale_deployed();
void formation_update_monthly_morale_at_rest();
void formation_decrease_monthly_counters(int formation_id);
void formation_clear_monthly_counters(int formation_id);

void formation_set_destination(int formation_id, int x, int y);
void formation_set_destination_building(int formation_id, int x, int y, int building_id);
void formation_set_home(int formation_id, int x, int y);

void formation_clear_figures();
int formation_add_figure(int formation_id, int figure_id, int deployed, int damage, int max_damage);

void formation_move_herds_away(int x, int y);
int formation_can_spawn_wolf(int formation_id);
void formation_herd_clear_direction(int formation_id);

void formation_increase_wait_ticks(int formation_id);
void formation_reset_wait_ticks(int formation_id);

void formation_set_enemy_legion(int formation_id, int enemy_legion_index);

void formation_update_direction(int formation_id, int first_figure_direction);


void formations_save_state(buffer *buf, buffer *totals);
void formations_load_state(buffer *buf, buffer *totals);

#endif // FIGURE_FORMATION_H
