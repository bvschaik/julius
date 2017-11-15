#ifndef SCENARIO_CRITERIA_H
#define SCENARIO_CRITERIA_H

#include "core/buffer.h"

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

void scenario_criteria_init_max_year();
int scenario_criteria_max_year();

void scenario_criteria_save_state(buffer *buf);

void scenario_criteria_load_state(buffer *buf);

#endif // SCENARIO_CRITERIA_H
