#include "invasion.h"

#include "city/message.h"
#include "core/calc.h"
#include "core/random.h"
#include "empire/object.h"
#include "figure/formation.h"
#include "figure/name.h"
#include "game/difficulty.h"
#include "game/time.h"
#include "scenario/data.h"
#include "scenario/map.h"

#include <string.h>

#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Figure.h"
#include "Data/Grid.h"
#include "Data/Invasion.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"
#include "../Building.h"
#include "CityInfo.h"
#include "Figure.h"

#define MAX_INVASIONS 20

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

static struct {
    int last_internal_invasion_id;
} data;

void scenario_invasion_init()
{
    memset(Data_InvasionWarnings, 0, sizeof(Data_InvasionWarnings));
    int path_current = 1;
    int path_max = empire_object_get_max_invasion_path();
    if (path_max == 0) {
        return;
    }
    struct Data_InvasionWarning *warning = &Data_InvasionWarnings[1];
    for (int i = 0; i < MAX_INVASIONS; i++) {
        random_generate_next();
        if (!Data_Scenario.invasions.type[i]) {
            continue;
        }
        Data_Scenario.invasions_month[i] = 2 + (random_byte() & 7);
        if (Data_Scenario.invasions.type[i] == INVASION_TYPE_LOCAL_UPRISING ||
            Data_Scenario.invasions.type[i] == INVASION_TYPE_DISTANT_BATTLE) {
            continue;
        }
        for (int year = 1; year < 8; year++) {
            const empire_object *obj = empire_object_get_battle_icon(path_current, year);
            if (!obj) {
                continue;
            }
            warning->inUse = 1;
            warning->empireInvasionPathId = obj->invasion_path_id;
            warning->warningYears = obj->invasion_years;
            warning->empireX = obj->x;
            warning->empireY = obj->y;
            warning->empireGraphicId = obj->image_id;
            warning->invasionId = i;
            warning->empireObjectId = obj->id;
            warning->gameMonthNotified = 0;
            warning->gameYearNotified = 0;
            warning->monthsToGo = 12 * Data_Scenario.invasions.year[i];
            warning->monthsToGo += Data_Scenario.invasions_month[i];
            warning->monthsToGo -= 12 * year;
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
        if (Data_InvasionWarnings[i].inUse && Data_InvasionWarnings[i].handled) {
            return 1;
        }
    }
    return 0;
}

int scenario_invasion_count()
{
    int num_invasions = 0;
    for (int i = 0; i < MAX_INVASIONS; i++) {
        if (Data_Scenario.invasions.type[i]) {
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
        for (int i = 0; i < 8; i++) {
            if (Data_Scenario.invasionPoints.x[i] != -1) {
                num_points++;
            }
        }
        if (invasion_point == 8) {
            if (num_points <= 2) {
                invasion_point = random_byte() & 1;
            } else if (num_points <= 4) {
                invasion_point = random_byte() & 3;
            } else {
                invasion_point = random_byte() & 7;
            }
        }
        if (num_points > 0) {
            while (Data_Scenario.invasionPoints.x[invasion_point] == -1) {
                invasion_point++;
                if (invasion_point >= 8) {
                    invasion_point = 0;
                }
            }
        }
        x = Data_Scenario.invasionPoints.x[invasion_point];
        y = Data_Scenario.invasionPoints.y[invasion_point];
    }
    if (x == -1 || y == -1) {
        map_point exit_point = scenario_map_exit();
        x = exit_point.x;
        y = exit_point.y;
    }
    // determine orientation
    if (y == 0) {
        orientation = Dir_4_Bottom;
    } else if (y >= Data_Scenario.mapSizeY - 1) {
        orientation = Dir_0_Top;
    } else if (x == 0) {
        orientation = Dir_2_Right;
    } else if (x >= Data_Scenario.mapSizeX - 1) {
        orientation = Dir_6_Left;
    } else {
        orientation = Dir_4_Bottom;
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
                Data_Figures[figureId].isFriendly = 0;
                Data_Figures[figureId].actionState = FigureActionState_151_EnemyInitial;
                Data_Figures[figureId].waitTicks = 200 * seq + 10 * fig + 10;
                Data_Figures[figureId].formationId = formation_id;
                Data_Figures[figureId].name = figure_name_get(figure_type, enemy_type);
                Data_Figures[figureId].isGhost = 1;
            }
            seq++;
        }
    }
    return grid_offset;
}

void scenario_invasion_process()
{
    int enemy_id = Data_Scenario.enemyId;
    for (int i = 0; i < MAX_INVASION_WARNINGS; i++) {
        if (!Data_InvasionWarnings[i].inUse) {
            continue;
        }
        // update warnings
        struct Data_InvasionWarning *warning = &Data_InvasionWarnings[i];
        warning->monthsToGo--;
        if (warning->monthsToGo <= 0) {
            if (warning->handled != 1) {
                warning->handled = 1;
                warning->gameYearNotified = game_time_year();
                warning->gameMonthNotified = game_time_month();
                if (warning->warningYears > 2) {
                    city_message_post(0, MESSAGE_DISTANT_BATTLE, 0, 0);
                } else if (warning->warningYears > 1) {
                    city_message_post(0, MESSAGE_ENEMIES_CLOSING, 0, 0);
                } else {
                    city_message_post(0, MESSAGE_ENEMIES_AT_THE_DOOR, 0, 0);
                }
            }
        }
        if (game_time_year() >= Data_Scenario.startYear + Data_Scenario.invasions.year[warning->invasionId] &&
            game_time_month() >= Data_Scenario.invasions_month[warning->invasionId]) {
            // invasion attack time has passed
            warning->inUse = 0;
            if (warning->warningYears > 1) {
                continue;
            }
            // enemy invasions
            if (Data_Scenario.invasions.type[warning->invasionId] == INVASION_TYPE_ENEMY_ARMY) {
                int grid_offset = start_invasion(
                    ENEMY_ID_TO_ENEMY_TYPE[enemy_id],
                    Data_Scenario.invasions.amount[warning->invasionId],
                    Data_Scenario.invasions.from[warning->invasionId],
                    Data_Scenario.invasions.attackType[warning->invasionId],
                    warning->invasionId);
                if (grid_offset > 0) {
                    if (ENEMY_ID_TO_ENEMY_TYPE[enemy_id] > 4) {
                        city_message_post(1, MESSAGE_ENEMY_ARMY_ATTACK, data.last_internal_invasion_id, grid_offset);
                    } else {
                        city_message_post(1, MESSAGE_BARBARIAN_ATTACK, data.last_internal_invasion_id, grid_offset);
                    }
                }
            }
            if (Data_Scenario.invasions.type[warning->invasionId] == INVASION_TYPE_CAESAR) {
                int grid_offset = start_invasion(
                    EnemyType_11_Caesar,
                    Data_Scenario.invasions.amount[warning->invasionId],
                    Data_Scenario.invasions.from[warning->invasionId],
                    Data_Scenario.invasions.attackType[warning->invasionId],
                    warning->invasionId);
                if (grid_offset > 0) {
                    city_message_post(1, MESSAGE_CAESAR_ARMY_ATTACK, data.last_internal_invasion_id, grid_offset);
                }
            }
        }
    }
    // local uprisings
    for (int i = 0; i < MAX_INVASIONS; i++) {
        if (Data_Scenario.invasions.type[i] == INVASION_TYPE_LOCAL_UPRISING) {
            if (game_time_year() == Data_Scenario.startYear + Data_Scenario.invasions.year[i] &&
                game_time_month() == Data_Scenario.invasions_month[i]) {
                int grid_offset = start_invasion(
                    EnemyType_0_Barbarian,
                    Data_Scenario.invasions.amount[i],
                    Data_Scenario.invasions.from[i],
                    Data_Scenario.invasions.attackType[i],
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
    if (Data_Settings.saveGameMissionId < 0 || Data_Settings.saveGameMissionId > 19) {
        return 0;
    }
    int amount = LOCAL_UPRISING_NUM_ENEMIES[Data_Settings.saveGameMissionId];
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
    int enemy_id = Data_Scenario.enemyId;
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

void scenario_invasion_save_state(buffer *buf)
{
    buffer_write_u16(buf, data.last_internal_invasion_id);
}

void scenario_invasion_load_state(buffer *buf)
{
    data.last_internal_invasion_id = buffer_read_u16(buf);
}
