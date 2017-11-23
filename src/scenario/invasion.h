#ifndef SCENARIO_INVASION_H
#define SCENARIO_INVASION_H

#include "core/buffer.h"

void scenario_invasion_init();

int scenario_invasion_start_from_mars();

void scenario_invasion_start_from_cheat();

void scenario_invasion_process();

void scenario_invasion_process_caesar();

void scenario_invasion_save_state(buffer *invasion_id, buffer *warnings);

void scenario_invasion_load_state(buffer *invasion_id, buffer *warnings);

int scenario_invasion_exists_upcoming();
int scenario_invasion_count();

void scenario_invasion_foreach_warning(void (*callback)(int x, int y, int image_id));

#endif // SCENARIO_INVASION_H
