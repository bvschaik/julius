#include "victory.h"

#include "building/construction.h"
#include "city/data_private.h"
#include "city/figures.h"
#include "city/finance.h"
#include "city/message.h"
#include "game/time.h"
#include "scenario/criteria.h"
#include "scenario/property.h"
#include "sound/music.h"
#include "window/mission_end.h"
#include "window/victory_dialog.h"

static struct {
    int state;
    int force_win;
} data;

void city_victory_reset()
{
    data.state = VICTORY_STATE_NONE;
    data.force_win = 0;
}

void city_victory_force_win()
{
    data.force_win = 1;
}

int city_victory_state()
{
    return data.state;
}

void city_victory_check()
{
    if (scenario_is_open_play()) {
        return;
    }
    int num_criteria = 0;
    data.state = VICTORY_STATE_WON;
    if (scenario_criteria_culture_enabled()) {
        num_criteria++;
        if (city_data.ratings.culture < scenario_criteria_culture()) {
            data.state = VICTORY_STATE_NONE;
        }
    }
    if (scenario_criteria_prosperity_enabled()) {
        num_criteria++;
        if (city_data.ratings.prosperity < scenario_criteria_prosperity()) {
            data.state = VICTORY_STATE_NONE;
        }
    }
    if (scenario_criteria_peace_enabled()) {
        num_criteria++;
        if (city_data.ratings.peace < scenario_criteria_peace()) {
            data.state = VICTORY_STATE_NONE;
        }
    }
    if (scenario_criteria_favor_enabled()) {
        num_criteria++;
        if (city_data.ratings.favor < scenario_criteria_favor()) {
            data.state = VICTORY_STATE_NONE;
        }
    }
    if (scenario_criteria_population_enabled()) {
        num_criteria++;
        if (city_data.population.population < scenario_criteria_population()) {
            data.state = VICTORY_STATE_NONE;
        }
    }

    if (num_criteria <= 0) {
        data.state = VICTORY_STATE_NONE;
        if (scenario_criteria_time_limit_enabled()) {
            num_criteria++;
        }
        if (scenario_criteria_survival_enabled()) {
            num_criteria++;
        }
    }
    if (game_time_year() >= scenario_criteria_max_year()) {
        if (scenario_criteria_time_limit_enabled()) {
            data.state = VICTORY_STATE_LOST;
        } else if (scenario_criteria_survival_enabled()) {
            data.state = VICTORY_STATE_WON;
        }
    }
    if (city_figures_total_invading_enemies() > 2 + city_data.figure.soldiers) {
        if (city_data.population.population < city_data.population.highest_ever / 4) {
            data.state = VICTORY_STATE_LOST;
        }
    }
    if (city_figures_total_invading_enemies() > 0) {
        if (city_data.population.population <= 0) {
            data.state = VICTORY_STATE_LOST;
        }
    }
    if (num_criteria <= 0) {
        data.state = VICTORY_STATE_NONE;
    }
    if (city_data.mission.has_won) {
        data.state = city_data.mission.continue_months_left <= 0 ? VICTORY_STATE_WON : VICTORY_STATE_NONE;
    }
    if (data.force_win) {
        data.state = VICTORY_STATE_WON;
    }
    if (data.state != VICTORY_STATE_NONE) {
        building_construction_clear_type();
        if (data.state == VICTORY_STATE_LOST) {
            if (city_data.mission.fired_message_shown) {
                window_mission_end_show_fired();
            } else {
                city_data.mission.fired_message_shown = 1;
                city_message_post(1, MESSAGE_FIRED, 0, 0);
            }
            data.force_win = 0;
        } else if (data.state == VICTORY_STATE_WON) {
            sound_music_stop();
            if (city_data.mission.victory_message_shown) {
                window_mission_end_show_won();
                data.force_win = 0;
            } else {
                city_data.mission.victory_message_shown = 1;
                window_victory_dialog_show();
            }
        }
    }
}

void city_victory_update_months_to_govern()
{
    if (city_data.mission.has_won) {
        city_data.mission.continue_months_left--;
    }
}

void city_victory_continue_governing(int months)
{
    city_data.mission.has_won = 1;
    city_data.mission.continue_months_left += months;
    city_data.mission.continue_months_chosen = months;
    city_data.emperor.salary_rank = 0;
    city_data.emperor.salary_amount = 0;
    city_finance_update_salary();
}

void city_victory_stop_governing()
{
    city_data.mission.has_won = 0;
    city_data.mission.continue_months_left = 0;
    city_data.mission.continue_months_chosen = 0;
}

int city_victory_has_won()
{
    return city_data.mission.has_won;
}
