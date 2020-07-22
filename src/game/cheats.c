#include "cheats.h"

#include "building/construction.h"
#include "building/type.h"
#include "city/gods.h"
#include "city/finance.h"
#include "city/victory.h"
#include "city/warning.h"
#include "core/string.h"
#include "figure/figure.h"
#include "game/tick.h"
#include "graphics/color.h"
#include "graphics/font.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/invasion.h"
#include "window/building_info.h"
#include "window/city.h"
#include "window/console.h"

#define NUMBER_OF_COMMANDS 6

static void game_cheat_add_money(uint8_t *);
static void game_cheat_start_invasion(uint8_t *);
static void game_cheat_advance_year(uint8_t *);
static void game_cheat_cast_blessing(uint8_t *);
static void game_cheat_show_tooltip(uint8_t *);
static void game_cheat_kill_all(uint8_t*);

static void (* const execute_command[])(uint8_t * args) = {
    game_cheat_add_money,
    game_cheat_start_invasion,
    game_cheat_advance_year,
    game_cheat_cast_blessing,
    game_cheat_show_tooltip,
    game_cheat_kill_all
};

static const char *commands[] = {
    "addmoney",
    "startinvasion",
    "nextyear",
    "blessing",
    "showtooltip",
    "killall"
};

static struct {
    int is_cheating;
    int tooltip_enabled;
} data;

static int parse_word(uint8_t * string, uint8_t * word){
    int count = 0;
    while( *string && *string != ' '){
        *word = *string;
        word++;
        string++;
        count++;
    }
    *word = 0;
    return count+1;
}

// return value is next argument index
static int parse_integer(uint8_t * string, int * value){
    uint8_t copy[MAX_COMMAND_SIZE];
    int count = 0;
    while( *string && *string != ' '){
        copy[count] = *string;
        count++;
        string++;
    }
    copy[count] = 0;
    *value = string_to_int(copy);
    return count+1;
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

int game_cheat_tooltip_enabled(void){
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

void game_cheat_breakpoint(){
    if (data.is_cheating) {
    }
}

void game_cheat_console(){
    if (data.is_cheating) {
        building_construction_clear_type();
        window_city_show();
        window_console_show();
    }
}

static void game_cheat_add_money(uint8_t * args){
    int money = 0;
    parse_integer(args, &money);
    city_finance_process_console(money);
    window_invalidate();

    city_warning_show_console((uint8_t*)"Added money");
}

static void game_cheat_start_invasion(uint8_t * args){
    int attack_type = 0;
    int size = 0;
    int invasion_point = 0;
    int index = parse_integer(args, &attack_type); // 0 barbarians, 1 caesar, 2 mars natives
    index = parse_integer(args+index, &size);
    parse_integer(args+index,&invasion_point);
    scenario_invasion_start_from_console(attack_type, size, invasion_point);

    city_warning_show_console((uint8_t*)"Started invasion");
}

static void game_cheat_advance_year(uint8_t * args){
    game_tick_cheat_year();

    city_warning_show_console((uint8_t*)"Year advanced");
} 

static void game_cheat_cast_blessing(uint8_t * args){
    int god_id = 0;
    parse_integer(args, &god_id);
    city_god_blessing_cheat(god_id);

    city_warning_show_console((uint8_t*)"Casted blessing");
}

static void game_cheat_show_tooltip(uint8_t * args){
    parse_integer(args, &data.tooltip_enabled);

    city_warning_show_console((uint8_t*)"Show tooltip toggled");

}

static void game_cheat_kill_all(uint8_t* args) {
    figure_kill_all();
    city_warning_show_console((uint8_t*)"Killed all walkers");
}


void game_cheat_parse_command(uint8_t * command){
    uint8_t command_to_call[MAX_COMMAND_SIZE];
    int next_arg = parse_word(command,command_to_call);
    for (int i = 0; i < NUMBER_OF_COMMANDS; i++) {
        if (string_compare_case_insensitive((char *)command_to_call, commands[i]) == 0) {
            (*execute_command[i])(command+next_arg);
        }
    }
}
