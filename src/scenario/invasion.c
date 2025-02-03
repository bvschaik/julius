#include "invasion.h"

#include "building/destruction.h"
#include "core/array.h"
#include "city/emperor.h"
#include "city/message.h"
#include "core/calc.h"
#include "core/log.h"
#include "core/random.h"
#include "empire/object.h"
#include "figure/enemy_army.h"
#include "figure/figure.h"
#include "figure/formation.h"
#include "figure/name.h"
#include "game/difficulty.h"
#include "game/time.h"
#include "map/grid.h"
#include "map/terrain.h"
#include "scenario/data.h"
#include "scenario/map.h"
#include "scenario/property.h"

#include <string.h>

#define MAX_ORIGINAL_INVASION_WARNINGS 101

#define INVASIONS_ARRAY_SIZE_STEP 16
#define WARNINGS_ARRAY_SIZE_STEP 32

#define INVASIONS_STRUCT_SIZE_CURRENT (5 * sizeof(int16_t) + 4 * sizeof(uint16_t) + 1 * sizeof(uint8_t))
#define WARNINGS_STRUCT_SIZE_CURRENT (1 * sizeof(int32_t) + 6 * sizeof(int16_t) + 5 * sizeof(uint8_t))

#define BARBARIAN_ENEMY_TYPE_MAX 4

#define CHEATED_ARMY_ID 23
#define CAESAR_ATTACK_ARMY_ID 24
#define ACTION_ARMY_ID_START 25

static const int ENEMY_ID_TO_ENEMY_TYPE[20] = {
    ENEMY_0_BARBARIAN,
    ENEMY_7_ETRUSCAN,
    ENEMY_7_ETRUSCAN,
    ENEMY_10_CARTHAGINIAN,
    ENEMY_8_GREEK,
    ENEMY_8_GREEK,
    ENEMY_9_EGYPTIAN,
    ENEMY_5_PERGAMUM,
    ENEMY_6_SELEUCID,
    ENEMY_3_CELT,
    ENEMY_3_CELT,
    ENEMY_3_CELT,
    ENEMY_2_GAUL,
    ENEMY_2_GAUL,
    ENEMY_4_GOTH,
    ENEMY_4_GOTH,
    ENEMY_4_GOTH,
    ENEMY_6_SELEUCID,
    ENEMY_1_NUMIDIAN,
    ENEMY_6_SELEUCID
};

static const int LOCAL_UPRISING_NUM_ENEMIES[20] = {
    0, 0, 0, 0, 0, 3, 3, 3, 0, 6, 6, 6, 6, 6, 9, 9, 9, 9, 9, 9
};

static const struct {
    int pct_type1;
    int pct_type2;
    int pct_type3;
    int figure_types[3];
    int formation_layout;
} ENEMY_PROPERTIES[12] = {
    {100, 0, 0, {FIGURE_ENEMY49_FAST_SWORD, 0, 0}, FORMATION_ENEMY_MOB}, // barbarian
    {40, 60, 0, {FIGURE_ENEMY49_FAST_SWORD, FIGURE_ENEMY51_SPEAR, 0}, FORMATION_ENEMY_MOB}, // numidian
    {50, 50, 0, {FIGURE_ENEMY50_SWORD, FIGURE_ENEMY53_AXE, 0}, FORMATION_ENEMY_MOB}, // gaul
    {80, 20, 0, {FIGURE_ENEMY50_SWORD, FIGURE_ENEMY48_CHARIOT, 0}, FORMATION_ENEMY_MOB}, // celt
    {50, 50, 0, {FIGURE_ENEMY49_FAST_SWORD, FIGURE_ENEMY52_MOUNTED_ARCHER, 0}, FORMATION_ENEMY_MOB}, // goth
    {30, 70, 0, {FIGURE_ENEMY44_SWORD, FIGURE_ENEMY43_SPEAR, 0}, FORMATION_COLUMN}, // pergamum
    {50, 50, 0, {FIGURE_ENEMY44_SWORD, FIGURE_ENEMY43_SPEAR, 0}, FORMATION_ENEMY_DOUBLE_LINE}, // seleucid
    {50, 50, 0, {FIGURE_ENEMY45_SWORD, FIGURE_ENEMY43_SPEAR, 0}, FORMATION_ENEMY_DOUBLE_LINE}, // etruscan
    {80, 20, 0, {FIGURE_ENEMY45_SWORD, FIGURE_ENEMY43_SPEAR, 0}, FORMATION_ENEMY_DOUBLE_LINE}, // greek
    {80, 20, 0, {FIGURE_ENEMY44_SWORD, FIGURE_ENEMY46_CAMEL, 0}, FORMATION_ENEMY_WIDE_COLUMN}, // egyptian
    {90, 10, 0, {FIGURE_ENEMY45_SWORD, FIGURE_ENEMY47_ELEPHANT, 0}, FORMATION_ENEMY_WIDE_COLUMN}, // carthaginian
    {100, 0, 0, {FIGURE_ENEMY_CAESAR_LEGIONARY, 0, 0}, FORMATION_COLUMN} // caesar
};

typedef struct {
    unsigned int id;
    int in_use;
    int handled;
    int invasion_path_id;
    int warning_years;
    int x;
    int y;
    int image_id;
    int empire_object_id;
    int year_notified;
    int month_notified;
    int months_to_go;
    int invasion_id;
} invasion_warning;

static struct {
    int last_internal_invasion_id;
    int last_action_army_id;
    array(invasion_t) invasions;
    array(invasion_warning) warnings;
} data;


static void new_invasion(invasion_t *invasion, unsigned int index)
{
    invasion->id = index;
}

static int invasion_in_use(const invasion_t *invasion)
{
    return invasion->type != INVASION_TYPE_NONE;
}

static void new_warning(invasion_warning *warning, unsigned int index)
{
    warning->id = index;
}

static int warning_in_use(const invasion_warning *warning)
{
    return warning->in_use;
}

static int clear_warnings(void)
{
    return array_init(data.warnings, WARNINGS_ARRAY_SIZE_STEP, new_warning, warning_in_use);
}

void scenario_invasion_clear(void)
{
    if (!array_init(data.invasions, INVASIONS_ARRAY_SIZE_STEP, new_invasion, invasion_in_use) || !clear_warnings()) {
        log_error("Error creating invasions array - not enough memory. The game will now crash.", 0, 0);
    }
}

static void init_warnings(void)
{
    if (!clear_warnings()) {
        log_error("Error creating warnings array - not enough memory. The game will likely crash.", 0, 0);
        return;
    }
    array_advance(data.warnings);
    int path_current = 1;
    int path_max = empire_object_get_max_invasion_path();
    if (path_max == 0) {
        return;
    }
    const invasion_t *invasion;
    array_foreach(data.invasions, invasion) {
        if (!invasion->type) {
            continue;
        }
        if (invasion->type == INVASION_TYPE_LOCAL_UPRISING ||
            invasion->type == INVASION_TYPE_DISTANT_BATTLE) {
            continue;
        }
        for (int year = 1; year < 8; year++) {
            const empire_object *obj = empire_object_get_battle_icon(path_current, year);
            if (!obj) {
                continue;
            }
            invasion_warning *warning = array_advance(data.warnings);
            if (!warning) {
                log_error("Error expanding warning array - not enough memory. The game will probably crash.", 0, 0);
                return;
            }
            warning->in_use = 1;
            warning->invasion_path_id = obj->invasion_path_id;
            warning->warning_years = obj->invasion_years;
            warning->x = obj->x;
            warning->y = obj->y;
            warning->image_id = obj->image_id;
            warning->invasion_id = invasion->id;
            warning->empire_object_id = obj->id;
            warning->month_notified = 0;
            warning->year_notified = 0;
            warning->months_to_go = 12 * invasion->year;
            warning->months_to_go += invasion->month;
            warning->months_to_go -= 12 * year;
            ++warning;
        }
        path_current++;
        if (path_current > path_max) {
            path_current = 1;
        }
    }
}

void scenario_invasion_init(void)
{
    invasion_t *invasion;
    array_foreach(data.invasions, invasion) {
        random_generate_next();
        if (!invasion->type) {
            continue;
        }
        invasion->month = 2 + (random_byte() & 7);
    }
    init_warnings();
}

int scenario_invasion_new(void)
{
    invasion_t *invasion;
    array_new_item(data.invasions, invasion);
    return invasion ? invasion->id : -1;
}

const invasion_t *scenario_invasion_get(int id)
{
    return array_item(data.invasions, id);
}

void scenario_invasion_update(const invasion_t *invasion)
{
    invasion_t *base_invasion = array_item(data.invasions, invasion->id);
    *base_invasion = *invasion;
    array_trim(data.invasions);
}

void scenario_invasion_delete(int id)
{
    invasion_t *invasion = array_item(data.invasions, id);
    memset(invasion, 0, sizeof(invasion_t));
    array_trim(data.invasions);
}

int scenario_invasion_exists_upcoming(void)
{
    const invasion_warning *warning;
    array_foreach(data.warnings, warning) {
        if (warning->in_use && !warning->handled) {
            return 1;
        }
    }
    return 0;
}

int scenario_invasion_get_years_remaining(void)
{
    int years_until_invasion = 4;
    const invasion_warning *warning;
    array_foreach(data.warnings, warning){
        if (warning->in_use && warning->handled && warning->warning_years < years_until_invasion) {
            years_until_invasion = warning->warning_years;
        }
    }
    return years_until_invasion != 4 ? years_until_invasion : 0;
}

void scenario_invasion_foreach_warning(void (*callback)(int x, int y, int image_id))
{
    const invasion_warning *warning;
    array_foreach(data.warnings, warning) {
        if (warning->in_use && warning->handled) {
            callback(warning->x, warning->y, warning->image_id);
        }
    }
}

int scenario_invasion_count_total(void)
{
    return data.invasions.size;
}

int scenario_invasion_count_active(void)
{
    int num_invasions = 0;
    const invasion_t *invasion;
    array_foreach(data.invasions, invasion) {
        if (invasion->type) {
            num_invasions++;
        }
    }
    return num_invasions;
}

static void determine_formations(int num_soldiers, int *num_formations, int soldiers_per_formation[])
{
    if (num_soldiers > 0) {
        if (num_soldiers <= 16) {
            *num_formations = 1;
            soldiers_per_formation[0] = num_soldiers;
        } else if (num_soldiers <= 32) {
            *num_formations = 2;
            soldiers_per_formation[1] = num_soldiers / 2;
            soldiers_per_formation[0] = num_soldiers - num_soldiers / 2;
        } else {
            *num_formations = 3;
            soldiers_per_formation[2] = num_soldiers / 3;
            soldiers_per_formation[1] = num_soldiers / 3;
            soldiers_per_formation[0] = num_soldiers - 2 * (num_soldiers / 3);
        }
    }
}

static int start_invasion(int enemy_type, int amount, int invasion_point, formation_attack_enum attack_type, int invasion_id)
{
    if (amount <= 0) {
        return -1;
    }
    int formations_per_type[3];
    int soldiers_per_formation[3][4];
    int x, y;
    int orientation;

    amount = difficulty_adjust_enemies(amount);
    if (amount >= 150) {
        amount = 150;
    }
    data.last_internal_invasion_id++;
    if (data.last_internal_invasion_id > 32000) {
        data.last_internal_invasion_id = 1;
    }
    // calculate soldiers per type
    int num_type1 = calc_adjust_with_percentage(amount, ENEMY_PROPERTIES[enemy_type].pct_type1);
    int num_type2 = calc_adjust_with_percentage(amount, ENEMY_PROPERTIES[enemy_type].pct_type2);
    int num_type3 = calc_adjust_with_percentage(amount, ENEMY_PROPERTIES[enemy_type].pct_type3);
    num_type1 += amount - (num_type1 + num_type2 + num_type3); // assign leftovers to type1

    for (int t = 0; t < 3; t++) {
        formations_per_type[t] = 0;
        for (int f = 0; f < 4; f++) {
            soldiers_per_formation[t][f] = 0;
        }
    }

    // calculate number of formations
    determine_formations(num_type1, &formations_per_type[0], soldiers_per_formation[0]);
    determine_formations(num_type2, &formations_per_type[1], soldiers_per_formation[1]);
    determine_formations(num_type3, &formations_per_type[2], soldiers_per_formation[2]);

    // determine invasion point
    if (enemy_type == ENEMY_11_CAESAR) {
        map_point entry_point = scenario_map_entry();
        x = entry_point.x;
        y = entry_point.y;
    } else {
        // Determinate maximum number of valid invasion points.
        // Strip out invalid invasion points from the end of the list.
        int num_points = MAX_INVASION_POINTS;
        for (int i = MAX_INVASION_POINTS - 1; i >= 0; i--) {
            if (scenario.invasion_points[i].x == -1) {
                num_points--;
            } else if (num_points != MAX_INVASION_POINTS) {
                break;
            }
        }
        if (invasion_point == MAX_INVASION_POINTS) { // random
            if (num_points <= 2) {
                invasion_point = random_byte() & 1;
            } else if (num_points <= 4) {
                invasion_point = random_byte() & 3;
            } else {
                invasion_point = random_byte() & 7;
            }
        }
        if (num_points > 0) {
            while (scenario.invasion_points[invasion_point].x == -1) {
                invasion_point++;
                if (invasion_point >= MAX_INVASION_POINTS) {
                    invasion_point = 0;
                }
            }
        }
        x = scenario.invasion_points[invasion_point].x;
        y = scenario.invasion_points[invasion_point].y;
    }
    if (x == -1 || y == -1) {
        map_point exit_point = scenario_map_exit();
        x = exit_point.x;
        y = exit_point.y;
    }
    // determine orientation
    if (y == 0) {
        orientation = DIR_4_BOTTOM;
    } else if (y >= scenario.map.height - 1) {
        orientation = DIR_0_TOP;
    } else if (x == 0) {
        orientation = DIR_2_RIGHT;
    } else if (x >= scenario.map.width - 1) {
        orientation = DIR_6_LEFT;
    } else {
        orientation = DIR_4_BOTTOM;
    }
    // check terrain
    int grid_offset = map_grid_offset(x, y);
    if (map_terrain_is(grid_offset, TERRAIN_ELEVATION | TERRAIN_ROCK | TERRAIN_TREE)) {
        return -1;
    }
    if (map_terrain_is(grid_offset, TERRAIN_WATER)) {
        if (!map_terrain_is(grid_offset, TERRAIN_ROAD)) { // bridge
            return -1;
        }
    } else if (map_terrain_is(grid_offset, TERRAIN_BUILDING | TERRAIN_AQUEDUCT | TERRAIN_GATEHOUSE | TERRAIN_WALL)) {
        building_destroy_by_enemy(x, y, grid_offset);
    }
    // spawn the lot!
    int seq = 0;
    for (int type_id = 0; type_id < 3; type_id++) {
        if (formations_per_type[type_id] <= 0) {
            continue;
        }
        figure_type type = ENEMY_PROPERTIES[enemy_type].figure_types[type_id];
        for (int i = 0; i < formations_per_type[type_id]; i++) {
            int formation_id = formation_create_enemy(
                type, x, y, ENEMY_PROPERTIES[enemy_type].formation_layout, orientation,
                enemy_type, attack_type, invasion_id, data.last_internal_invasion_id
            );
            if (formation_id <= 0) {
                continue;
            }
            for (int fig = 0; fig < soldiers_per_formation[type_id][i]; fig++) {
                figure *f = figure_create(type, x, y, orientation);
                f->faction_id = 0;
                f->is_friendly = 0;
                f->action_state = FIGURE_ACTION_151_ENEMY_INITIAL;
                // TODO: should we adjust wait ticks to make enemy camping harder?
                f->wait_ticks = 200 * seq + 10 * fig + 10;
                f->formation_id = formation_id;
                f->name = figure_name_get(type, enemy_type);
                f->is_ghost = 1;
            }
            seq++;
        }
    }
    return grid_offset;
}

static void repeat_invasion_without_warnings(invasion_t *invasion)
{
    if (invasion->repeat.times != INVASIONS_REPEAT_INFINITE) {
        invasion->repeat.times--;
    }
    int years = random_between_from_stdlib(invasion->repeat.interval.min, invasion->repeat.interval.max);

    invasion->year += years;
    invasion->month = 2 + (random_from_stdlib() & 7);

    int grid_offset = start_invasion(ENEMY_ID_TO_ENEMY_TYPE[invasion->from],
        random_between_from_stdlib(invasion->amount.min, invasion->amount.max),
        invasion->from, invasion->attack_type, invasion->id);
    if (grid_offset > 0) {
        city_message_post(1, MESSAGE_ENEMY_ARMY_ATTACK, data.last_internal_invasion_id, grid_offset);
    }
}

static void repeat_invasion_with_warnings(invasion_t *invasion)
{
    repeat_invasion_without_warnings(invasion);

    invasion_warning *warning;
    array_foreach(data.warnings, warning) {
        if (warning->invasion_id != invasion->id) {
            continue;
        }
        warning->in_use = 1;
        warning->handled = 0;
        warning->month_notified = 0;
        warning->year_notified = 0;
        warning->months_to_go = 12 * invasion->year;
        warning->months_to_go += invasion->month;
        warning->months_to_go -= 12 * warning->warning_years;
        if (warning->warning_years > 1) {
            warning->months_to_go++;  // later warnings haven't been handled by scenario_invasion_process, so we need to add a month
        }
    }
}

void scenario_invasion_process(void)
{
    int enemy_id = scenario.enemy_id;
    invasion_warning *warning;
    array_foreach(data.warnings, warning) {
        if (!warning->in_use) {
            continue;
        }
        // update warnings
        warning->months_to_go--;
        if (warning->months_to_go <= 0) {
            if (warning->handled != 1) {
                warning->handled = 1;
                warning->year_notified = game_time_year();
                warning->month_notified = game_time_month();
                if (warning->warning_years > 2) {
                    city_message_post(0, MESSAGE_DISTANT_BATTLE, 0, 0);
                } else if (warning->warning_years > 1) {
                    city_message_post(0, MESSAGE_ENEMIES_CLOSING, 0, 0);
                } else {
                    city_message_post(0, MESSAGE_ENEMIES_AT_THE_DOOR, 0, 0);
                }
            }
        }
        invasion_t *invasion = array_item(data.invasions, warning->invasion_id);
        if (game_time_year() >= scenario.start_year + invasion->year &&
            game_time_month() >= invasion->month) {
            // invasion attack time has passed
            warning->in_use = 0;
            if (warning->warning_years > 1) {
                continue;
            }
            int random_amount = random_between_from_stdlib(invasion->amount.min, invasion->amount.max);
            // enemy invasions
            if (invasion->type == INVASION_TYPE_ENEMY_ARMY) {
                int grid_offset = start_invasion(ENEMY_ID_TO_ENEMY_TYPE[enemy_id],
                    random_amount, invasion->from, invasion->attack_type, warning->invasion_id);
                if (grid_offset > 0) {
                    if (ENEMY_ID_TO_ENEMY_TYPE[enemy_id] > BARBARIAN_ENEMY_TYPE_MAX) {
                        city_message_post(1, MESSAGE_ENEMY_ARMY_ATTACK, data.last_internal_invasion_id, grid_offset);
                    } else {
                        city_message_post(1, MESSAGE_BARBARIAN_ATTACK, data.last_internal_invasion_id, grid_offset);
                    }
                }
            }
            if (invasion->type == INVASION_TYPE_CAESAR) {
                int grid_offset = start_invasion(ENEMY_11_CAESAR,
                    random_amount, invasion->from, invasion->attack_type, warning->invasion_id);
                if (grid_offset > 0) {
                    city_message_post(1, MESSAGE_CAESAR_ARMY_ATTACK, data.last_internal_invasion_id, grid_offset);
                }
            }
            if (invasion->repeat.times != 0) {
                repeat_invasion_with_warnings(invasion);
            }
        }
    }
    // local uprisings
    invasion_t *invasion;
    array_foreach(data.invasions, invasion){
        if (invasion->type == INVASION_TYPE_LOCAL_UPRISING) {
            if (game_time_year() == scenario.start_year + invasion->year && game_time_month() == invasion->month) {
                int grid_offset = start_invasion(ENEMY_0_BARBARIAN,
                    random_between_from_stdlib(invasion->amount.min, invasion->amount.max),
                    invasion->from, invasion->attack_type, invasion->id);
                if (grid_offset > 0) {
                    city_message_post(1, MESSAGE_LOCAL_UPRISING, data.last_internal_invasion_id, grid_offset);
                }
            }
            if (invasion->repeat.times != 0) {
                repeat_invasion_without_warnings(invasion);
            }
        }
    }
}

int scenario_invasion_start_from_mars(void)
{
    int mission = scenario_campaign_mission();
    if (mission < 0 || mission > 19) {
        return 0;
    }
    int amount = LOCAL_UPRISING_NUM_ENEMIES[mission];
    if (amount <= 0) {
        return 0;
    }
    int grid_offset = start_invasion(ENEMY_0_BARBARIAN, amount, 8, FORMATION_ATTACK_FOOD_CHAIN, CHEATED_ARMY_ID);
    if (grid_offset) {
        city_message_post(1, MESSAGE_LOCAL_UPRISING_MARS, data.last_internal_invasion_id, grid_offset);
    }
    return 1;
}

int scenario_invasion_start_from_caesar(int size)
{
    int grid_offset = start_invasion(ENEMY_11_CAESAR, size, 0, FORMATION_ATTACK_BEST_BUILDINGS, CAESAR_ATTACK_ARMY_ID);
    if (grid_offset > 0) {
        city_message_post(1, MESSAGE_CAESAR_ARMY_ATTACK, data.last_internal_invasion_id, grid_offset);
        return 1;
    }
    return 0;
}

void scenario_invasion_start_from_cheat(void)
{
    int enemy_id = scenario.enemy_id;
    int grid_offset = start_invasion(ENEMY_ID_TO_ENEMY_TYPE[enemy_id], 150, 8,
        FORMATION_ATTACK_FOOD_CHAIN, CHEATED_ARMY_ID);
    if (grid_offset) {
        if (ENEMY_ID_TO_ENEMY_TYPE[enemy_id] > BARBARIAN_ENEMY_TYPE_MAX) {
            city_message_post(1, MESSAGE_ENEMY_ARMY_ATTACK, data.last_internal_invasion_id, grid_offset);
        } else {
            city_message_post(1, MESSAGE_BARBARIAN_ATTACK, data.last_internal_invasion_id, grid_offset);
        }
    }
}

void scenario_invasion_start_from_action(invasion_type_enum invasion_type, int size, int invasion_point,
    formation_attack_enum attack_type, enemy_type_t enemy_id)
{
    if (attack_type > FORMATION_ATTACK_RANDOM) {
        attack_type = FORMATION_ATTACK_RANDOM;
    }
    data.last_action_army_id++;
    if (data.last_action_army_id < ACTION_ARMY_ID_START || data.last_action_army_id >= MAX_ENEMY_ARMIES) {
        data.last_action_army_id = ACTION_ARMY_ID_START;
    }
    switch (invasion_type) {
        case INVASION_TYPE_ENEMY_ARMY:
        {
            if (enemy_id <= ENEMY_UNDEFINED) {
                enemy_id = scenario.enemy_id;
            }
            enemy_army_clear(data.last_action_army_id);
            int grid_offset = start_invasion(ENEMY_ID_TO_ENEMY_TYPE[enemy_id], size,
                invasion_point, attack_type, data.last_action_army_id);
            if (grid_offset) {
                if (ENEMY_ID_TO_ENEMY_TYPE[enemy_id] > BARBARIAN_ENEMY_TYPE_MAX) {
                    city_message_post(1, MESSAGE_ENEMY_ARMY_ATTACK, data.last_internal_invasion_id, grid_offset);
                } else {
                    city_message_post(1, MESSAGE_BARBARIAN_ATTACK, data.last_internal_invasion_id, grid_offset);
                }
            }
            break;
        }
        case INVASION_TYPE_CAESAR:
        {
            city_emperor_force_attack(size);
            break;
        }
        case INVASION_TYPE_LOCAL_UPRISING:
        {
            if (enemy_id <= ENEMY_UNDEFINED) {
                enemy_id = ENEMY_0_BARBARIAN;
            }
            enemy_army_clear(data.last_action_army_id);
            int grid_offset = start_invasion(ENEMY_ID_TO_ENEMY_TYPE[enemy_id], size,
                invasion_point, attack_type, data.last_action_army_id);
            if (grid_offset) {
                city_message_post(1, MESSAGE_LOCAL_UPRISING, data.last_internal_invasion_id, grid_offset);
            }
            break;
        }
        case INVASION_TYPE_MARS_NATIVES:
        {
            enemy_army_clear(data.last_action_army_id);
            int grid_offset = start_invasion(ENEMY_0_BARBARIAN, size, 8, attack_type, data.last_action_army_id);
            if (grid_offset) {
                city_message_post(1, MESSAGE_LOCAL_UPRISING_MARS, data.last_internal_invasion_id, grid_offset);
            }
            break;
        }
        default:
            break;
    }
}

void scenario_invasion_start_from_console(invasion_type_enum invasion_type, int size, int invasion_point)
{
    int attack_type = FORMATION_ATTACK_RANDOM;
    int enemy_id = scenario.enemy_id;
    switch (invasion_type) {
        case INVASION_TYPE_ENEMY_ARMY:
            attack_type = FORMATION_ATTACK_RANDOM;
            break;
        case INVASION_TYPE_CAESAR:
            attack_type = FORMATION_ATTACK_BEST_BUILDINGS;
            break;
        case INVASION_TYPE_LOCAL_UPRISING:
        case INVASION_TYPE_MARS_NATIVES:
            enemy_id = ENEMY_0_BARBARIAN;
            attack_type = FORMATION_ATTACK_FOOD_CHAIN;
            break;
        default:
            break;
    }
    scenario_invasion_start_from_action(invasion_type, size, invasion_point, attack_type, enemy_id);
}

void scenario_invasion_warning_save_state(buffer *invasion_id, buffer *warnings)
{
    buffer_write_u16(invasion_id, data.last_internal_invasion_id);

    buffer_init_dynamic_array(warnings, data.warnings.size, WARNINGS_STRUCT_SIZE_CURRENT);

    const invasion_warning *w;
    array_foreach(data.warnings, w) {
        buffer_write_u8(warnings, w->in_use);
        buffer_write_u8(warnings, w->handled);
        buffer_write_u8(warnings, w->invasion_path_id);
        buffer_write_u8(warnings, w->warning_years);
        buffer_write_i16(warnings, w->x);
        buffer_write_i16(warnings, w->y);
        buffer_write_i16(warnings, w->image_id);
        buffer_write_i16(warnings, w->empire_object_id);
        buffer_write_i16(warnings, w->month_notified);
        buffer_write_i16(warnings, w->year_notified);
        buffer_write_i32(warnings, w->months_to_go);
        buffer_write_u8(warnings, w->invasion_id);
    }
}

void scenario_invasion_warning_load_state(buffer *invasion_id, buffer *warnings, int has_dynamic_warnings)
{
    data.last_internal_invasion_id = buffer_read_u16(invasion_id);

    unsigned int size = has_dynamic_warnings ? buffer_load_dynamic_array(warnings) : MAX_ORIGINAL_INVASION_WARNINGS;

    if (!array_init(data.warnings, WARNINGS_ARRAY_SIZE_STEP, new_warning, warning_in_use) ||
        !array_expand(data.warnings, size)) {
        log_error("Error creating warnings array - not enough memory. The game will now crash.", 0, 0);
    }

    for (unsigned int i = 0; i < size; i++) {
        invasion_warning *w = array_next(data.warnings);
        w->in_use = buffer_read_u8(warnings);
        w->handled = buffer_read_u8(warnings);
        w->invasion_path_id = buffer_read_u8(warnings);
        w->warning_years = buffer_read_u8(warnings);
        w->x = buffer_read_i16(warnings);
        w->y = buffer_read_i16(warnings);
        w->image_id = buffer_read_i16(warnings);
        w->empire_object_id = buffer_read_i16(warnings);
        w->month_notified = buffer_read_i16(warnings);
        w->year_notified = buffer_read_i16(warnings);
        w->months_to_go = buffer_read_i32(warnings);
        w->invasion_id = buffer_read_u8(warnings);
        if (!has_dynamic_warnings) {
            buffer_skip(warnings, 11);
        }
    }
    array_trim(data.warnings);
}

void scenario_invasion_save_state(buffer *buf)
{
    buffer_init_dynamic_array(buf, data.invasions.size, INVASIONS_STRUCT_SIZE_CURRENT);

    const invasion_t *invasion;
    array_foreach(data.invasions, invasion) {
        buffer_write_i16(buf, invasion->type);
        buffer_write_i16(buf, invasion->year);
        buffer_write_u16(buf, invasion->amount.min);
        buffer_write_u16(buf, invasion->amount.max);
        buffer_write_i16(buf, invasion->from);
        buffer_write_i16(buf, invasion->attack_type);
        buffer_write_u8(buf, invasion->month);
        buffer_write_i16(buf, invasion->repeat.times);
        buffer_write_u16(buf, invasion->repeat.interval.min);
        buffer_write_u16(buf, invasion->repeat.interval.max);
    }
}

void scenario_invasion_load_state(buffer *buf)
{
    unsigned int size = buffer_load_dynamic_array(buf);

    if (!array_init(data.invasions, INVASIONS_ARRAY_SIZE_STEP, new_invasion, invasion_in_use) ||
        !array_expand(data.invasions, size)) {
        log_error("Error creating invasions array - not enough memory. The game will now crash.", 0, 0);
    }

    for (unsigned int i = 0; i < size; i++) {
        invasion_t *invasion = array_next(data.invasions);
        invasion->type = buffer_read_i16(buf);
        invasion->year = buffer_read_i16(buf);
        invasion->amount.min = buffer_read_u16(buf);
        invasion->amount.max = buffer_read_u16(buf);
        invasion->from = buffer_read_i16(buf);
        invasion->attack_type = buffer_read_i16(buf);
        invasion->month = buffer_read_u8(buf);
        invasion->repeat.times = buffer_read_i16(buf);
        invasion->repeat.interval.min = buffer_read_u16(buf);
        invasion->repeat.interval.max = buffer_read_u16(buf);
    }
    array_trim(data.invasions);
}

int scenario_invasion_count_active_from_buffer(buffer *buf)
{
    unsigned int size = buffer_load_dynamic_array(buf);

    int num_invasions = 0;

    for (unsigned int i = 0; i < size; i++) {
        if (buffer_read_i16(buf) != INVASION_TYPE_NONE) {
            num_invasions++;
        }
        buffer_skip(buf, INVASIONS_STRUCT_SIZE_CURRENT - sizeof(int16_t));
    }

    return num_invasions;
}

void scenario_invasion_load_state_old_version(buffer *buf, invasion_old_state_sections section)
{
    invasion_t *invasion;
    if (section == INVASION_OLD_STATE_FIRST_SECTION) {
        if (!array_init(data.invasions, INVASIONS_ARRAY_SIZE_STEP, new_invasion, invasion_in_use) ||
            !array_expand(data.invasions, MAX_ORIGINAL_INVASIONS)) {
            log_error("Error creating invasions array - not enough memory. The game will now crash.", 0, 0);
        }
        for (size_t i = 0; i < MAX_ORIGINAL_INVASIONS; i++) {
            array_advance(data.invasions);
        }
        array_foreach(data.invasions, invasion) {
            invasion->year = buffer_read_i16(buf);
        }
        array_foreach(data.invasions, invasion) {
            invasion->type = buffer_read_i16(buf);
        }
        array_foreach(data.invasions, invasion) {
            invasion->amount.min = buffer_read_i16(buf);
            invasion->amount.max = invasion->amount.min;
        }
        array_foreach(data.invasions, invasion) {
            invasion->from = buffer_read_i16(buf);
        }
        array_foreach(data.invasions, invasion) {
            invasion->attack_type = buffer_read_i16(buf);
        }
    } else if (section == INVASION_OLD_STATE_LAST_SECTION) {
        array_foreach(data.invasions, invasion) {
            invasion->month = buffer_read_u8(buf);
        }
        array_trim(data.invasions);
    }
}
