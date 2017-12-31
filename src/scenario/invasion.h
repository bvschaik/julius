#ifndef SCENARIO_INVASION_H
#define SCENARIO_INVASION_H

#include "core/buffer.h"

void scenario_invasion_init();

int scenario_invasion_exists_upcoming();

void scenario_invasion_foreach_warning(void (*callback)(int x, int y, int image_id));

int scenario_invasion_count();

int scenario_invasion_start_from_mars();

int scenario_invasion_start_from_caesar(int size);

void scenario_invasion_start_from_cheat();

void scenario_invasion_process();

void scenario_invasion_save_state(buffer *invasion_id, buffer *warnings);

void scenario_invasion_load_state(buffer *invasion_id, buffer *warnings);

#endif // SCENARIO_INVASION_H
