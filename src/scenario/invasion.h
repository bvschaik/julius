#ifndef SCENARIO_INVASION_H
#define SCENARIO_INVASION_H

#include "core/buffer.h"
#include "figure/formation.h"
#include "scenario/types.h"

void scenario_invasion_clear(void);
void scenario_invasion_init(void);

int scenario_invasion_exists_upcoming(void);

void scenario_invasion_foreach_warning(void (*callback)(int x, int y, int image_id));

int scenario_invasion_count(void);

int scenario_invasion_get_years_remaining(void);

int scenario_invasion_start_from_mars(void);

int scenario_invasion_start_from_caesar(int size);

void scenario_invasion_start_from_cheat(void);

void scenario_invasion_start_from_action(invasion_type_enum invasion_type, int size, int invasion_point, formation_attack_enum attack_type, enemy_type_t enemy_id);
void scenario_invasion_start_from_console(invasion_type_enum invasion_type, int size, int invasion_point);

void scenario_invasion_process(void);

void scenario_invasion_save_state(buffer *invasion_id, buffer *warnings);

void scenario_invasion_load_state(buffer *invasion_id, buffer *warnings);

#endif // SCENARIO_INVASION_H
