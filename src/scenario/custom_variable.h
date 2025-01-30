#ifndef SCENARIO_CUSTOM_VARIABLE_H
#define SCENARIO_CUSTOM_VARIABLE_H

#include "core/buffer.h"

#define MAX_ORIGINAL_CUSTOM_VARIABLES 100
#define CUSTOM_VARIABLE_NAME_LENGTH 64

unsigned int scenario_custom_variable_create(const uint8_t *name, int initial_value);

void scenario_custom_variable_delete(unsigned int id);
void scenario_custom_variable_delete_all(void);

unsigned int scenario_custom_variable_get_id_by_name(const uint8_t *name);

unsigned int scenario_custom_variable_count(void);
int scenario_custom_variable_exists(unsigned int id);

const uint8_t *scenario_custom_variable_get_name(unsigned int id);
void scenario_custom_variable_rename(unsigned int id, const uint8_t *name);

int scenario_custom_variable_get_value(unsigned int id);
void scenario_custom_variable_set_value(unsigned int id, int new_value);

void scenario_custom_variable_save_state(buffer *buf);
void scenario_custom_variable_load_state(buffer *buf);
void scenario_custom_variable_load_state_old_version(buffer *buf);

#endif // SCENARIO_CUSTOM_VARIABLE_H
