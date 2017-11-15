#include "criteria.h"

#include "Data/Scenario.h"

static int max_game_year;

int scenario_criteria_population_enabled()
{
    return Data_Scenario.winCriteria_populationEnabled;
}

int scenario_criteria_population()
{
    return Data_Scenario.winCriteria_population;
}

int scenario_criteria_culture_enabled()
{
    return Data_Scenario.winCriteria.cultureEnabled;
}

int scenario_criteria_culture()
{
    return Data_Scenario.winCriteria.culture;
}

int scenario_criteria_prosperity_enabled()
{
    return Data_Scenario.winCriteria.prosperityEnabled;
}

int scenario_criteria_prosperity()
{
    return Data_Scenario.winCriteria.prosperity;
}

int scenario_criteria_peace_enabled()
{
    return Data_Scenario.winCriteria.peaceEnabled;
}

int scenario_criteria_peace()
{
    return Data_Scenario.winCriteria.peace;
}

int scenario_criteria_favor_enabled()
{
    return Data_Scenario.winCriteria.favorEnabled;
}

int scenario_criteria_favor()
{
    return Data_Scenario.winCriteria.favor;
}

int scenario_criteria_time_limit_enabled()
{
    return Data_Scenario.winCriteria.timeLimitYearsEnabled;
}

int scenario_criteria_time_limit_years()
{
    return Data_Scenario.winCriteria.timeLimitYears;
}

int scenario_criteria_survival_enabled()
{
    return Data_Scenario.winCriteria.survivalYearsEnabled;
}

int scenario_criteria_survival_years()
{
    return Data_Scenario.winCriteria.survivalYears;
}

int scenario_criteria_milestone_year(int percentage)
{
    switch (percentage) {
        case 25:
            return Data_Scenario.startYear + Data_Scenario.milestone25;
        case 50:
            return Data_Scenario.startYear + Data_Scenario.milestone50;
        case 75:
            return Data_Scenario.startYear + Data_Scenario.milestone75;
    }
    return 0;
}

void scenario_criteria_init_max_year()
{
    if (Data_Scenario.winCriteria.timeLimitYearsEnabled) {
        max_game_year = Data_Scenario.startYear + Data_Scenario.winCriteria.timeLimitYears;
    } else if (Data_Scenario.winCriteria.survivalYearsEnabled) {
        max_game_year = Data_Scenario.startYear + Data_Scenario.winCriteria.survivalYears;
    } else {
        max_game_year = 1000000 + Data_Scenario.startYear;
    }
}

int scenario_criteria_max_year()
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
