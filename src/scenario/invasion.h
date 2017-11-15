#ifndef SCENARIO_INVASION_H
#define SCENARIO_INVASION_H

#include "core/buffer.h"

void scenario_invasion_init();

int scenario_invasion_exists_upcoming();

int scenario_invasion_count();

int scenario_invasion_start_from_mars();

void scenario_invasion_start_from_cheat();

void scenario_invasion_process();

void scenario_invasion_process_caesar();

void scenario_invasion_save_state(buffer *buf);

void scenario_invasion_load_state(buffer *buf);

#endif // SCENARIO_INVASION_H
