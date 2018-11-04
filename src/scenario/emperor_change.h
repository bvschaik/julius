#ifndef SCENARIO_EMPEROR_CHANGE_H
#define SCENARIO_EMPEROR_CHANGE_H

#include "core/buffer.h"

void scenario_emperor_change_init(void);

void scenario_emperor_change_process(void);

void scenario_emperor_change_save_state(buffer *time, buffer *state);

void scenario_emperor_change_load_state(buffer *time, buffer *state);

#endif // SCENARIO_EMPEROR_CHANGE_H
