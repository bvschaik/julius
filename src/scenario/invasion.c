#include "invasion.h"

#include "city/message.h"
#include "core/calc.h"
#include "core/random.h"
#include "empire/object.h"
#include "figure/figure.h"
#include "figure/formation.h"
#include "figure/name.h"
#include "game/difficulty.h"
#include "game/time.h"
#include "scenario/data.h"
#include "scenario/map.h"
#include "scenario/property.h"

#include <string.h>

#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Grid.h"
#include "Data/State.h"
#include "../Building.h"
#include "CityInfo.h"
#include "Figure.h"

#define MAX_INVASION_WARNINGS 101

static const int ENEMY_ID_TO_ENEMY_TYPE[20] = {
    0, 7, 7, 10, 8, 8, 9, 5, 6, 3, 3, 3, 2, 2, 4, 4, 4, 6, 1, 6
};

static const int LOCAL_UPRISING_NUM_ENEMIES[20] = {
    0, 0, 0, 0, 0, 3, 3, 3, 0, 6, 6, 6, 6, 6, 9, 9, 9, 9, 9, 9
};

static const struct {
    int pctType1;
    int pctType2;
    int pctType3;
    int figureTypes[3];
    int formationLayout;
} ENEMY_PROPERTIES[12] = {
    {100, 0, 0, {49, 0, 0}, 8},
    {40, 60, 0, {49, 51, 0}, 8},
    {50, 50, 0, {50, 53, 0}, 8},
    {80, 20, 0, {50, 48, 0}, 8},
    {50, 50, 0, {49, 52, 0}, 8},
    {30, 70, 0, {44, 43, 0}, 0},
    {50, 50, 0, {44, 43, 0}, 10},
    {50, 50, 0, {45, 43, 0}, 10},
    {80, 20, 0, {45, 43, 0}, 10},
    {80, 20, 0, {44, 46, 0}, 11},
    {90, 10, 0, {45, 47, 0}, 11},
    {100, 0, 0, {57, 0, 0}, 0}
};

typedef struct {
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
    invasion_warning warnings[MAX_INVASION_WARNINGS];
} data;

void scenario_invasion_init()
{
    memset(data.warnings, 0, MAX_INVASION_WARNINGS * sizeof(invasion_warning));
    int path_current = 1;
    int path_max = empire_object_get_max_invasion_path();
    if (path_max == 0) {
        return;
    }
    invasion_warning *warning = &data.warnings[1];
    for (int i = 0; i < MAX_INVASIONS; i++) {
        random_generate_next();
        if (!scenario.invasions[i].type) {
            continue;
        }
        scenario.invasions[i].month = 2 + (random_byte() & 7);
        if (scenario.invasions[i].type == INVASION_TYPE_LOCAL_UPRISING ||
            scenario.invasions[i].type == INVASION_TYPE_DISTANT_BATTLE) {
            continue;
        }
        for (int year = 1; year < 8; year++) {
            const empire_object *obj = empire_object_get_battle_icon(path_current, year);
            if (!obj) {
                continue;
            }
            warning->in_use = 1;
            warning->invasion_path_id = obj->invasion_path_id;
            warning->warning_years = obj->invasion_years;
            warning->x = obj->x;
            warning->y = obj->y;
            warning->image_id = obj->image_id;
            warning->invasion_id = i;
            warning->empire_object_id = obj->id;
            warning->month_notified = 0;
            warning->year_notified = 0;
            warning->months_to_go = 12 * scenario.invasions[i].year;
            warning->months_to_go += scenario.invasions[i].month;
            warning->months_to_go -= 12 * year;
            ++warning;
        }
        path_current++;
        if (path_current > path_max) {
            path_current = 1;
        }
    }
}

int scenario_invasion_exists_upcoming()
{
    for (int i = 0; i < MAX_INVASION_WARNINGS; i++) {
        if (data.warnings[i].in_use && data.warnings[i].handled) {
            return 1;
        }
    }
    return 0;
}

void scenario_invasion_foreach_warning(void (*callback)(int x, int y, int image_id))
{
    for (int i = 0; i < MAX_INVASION_WARNINGS; i++) {
        if (data.warnings[i].in_use && data.warnings[i].handled) {
            callback(data.warnings[i].x, data.warnings[i].y, data.warnings[i].image_id);
        }
    }
}

int scenario_invasion_count()
{
    int num_invasions = 0;
    for (int i = 0; i < MAX_INVASIONS; i++) {
        if (scenario.invasions[i].type) {
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

static int start_invasion(int enemy_type, int amount, int invasion_point, int attack_type, int invasion_id)
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
    int num_type1 = calc_adjust_with_percentage(amount, ENEMY_PROPERTIES[enemy_type].pctType1);
    int num_type2 = calc_adjust_with_percentage(amount, ENEMY_PROPERTIES[enemy_type].pctType2);
    int num_type3 = calc_adjust_with_percentage(amount, ENEMY_PROPERTIES[enemy_type].pctType3);
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
    if (enemy_type == EnemyType_11_Caesar) {
        map_point entry_point = scenario_map_entry();
        x = entry_point.x;
        y = entry_point.y;
    } else {
        int num_points = 0;
        for (int i = 0; i < MAX_INVASION_POINTS; i++) {
            if (scenario.invasion_points[i].x != -1) {
                num_points++;
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
    int grid_offset = GridOffset(x, y);
    int terrain = Data_Grid_terrain[grid_offset];
    if (terrain & (Terrain_Elevation | Terrain_Rock | Terrain_Tree)) {
        return -1;
    }
    if (terrain & Terrain_Water) {
        if (!(terrain & Terrain_Road)) { // bridge
            return -1;
        }
    } else if (terrain & (Terrain_Building | Terrain_Aqueduct | Terrain_Gatehouse | Terrain_Wall)) {
        Building_destroyByEnemy(x, y, grid_offset);
    }
    // spawn the lot!
    int seq = 0;
    for (int type = 0; type < 3; type++) {
        if (formations_per_type[type] <= 0) {
            continue;
        }
        int figure_type = ENEMY_PROPERTIES[enemy_type].figureTypes[type];
        for (int i = 0; i < formations_per_type[type]; i++) {
            int formation_id = formation_create_enemy(
                figure_type, x, y, ENEMY_PROPERTIES[enemy_type].formationLayout, orientation,
                enemy_type, attack_type, invasion_id, data.last_internal_invasion_id
            );
            if (formation_id <= 0) {
                continue;
            }
            for (int fig = 0; fig < soldiers_per_formation[type][i]; fig++) {
                int figureId = Figure_create(figure_type, x, y, orientation);
                struct Data_Figure *f = figure_get(figureId);
                f->isFriendly = 0;
                f->actionState = FigureActionState_151_EnemyInitial;
                f->waitTicks = 200 * seq + 10 * fig + 10;
                f->formationId = formation_id;
                f->name = figure_name_get(figure_type, enemy_type);
                f->isGhost = 1;
            }
            seq++;
        }
    }
    return grid_offset;
}

void scenario_invasion_process()
{
    int enemy_id = scenario.enemy_id;
    for (int i = 0; i < MAX_INVASION_WARNINGS; i++) {
        if (!data.warnings[i].in_use) {
            continue;
        }
        // update warnings
        invasion_warning *warning = &data.warnings[i];
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
        if (game_time_year() >= scenario.start_year + scenario.invasions[warning->invasion_id].year &&
            game_time_month() >= scenario.invasions[warning->invasion_id].month) {
            // invasion attack time has passed
            warning->in_use = 0;
            if (warning->warning_years > 1) {
                continue;
            }
            // enemy invasions
            if (scenario.invasions[warning->invasion_id].type == INVASION_TYPE_ENEMY_ARMY) {
                int grid_offset = start_invasion(
                    ENEMY_ID_TO_ENEMY_TYPE[enemy_id],
                    scenario.invasions[warning->invasion_id].amount,
                    scenario.invasions[warning->invasion_id].from,
                    scenario.invasions[warning->invasion_id].attack_type,
                    warning->invasion_id);
                if (grid_offset > 0) {
                    if (ENEMY_ID_TO_ENEMY_TYPE[enemy_id] > 4) {
                        city_message_post(1, MESSAGE_ENEMY_ARMY_ATTACK, data.last_internal_invasion_id, grid_offset);
                    } else {
                        city_message_post(1, MESSAGE_BARBARIAN_ATTACK, data.last_internal_invasion_id, grid_offset);
                    }
                }
            }
            if (scenario.invasions[warning->invasion_id].type == INVASION_TYPE_CAESAR) {
                int grid_offset = start_invasion(
                    EnemyType_11_Caesar,
                    scenario.invasions[warning->invasion_id].amount,
                    scenario.invasions[warning->invasion_id].from,
                    scenario.invasions[warning->invasion_id].attack_type,
                    warning->invasion_id);
                if (grid_offset > 0) {
                    city_message_post(1, MESSAGE_CAESAR_ARMY_ATTACK, data.last_internal_invasion_id, grid_offset);
                }
            }
        }
    }
    // local uprisings
    for (int i = 0; i < MAX_INVASIONS; i++) {
        if (scenario.invasions[i].type == INVASION_TYPE_LOCAL_UPRISING) {
            if (game_time_year() == scenario.start_year + scenario.invasions[i].year &&
                game_time_month() == scenario.invasions[i].month) {
                int grid_offset = start_invasion(
                    EnemyType_0_Barbarian,
                    scenario.invasions[i].amount,
                    scenario.invasions[i].from,
                    scenario.invasions[i].attack_type,
                    i);
                if (grid_offset > 0) {
                    city_message_post(1, MESSAGE_LOCAL_UPRISING, data.last_internal_invasion_id, grid_offset);
                }
            }
        }
    }
}

int scenario_invasion_start_from_mars()
{
    int mission = scenario_campaign_mission();
    if (mission < 0 || mission > 19) {
        return 0;
    }
    int amount = LOCAL_UPRISING_NUM_ENEMIES[mission];
    if (amount <= 0) {
        return 0;
    }
    int grid_offset = start_invasion(EnemyType_0_Barbarian, amount, 8, FORMATION_ATTACK_FOOD_CHAIN, 23);
    if (grid_offset) {
        city_message_post(1, MESSAGE_LOCAL_UPRISING_MARS, data.last_internal_invasion_id, grid_offset);
    }
    return 1;
}

void scenario_invasion_start_from_cheat()
{
    int enemy_id = scenario.enemy_id;
    int grid_offset = start_invasion(ENEMY_ID_TO_ENEMY_TYPE[enemy_id], 150, 8, FORMATION_ATTACK_FOOD_CHAIN, 23);
    if (grid_offset) {
        if (ENEMY_ID_TO_ENEMY_TYPE[enemy_id] > 4) {
            city_message_post(1, MESSAGE_ENEMY_ARMY_ATTACK, data.last_internal_invasion_id, grid_offset);
        } else {
            city_message_post(1, MESSAGE_BARBARIAN_ATTACK, data.last_internal_invasion_id, grid_offset);
        }
    }
}

static void caesar_invasion_pause()
{
    formation_caesar_pause();
}

static void caesar_invasion_retreat()
{
    formation_caesar_retreat();
    if (!Data_CityInfo.caesarInvasionRetreatMessageShown) {
        Data_CityInfo.caesarInvasionRetreatMessageShown = 1;
        city_message_post(1, MESSAGE_CAESAR_ARMY_RETREAT, 0, 0);
    }
}

void scenario_invasion_process_caesar()
{
    if (Data_CityInfo.numImperialSoldiersInCity) {
        // caesar invasion in progress
        Data_CityInfo.caesarInvasionDurationDayCountdown--;
        if (Data_CityInfo.ratingFavor >= 35 && Data_CityInfo.caesarInvasionDurationDayCountdown < 176) {
            caesar_invasion_retreat();
        } else if (Data_CityInfo.ratingFavor >= 22) {
            if (Data_CityInfo.caesarInvasionDurationDayCountdown > 0) {
                caesar_invasion_pause();
            } else if (Data_CityInfo.caesarInvasionDurationDayCountdown == 0) {
                city_message_post(1, MESSAGE_CAESAR_ARMY_CONTINUE, 0, 0); // a year has passed (11 months), siege goes on
            }
        }
    } else if (Data_CityInfo.caesarInvasionSoldiersDied && Data_CityInfo.caesarInvasionSoldiersDied >= Data_CityInfo.caesarInvasionSize) {
        // player defeated caesar army
        Data_CityInfo.caesarInvasionSize = 0;
        Data_CityInfo.caesarInvasionSoldiersDied = 0;
        if (Data_CityInfo.ratingFavor < 35) {
            CityInfo_Ratings_changeFavor(10);
            if (Data_CityInfo.caesarInvasionCount < 2) {
                city_message_post(1, MESSAGE_CAESAR_RESPECT_1, 0, 0);
            } else if (Data_CityInfo.caesarInvasionCount < 3) {
                city_message_post(1, MESSAGE_CAESAR_RESPECT_2, 0, 0);
            } else {
                city_message_post(1, MESSAGE_CAESAR_RESPECT_3, 0, 0);
            }
        }
    } else if (Data_CityInfo.caesarInvasionDaysUntilInvasion <= 0) {
        if (Data_CityInfo.ratingFavor <= 10) {
            // warn player that caesar is angry and will invade in a year
            Data_CityInfo.caesarInvasionWarningsGiven++;
            Data_CityInfo.caesarInvasionDaysUntilInvasion = 192;
            if (Data_CityInfo.caesarInvasionWarningsGiven <= 1) {
                city_message_post(1, MESSAGE_CAESAR_WRATH, 0, 0);
            }
        }
    } else {
        Data_CityInfo.caesarInvasionDaysUntilInvasion--;
        if (Data_CityInfo.caesarInvasionDaysUntilInvasion == 0) {
            // invade!
            int size;
            if (Data_CityInfo.caesarInvasionCount == 0) {
                size = 32;
            } else if (Data_CityInfo.caesarInvasionCount == 1) {
                size = 64;
            } else if (Data_CityInfo.caesarInvasionCount == 2) {
                size = 96;
            } else {
                size = 144;
            }
            int invasion_id = start_invasion(
                EnemyType_11_Caesar, size, 0, FORMATION_ATTACK_BEST_BUILDINGS, 24);
            if (invasion_id > 0) {
                Data_CityInfo.caesarInvasionCount++;
                Data_CityInfo.caesarInvasionDurationDayCountdown = 192;
                Data_CityInfo.caesarInvasionRetreatMessageShown = 0;
                city_message_post(1, MESSAGE_CAESAR_ARMY_ATTACK, data.last_internal_invasion_id, invasion_id);
                Data_CityInfo.caesarInvasionSize = size;
                Data_CityInfo.caesarInvasionSoldiersDied = 0;
            }
        }
    }
}

void scenario_invasion_save_state(buffer *invasion_id, buffer *warnings)
{
    buffer_write_u16(invasion_id, data.last_internal_invasion_id);

    for (int i = 0; i < MAX_INVASION_WARNINGS; i++) {
        const invasion_warning *w = &data.warnings[i];
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
        for (int x = 0; x < 11; x++) {
            buffer_write_u8(warnings, 0);
        }
    }
}

void scenario_invasion_load_state(buffer *invasion_id, buffer *warnings)
{
    data.last_internal_invasion_id = buffer_read_u16(invasion_id);

    for (int i = 0; i < MAX_INVASION_WARNINGS; i++) {
        invasion_warning *w = &data.warnings[i];
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
        buffer_skip(warnings, 11);
    }
}
