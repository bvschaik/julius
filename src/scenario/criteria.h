#ifndef SCENARIO_CRITERIA_H
#define SCENARIO_CRITERIA_H

#include "core/buffer.h"

int scenario_criteria_population_enabled(void);
int scenario_criteria_population(void);

int scenario_criteria_culture_enabled(void);
int scenario_criteria_culture(void);

int scenario_criteria_prosperity_enabled(void);
int scenario_criteria_prosperity(void);

int scenario_criteria_peace_enabled(void);
int scenario_criteria_peace(void);

int scenario_criteria_favor_enabled(void);
int scenario_criteria_favor(void);

int scenario_criteria_time_limit_enabled(void);
int scenario_criteria_time_limit_years(void);

int scenario_criteria_survival_enabled(void);
int scenario_criteria_survival_years(void);

int scenario_criteria_milestone_year(int percentage);

void scenario_criteria_init_max_year(void);
int scenario_criteria_max_year(void);

void scenario_criteria_save_state(buffer *buf);

void scenario_criteria_load_state(buffer *buf);

#endif // SCENARIO_CRITERIA_H
