#include "games.h"

#include "building/granary.h"
#include "building/warehouse.h"
#include "building/monument.h"
#include "core/calc.h"
#include "city/constants.h"
#include "city/data_private.h"
#include "city/emperor.h"
#include "city/finance.h"
#include "city/message.h"
#include "city/sentiment.h"
#include "core/config.h"
#include "game/time.h"

#define POPULATION_SCALING_FACTOR 1200
#define BASE_RESOURCE_REQUIREMENT 3

typedef enum {
    G_PLANNING,
    G_STARTING,
    G_ENDING
} games_messages;

static void naval_battle_start(int id);
static void executions_start(int id);
static void imperial_games_start(int id);

games_type ALL_GAMES[MAX_GAMES] = {
    {
        1, TR_WINDOW_GAMES_OPTION_1, TR_WINDOW_GAMES_OPTION_1_DESC, MESSAGE_NG_GAMES_PLANNED, 1500, 100, 1, 32, 12,
        BUILDING_COLOSSEUM, 1, { [RESOURCE_WINE] = 1, [RESOURCE_TIMBER] = 1 }, naval_battle_start
    },
    {
        2, TR_WINDOW_GAMES_OPTION_5, TR_WINDOW_GAMES_OPTION_5_DESC, MESSAGE_IG_GAMES_PLANNED, 800, 150, 1, 32, 12,
        BUILDING_COLOSSEUM, 0, { [RESOURCE_WHEAT] = 2, [RESOURCE_OIL] = 1 }, imperial_games_start
    },
    {
        3, TR_WINDOW_GAMES_OPTION_2, TR_WINDOW_GAMES_OPTION_2_DESC, MESSAGE_AN_GAMES_PLANNED, 800, 150, 1, 32, 12,
        BUILDING_COLOSSEUM, 0, { [RESOURCE_MEAT] = 2 }, executions_start
    }
};


games_type *city_games_get_game_type(int id)
{
    for (int i = 0; i <= MAX_GAMES; ++i) {
        if (ALL_GAMES[i].id == id) {
            return &ALL_GAMES[i];
        }
    }
    return 0;
}

int city_games_money_cost(int game_type_id)
{
    games_type *game = city_games_get_game_type(game_type_id);
    if (!game) {
        return 0;
    }
    int cost = game->cost_base + game->cost_scaling * (city_data.population.population / POPULATION_SCALING_FACTOR);
    return cost;
}

int city_games_resource_cost(int game_type_id, resource_type resource)
{
    games_type *game = city_games_get_game_type(game_type_id);
    if (!game) {
        return 0;
    }
    int cost = game->resource_cost[resource] * (BASE_RESOURCE_REQUIREMENT + city_data.population.population / POPULATION_SCALING_FACTOR);
    return cost;
}


static void post_games_message(int type)
{
    games_type *game = city_games_get_game_type(city_data.games.selected_games_id);
    city_message_post(1, game->message_planning + type, 0, 0);
}

static void begin_games(void)
{
    games_type *game = city_games_get_game_type(city_data.games.selected_games_id);

    city_data.games.months_to_go = 0;
    city_data.games.games_is_active = 1;
    city_data.games.remaining_duration = game->duration_days;
    game->games_start_function(city_data.games.selected_games_id);

    post_games_message(G_STARTING);
}

static void end_games(void)
{
    city_data.games.games_is_active = 0;
    city_data.games.remaining_duration = 0;

    city_data.games.months_since_last = 0;

    post_games_message(G_ENDING);
}

void city_games_schedule(int game_id)
{
    games_type *game = city_games_get_game_type(game_id);
    city_emperor_decrement_personal_savings(city_games_money_cost(game_id));

    for (int resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
        int resource_cost = city_games_resource_cost(game_id, resource);
        if (resource_cost) {
            resource_cost = building_warehouses_remove_resource(resource, resource_cost);
            if (resource_cost > 0 && resource_is_food(resource)) {
                building_granaries_remove_resource(resource, resource_cost * RESOURCE_ONE_LOAD);
            }
        }
    }

    city_data.games.months_to_go = game->delay_months;
    post_games_message(G_PLANNING);
}

void city_games_decrement_month_counts(void)
{
    city_data.games.months_since_last++;

    if (city_data.games.months_to_go) {
        city_data.games.months_to_go--;
        if (city_data.games.months_to_go == 0) {
            begin_games();
        }
    }
    if (city_data.games.naval_battle_bonus_months) {
        city_data.games.naval_battle_bonus_months--;
    }
    if (city_data.games.executions_bonus_months) {
        city_data.games.executions_bonus_months--;
    }
    if (city_data.games.imperial_games_bonus_months) {
        city_data.games.imperial_games_bonus_months--;
    }
    if (city_data.games.games_4_bonus_months) {
        city_data.games.games_4_bonus_months--;
    }
}

void city_games_decrement_duration(void)
{
    if (city_data.games.remaining_duration && city_data.games.games_is_active) {
        city_data.games.remaining_duration--;
        if (city_data.games.remaining_duration == 0) {
            end_games();
        }
    }
}

int city_games_naval_battle_active(void)
{
    return city_data.games.naval_battle_bonus_months;
}

int city_games_executions_active(void)
{
    return city_data.games.executions_bonus_months;
}

int city_games_imperial_festival_active(void)
{
    return city_data.games.imperial_games_bonus_months;
}

int city_games_naval_battle_distant_battle_bonus_active(void)
{
    return city_data.games.naval_battle_distant_battle_bonus;
}

void city_games_remove_naval_battle_distant_battle_bonus(void)
{
    city_data.games.naval_battle_distant_battle_bonus = 0;
}

static void naval_battle_start(int id)
{
    games_type *game = city_games_get_game_type(city_data.games.selected_games_id);
    city_data.games.naval_battle_bonus_months = game->bonus_duration;
    city_data.games.naval_battle_distant_battle_bonus = 1;
}

static void executions_start(int id)
{
    games_type *game = city_games_get_game_type(city_data.games.selected_games_id);
    city_data.games.executions_bonus_months = game->bonus_duration;
}

static void imperial_games_start(int id)
{
    games_type *game = city_games_get_game_type(city_data.games.selected_games_id);
    city_data.games.imperial_games_bonus_months = game->bonus_duration;
}
