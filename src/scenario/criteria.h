#ifndef SCENARIO_CRITERIA_H
#define SCENARIO_CRITERIA_H

int scenario_is_open_play();

int scenario_criteria_population_enabled();
int scenario_criteria_population();

int scenario_criteria_culture_enabled();
int scenario_criteria_culture();

int scenario_criteria_prosperity_enabled();
int scenario_criteria_prosperity();

int scenario_criteria_peace_enabled();
int scenario_criteria_peace();

int scenario_criteria_favor_enabled();
int scenario_criteria_favor();

int scenario_criteria_time_limit_enabled();
int scenario_criteria_time_limit_years();

int scenario_criteria_survival_enabled();
int scenario_criteria_survival_years();

int scenario_criteria_milestone_year(int percentage);

#endif // SCENARIO_CRITERIA_H
