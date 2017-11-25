#ifndef SCENARIO_H
#define SCENARIO_H

struct buffer;

void Scenario_initialize(const char *scenarioName);
void scenario_load_state(buffer *buf);
void scenario_save_state(buffer *buf);

#endif
