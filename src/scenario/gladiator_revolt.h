#ifndef SCENARIO_GLADIATOR_REVOLT_H
#define SCENARIO_GLADIATOR_REVOLT_H

#include "core/buffer.h"

void scenario_gladiator_revolt_init(void);

void scenario_gladiator_revolt_process(void);

int scenario_gladiator_revolt_is_in_progress(void);

int scenario_gladiator_revolt_is_finished(void);

void scenario_gladiator_revolt_save_state(buffer *buf);

void scenario_gladiator_revolt_load_state(buffer *buf);

#endif // SCENARIO_GLADIATOR_REVOLT_H
