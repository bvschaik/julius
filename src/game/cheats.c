#include "cheats.h"

#include "building/construction.h"
#include "building/type.h"
#include "city/finance.h"
#include "city/victory.h"
#include "city/warning.h"
#include "core/string.h"
#include "graphics/window.h"
#include "scenario/invasion.h"
#include "window/building_info.h"
#include "window/city.h"
#include "window/console.h"

#define NUMBER_OF_COMMANDS 1

static void game_cheat_add_money(uint8_t *);

static void (* const execute_command[])(uint8_t * args) = {
    game_cheat_add_money,
};

static const char *commands[] = {
    "addmoney",
};

static struct {
    int is_cheating;
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
        asm("nop");
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
void game_cheat_parse_command(uint8_t * command){
    uint8_t command_to_call[MAX_COMMAND_SIZE];
    int next_arg = parse_word(command,command_to_call);
    for (int i = 0; i < NUMBER_OF_COMMANDS; i++) {
        if (string_compare_case_insensitive((char *)command_to_call, commands[i]) == 0) {
            (*execute_command[i])(command+next_arg);
        }
    }
}