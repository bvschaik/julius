#ifndef SCENARIO_SCENARIO_H
#define SCENARIO_SCENARIO_H

#include "core/buffer.h"

void scenario_settings_init();

void scenario_settings_init_mission();

void scenario_save_state(buffer *buf);

void scenario_load_state(buffer *buf);

void scenario_settings_save_state(buffer *part1, buffer *part2, buffer *part3);

void scenario_settings_load_state(buffer *part1, buffer *part2, buffer *part3);

#endif // SCENARIO_SCENARIO_H
