#include "cheats.h"

#include "building/construction.h"
#include "building/menu.h"
#include "building/monument.h"
#include "building/type.h"
#include "city/gods.h"
#include "city/finance.h"
#include "city/data_private.h"
#include "city/sentiment.h"
#include "city/victory.h"
#include "city/warning.h"
#include "core/lang.h"
#include "core/string.h"
#include "empire/city.h"
#include "figure/figure.h"
#include "figuretype/crime.h"
#include "game/tick.h"
#include "graphics/color.h"
#include "graphics/font.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/invasion.h"
#include "scenario/scenario.h"
#include "translation/translation.h"
#include "window/building_info.h"
#include "window/city.h"
#include "window/console.h"

#include <string.h>

#define NUMBER_OF_COMMANDS 10

static void game_cheat_add_money(uint8_t *);
static void game_cheat_start_invasion(uint8_t *);
static void game_cheat_advance_year(uint8_t *);
static void game_cheat_cast_blessing(uint8_t *);
static void game_cheat_show_tooltip(uint8_t *);
static void game_cheat_kill_all(uint8_t *);
static void game_cheat_finish_monuments(uint8_t *);
static void game_cheat_set_monument_phase(uint8_t *);
static void game_cheat_unlock_all_buildings(uint8_t *);
static void game_cheat_incite_riot(uint8_t *);

static void (*const execute_command[])(uint8_t *args) = {
    game_cheat_add_money,
    game_cheat_start_invasion,
    game_cheat_advance_year,
    game_cheat_cast_blessing,
    game_cheat_show_tooltip,
    game_cheat_kill_all,
    game_cheat_finish_monuments,
    game_cheat_set_monument_phase,
    game_cheat_unlock_all_buildings,
    game_cheat_incite_riot
};

static const char *commands[] = {
    "addmoney",
    "startinvasion",
    "nextyear",
    "blessing",
    "showtooltip",
    "killall",
    "finishmonuments",
    "monumentphase",
    "whathaveromansdoneforus",
    "nike"
};

static struct {
    int is_cheating;
    int tooltip_enabled;
} data;

static int parse_word(uint8_t *string, uint8_t *word)
{
    int count = 0;
    while (*string && *string != ' ') {
        *word = *string;
        word++;
        string++;
        count++;
    }
    *word = 0;
    return count + 1;
}

// return value is next argument index
static int parse_integer(uint8_t *string, int *value)
{
    uint8_t copy[MAX_COMMAND_SIZE];
    int count = 0;
    while (*string && *string != ' ') {
        copy[count] = *string;
        count++;
        string++;
    }
    copy[count] = 0;
    *value = string_to_int(copy);
    return count + 1;
}

void game_cheat_activate(void)
{
    if (window_is(WINDOW_BUILDING_INFO)) {
        data.is_cheating = window_building_info_get_building_type() == BUILDING_WELL;
    } else if (data.is_cheating && window_is(WINDOW_MESSAGE_DIALOG)) {
        data.is_cheating = 2;
        scenario_invasion_start_from_cheat();
    } else {
        data.is_cheating = 0;
    }
}

int game_cheat_tooltip_enabled(void)
{
    return data.tooltip_enabled;
}

void game_cheat_money(void)
{
    if (data.is_cheating) {
        city_finance_process_cheat();
        window_invalidate();
    }
}

void game_cheat_victory(void)
{
    if (data.is_cheating) {
        city_victory_force_win();
    }
}

void game_cheat_breakpoint()
{
    if (data.is_cheating) {
    }
}

void game_cheat_console()
{
    if (data.is_cheating) {
        building_construction_clear_type();
        window_city_show();
        window_console_show();
    }
}

static void show_warning(translation_key key)
{
    city_warning_show_custom(lang_get_string(CUSTOM_TRANSLATION, key), NEW_WARNING_SLOT);    
}

static void game_cheat_add_money(uint8_t *args)
{
    int money = 0;
    parse_integer(args, &money);
    city_finance_process_console(money);
    window_invalidate();
    show_warning(TR_CHEAT_ADDED_MONEY);
}

static void game_cheat_start_invasion(uint8_t *args)
{
    int attack_type = 0;
    int size = 0;
    int invasion_point = 0;
    int index = parse_integer(args, &attack_type); // 0 barbarians, 1 caesar, 2 mars natives
    index = parse_integer(args + index, &size);
    parse_integer(args + index, &invasion_point);
    scenario_invasion_start_from_console(attack_type, size, invasion_point);
    show_warning(TR_CHEAT_STARTED_INVASION);
}

static void game_cheat_advance_year(uint8_t *args)
{
    game_tick_cheat_year();
    show_warning(TR_CHEAT_YEAR_ADVANCED);
}

static void game_cheat_cast_blessing(uint8_t *args)
{
    int god_id = 0;
    parse_integer(args, &god_id);
    city_god_blessing_cheat(god_id);
    show_warning(TR_CHEAT_CASTED_BLESSING);
}

static void game_cheat_show_tooltip(uint8_t *args)
{
    parse_integer(args, &data.tooltip_enabled);
    show_warning(TR_CHEAT_TOGGLE_TOOLTIPS);
}

static void game_cheat_kill_all(uint8_t *args)
{
    figure_kill_all();
    show_warning(TR_CHEAT_KILLED_ALL_WALKERS);
}

static void game_cheat_finish_monuments(uint8_t *args)
{
    building_monument_finish_monuments();
    show_warning(TR_CHEAT_FINISHED_MONUMENTS);
}

static void game_cheat_set_monument_phase(uint8_t *args)
{
    int phase = 0;
    parse_integer(args, &phase);
    building_monuments_set_construction_phase(phase);
    show_warning(TR_CHEAT_UPDATED_MONUMENTS);
}

static void game_cheat_unlock_all_buildings(uint8_t *args)
{
    building_menu_enable_all();
    empire_unlock_all_resources();
    scenario_unlock_all_buildings();
    show_warning(TR_CHEAT_UNLOCKED_ALL_BUILDINGS);
}

static void game_cheat_incite_riot(uint8_t *args)
{
    city_data.sentiment.value = 0;
    city_sentiment_change_happiness(-100);
    figure_generate_criminals();
    show_warning(TR_CHEAT_INCITED_RIOT);
    city_data.sentiment.value = 50;
    city_sentiment_change_happiness(50);
}

void game_cheat_parse_command(uint8_t *command)
{
    uint8_t command_to_call[MAX_COMMAND_SIZE];
    int next_arg = parse_word(command, command_to_call);
    for (int i = 0; i < NUMBER_OF_COMMANDS; i++) {
        if (strcmp((char *) command_to_call, commands[i]) == 0) {
            (*execute_command[i])(command + next_arg);
        }
    }
}
