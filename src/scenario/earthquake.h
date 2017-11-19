#ifndef SCENARIO_EARTHQUAKE_H
#define SCENARIO_EARTHQUAKE_H

#include "core/buffer.h"

void scenario_earthquake_init();

void scenario_earthquake_process();

int scenario_earthquake_is_in_progress();

void scenario_earthquake_save_state(buffer *buf);

void scenario_earthquake_load_state(buffer *buf);

#endif // SCENARIO_EARTHQUAKE_H
