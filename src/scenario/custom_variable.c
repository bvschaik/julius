#include "custom_variable.h"

#include "core/array.h"
#include "core/log.h"
#include "core/string.h"
#include "scenario/message_media_text_blob.h"

typedef struct {
    unsigned int id;
    int in_use;
    int value;
    uint8_t name[CUSTOM_VARIABLE_NAME_LENGTH];
} custom_variable_t;

static array(custom_variable_t) custom_variables;

#define CUSTOM_VARIABLES_SIZE_STEP 8

static void new_variable(custom_variable_t *variable, unsigned int index)
{
    variable->id = index;
}

static int variable_in_use(const custom_variable_t *variable)
{
    return variable->in_use;
}

unsigned int scenario_custom_variable_create(const uint8_t *name, int initial_value)
{
    custom_variable_t *variable;
    array_new_item_after_index(custom_variables, 1, variable);
    if (!variable) {
        return 0;
    }
    variable->in_use = 1;
    variable->value = initial_value;
    if (name) {
        string_copy(name, variable->name, CUSTOM_VARIABLE_NAME_LENGTH);
    }

    return variable->id;
}

void scenario_custom_variable_delete_all(void)
{
    array_init(custom_variables, CUSTOM_VARIABLES_SIZE_STEP, new_variable, variable_in_use);
    array_advance(custom_variables);
}

unsigned int scenario_custom_variable_get_id_by_name(const uint8_t *name)
{
    const custom_variable_t *variable;
    array_foreach(custom_variables, variable) {
        if (name && string_equals(variable->name, name)) {
            return variable->id;
        }
    }
    return 0;
}

static custom_variable_t *get_variable(unsigned int id)
{
    if (id < 1 || id >= custom_variables.size) {
        return 0;
    }
    return array_item(custom_variables, id);
}

unsigned int scenario_custom_variable_count(void)
{
    return custom_variables.size;
}

int scenario_custom_variable_exists(unsigned int id)
{
    const custom_variable_t *variable = get_variable(id);
    return variable && variable->in_use;
}

void scenario_custom_variable_delete(unsigned int id)
{
    custom_variable_t *variable = get_variable(id);
    if (variable) {
        variable->in_use = 0;
    }
}

const uint8_t *scenario_custom_variable_get_name(unsigned int id)
{
    const custom_variable_t *variable = get_variable(id);
    return variable ? variable->name : 0;
}

void scenario_custom_variable_rename(unsigned int id, const uint8_t *name)
{
    custom_variable_t *variable = get_variable(id);
    if (!variable) {
        return;
    }
    if (!name || !*name) {
        variable->name[0] = 0;
    }
    string_copy(name, variable->name, CUSTOM_VARIABLE_NAME_LENGTH);
}

int scenario_custom_variable_get_value(unsigned int id)
{
    const custom_variable_t *variable = get_variable(id);
    return variable ? variable->value : 0;
}

void scenario_custom_variable_set_value(unsigned int id, int new_value)
{
    custom_variable_t *variable = get_variable(id);
    if (!variable) {
        return;
    }
    variable->value = new_value;
}

void scenario_custom_variable_save_state(buffer *buf)
{
    uint32_t struct_size = sizeof(int32_t) + sizeof(uint8_t) + sizeof(uint8_t) * CUSTOM_VARIABLE_NAME_LENGTH;
    buffer_init_dynamic_array(buf, custom_variables.size, struct_size);

    const custom_variable_t *variable;
    array_foreach(custom_variables, variable) {
        buffer_write_u8(buf, variable->in_use);
        buffer_write_i32(buf, variable->value);
        buffer_write_raw(buf, variable->name, CUSTOM_VARIABLE_NAME_LENGTH);
    }
}

void scenario_custom_variable_load_state(buffer *buf)
{
    unsigned int total_variables = buffer_load_dynamic_array(buf);

    if (!array_init(custom_variables, CUSTOM_VARIABLES_SIZE_STEP, new_variable, variable_in_use) ||
        !array_expand(custom_variables, total_variables)) {
        log_error("Failed to initialize custom variables array - out of memory. The game will probably crash.", 0, 0);
        return;
    }
    for (unsigned int i = 0; i < total_variables; i++) {
        custom_variable_t *variable = array_next(custom_variables);
        variable->in_use = buffer_read_u8(buf);
        variable->value = buffer_read_i32(buf);
        buffer_read_raw(buf, variable->name, CUSTOM_VARIABLE_NAME_LENGTH);
    }
}

void scenario_custom_variable_load_state_old_version(buffer *buf)
{
    if (!array_init(custom_variables, CUSTOM_VARIABLES_SIZE_STEP, new_variable, variable_in_use) ||
        !array_expand(custom_variables, MAX_ORIGINAL_CUSTOM_VARIABLES)) {
        log_error("Failed to initialize custom variables array - out of memory. The game will probably crash.", 0, 0);
        return;
    }
    
    for (unsigned int i = 0; i < MAX_ORIGINAL_CUSTOM_VARIABLES; i++) {
        custom_variable_t *variable = array_next(custom_variables);
        variable->in_use = buffer_read_u8(buf);
        variable->value = buffer_read_i32(buf);
        int name_link = buffer_read_i32(buf);
        if (!variable->in_use) {
            name_link = 0;
            variable->value = 0;
        }
        if (!name_link) {
            continue;
        }
        const text_blob_string_t *text = message_media_text_blob_get_entry(name_link);
        if (!text) {
            continue;
        }
        string_copy(text->text, variable->name, CUSTOM_VARIABLE_NAME_LENGTH);
        message_media_text_blob_mark_entry_as_unused(text);
    }
    array_trim(custom_variables);
    message_media_text_blob_remove_unused();
}
