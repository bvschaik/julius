#include "criteria.h"

#include "scenario/data.h"

static int max_game_year;

int scenario_criteria_population_enabled(void)
{
    return scenario.win_criteria.population.enabled;
}

int scenario_criteria_population(void)
{
    return scenario.win_criteria.population.goal;
}

int scenario_criteria_culture_enabled(void)
{
    return scenario.win_criteria.culture.enabled;
}

int scenario_criteria_culture(void)
{
    return scenario.win_criteria.culture.goal;
}

int scenario_criteria_prosperity_enabled(void)
{
    return scenario.win_criteria.prosperity.enabled;
}

int scenario_criteria_prosperity(void)
{
    return scenario.win_criteria.prosperity.goal;
}

int scenario_criteria_peace_enabled(void)
{
    return scenario.win_criteria.peace.enabled;
}

int scenario_criteria_peace(void)
{
    return scenario.win_criteria.peace.goal;
}

int scenario_criteria_favor_enabled(void)
{
    return scenario.win_criteria.favor.enabled;
}

int scenario_criteria_favor(void)
{
    return scenario.win_criteria.favor.goal;
}

int scenario_criteria_time_limit_enabled(void)
{
    return scenario.win_criteria.time_limit.enabled;
}

int scenario_criteria_time_limit_years(void)
{
    return scenario.win_criteria.time_limit.years;
}

int scenario_criteria_survival_enabled(void)
{
    return scenario.win_criteria.survival_time.enabled;
}

int scenario_criteria_survival_years(void)
{
    return scenario.win_criteria.survival_time.years;
}

int scenario_criteria_milestone_year(int percentage)
{
    switch (percentage) {
        case 25:
            return scenario.start_year + scenario.win_criteria.milestone25_year;
        case 50:
            return scenario.start_year + scenario.win_criteria.milestone50_year;
        case 75:
            return scenario.start_year + scenario.win_criteria.milestone75_year;
        default:
            return 0;
    }
}

void scenario_criteria_init_max_year(void)
{
    if (scenario.win_criteria.time_limit.enabled) {
        max_game_year = scenario.start_year + scenario.win_criteria.time_limit.years;
    } else if (scenario.win_criteria.survival_time.enabled) {
        max_game_year = scenario.start_year + scenario.win_criteria.survival_time.years;
    } else {
        max_game_year = 1000000 + scenario.start_year;
    }
}

int scenario_criteria_max_year(void)
{
    return max_game_year;
}

void scenario_criteria_save_state(buffer *buf)
{
    buffer_write_i32(buf, max_game_year);
}

void scenario_criteria_load_state(buffer *buf)
{
    max_game_year = buffer_read_i32(buf);
}
