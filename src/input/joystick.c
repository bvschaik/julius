#include "joystick.h"

#include "core/log.h"

#include <string.h>

typedef int joystick_axis;

typedef struct {
    int is_down;
    int went_down;
    int went_up;
} joystick_button;

typedef struct {
    joystick_button top;
    joystick_button left;
    joystick_button bottom;
    joystick_button right;
} joystick_hat;

typedef struct {
    int delta_x;
    int delta_y;
} joystick_trackball;

static struct {
    joystick_model connected_models[JOYSTICK_MAX_CONTROLLERS];
    struct {
        int id;
        int connected;
        joystick_model *model;
        joystick_axis axis[JOYSTICK_MAX_AXIS];
        joystick_button button[JOYSTICK_MAX_BUTTONS];
        joystick_trackball trackball[JOYSTICK_MAX_TRACKBALLS];
        joystick_hat hat[JOYSTICK_MAX_HATS];
    } joystick[JOYSTICK_MAX_CONTROLLERS];
} data;

joystick_model *joystick_get_model_by_guid(const char *guid)
{
    for (int i = 0; i < JOYSTICK_MAX_CONTROLLERS; ++i) {
        joystick_model *model = &data.connected_models[i];
        if (strcmp(guid, model->guid) == 0) {
            return model;
        }
    }
    return 0;
}

joystick_model *joystick_get_first_unused_model(void)
{
    for (int i = 0; i < JOYSTICK_MAX_CONTROLLERS; ++i) {
        joystick_model *model = &data.connected_models[i];
        if (!model->connected_joysticks) {
            return model;
        }
    }
    return 0;
}

static void update_button(joystick_button *button, int value)
{
    if (value) {
        if (!button->is_down) {
            button->went_down = 1;
            button->is_down = 1;
        }
    } else {
        if (button->is_down) {
            button->is_down = 0;
            button->went_up = 1;
        }
    }
}

static void update_hat(joystick_hat *hat, joystick_hat_position position)
{
    update_button(&hat->top, position & JOYSTICK_HAT_UP);
    update_button(&hat->left, position & JOYSTICK_HAT_LEFT);
    update_button(&hat->bottom, position & JOYSTICK_HAT_DOWN);
    update_button(&hat->right, position & JOYSTICK_HAT_RIGHT);
}

static void update_trackball(joystick_trackball *trackball, int delta_x, int delta_y)
{
    trackball->delta_x += delta_x;
    trackball->delta_y += delta_y;
}

static void joystick_reset_all_button_states(int slot)
{
    memset(data.joystick[slot].button, 0, sizeof(joystick_button) * JOYSTICK_MAX_BUTTONS);
}

static void joystick_reset_all_axis_states(int slot)
{
    memset(data.joystick[slot].axis, 0, sizeof(int) * JOYSTICK_MAX_AXIS);
}

static void joystick_reset_all_hat_states(int slot)
{
    memset(data.joystick[slot].hat, 0, sizeof(joystick_hat) * JOYSTICK_MAX_HATS);
}

static void joystick_reset_all_trackball_states(int slot)
{
    memset(data.joystick[slot].trackball, 0, sizeof(joystick_trackball) * JOYSTICK_MAX_TRACKBALLS);
}

static void joystick_reset_state(int slot)
{
    joystick_reset_all_axis_states(slot);
    joystick_reset_all_button_states(slot);
    joystick_reset_all_trackball_states(slot);
    joystick_reset_all_hat_states(slot);
}

static int get_free_joystick_slot(void)
{
    for (int i = 0; i < JOYSTICK_MAX_CONTROLLERS; ++i) {
        if (!data.joystick[i].connected) {
            return i;
        }
    }
    return JOYSTICK_NO_SLOT;
}

static int get_joystick_slot_from_id(int joystick_id)
{
    for (int i = 0; i < JOYSTICK_MAX_CONTROLLERS; ++i) {
        if (data.joystick[i].id == joystick_id) {
            return i;
        }
    }
    return JOYSTICK_NO_SLOT;
}

int joystick_add(int joystick_id, joystick_model *model)
{
    int slot = get_free_joystick_slot();
    if (slot != JOYSTICK_NO_SLOT) {
        data.joystick[slot].id = joystick_id;
        data.joystick[slot].model = model;
        data.joystick[slot].connected = 1;
        model->connected_joysticks++;
    }
    return slot;
}

int joystick_is_listened(int joystick_id)
{
    return get_joystick_slot_from_id(joystick_id) != JOYSTICK_NO_SLOT;
}

int joystick_remove(int joystick_id)
{
    int slot = get_joystick_slot_from_id(joystick_id);
    if (slot == JOYSTICK_NO_SLOT || !data.joystick[slot].connected) {
        return 0;
    }
    data.joystick[slot].connected = 0;
    data.joystick[slot].model->connected_joysticks--;
    data.joystick[slot].model = 0;
    joystick_reset_state(slot);
    return 1;
}

void joystick_update_element(int joystick_id, joystick_element element, int element_id, int value1, int value2)
{
    int slot = get_joystick_slot_from_id(joystick_id);
    if (slot == JOYSTICK_NO_SLOT) {
        return;
    }
    switch (element) {
        case JOYSTICK_ELEMENT_BUTTON:
            update_button(&data.joystick[slot].button[element_id], value1);
            break;
        case JOYSTICK_ELEMENT_HAT:
            update_hat(&data.joystick[slot].hat[element_id], value1);
            break;
        case JOYSTICK_ELEMENT_AXIS:
            data.joystick[slot].axis[element_id] = value1;
            break;
        case JOYSTICK_ELEMENT_TRACKBALL:
            update_trackball(&data.joystick[slot].trackball[element_id], value1, value2);
            break;
        default:
            log_info("Trying to update wrong joystick element", 0, element);
    }
}

static int get_input_for_mapping(int joystick_id, mapping_action *action, int *value, joystick_element *element)
{
    for (int j = 0; j < JOYSTICK_MAPPING_ELEMENTS_MAX; ++j) {
        joystick_element current_element = first_mapping[action].element[j].type;
        int current_value = 0;
        if (current_element == JOYSTICK_ELEMENT_NONE) {
            continue;
        }
        int element_id = first_mapping[action].element[j].id;
        int element_position = first_mapping[action].element[j].position;
        switch (current_element) {
            case JOYSTICK_ELEMENT_AXIS:
                current_value = data.joystick[i].axis[element_id];
                break;
            case JOYSTICK_ELEMENT_BUTTON:
                current_value = data.joystick[i].button[element_id].is_down;
                break;
            case JOYSTICK_ELEMENT_TRACKBALL:
                if (element_position == JOYSTICK_TRACKBALL_X) {
                    current_value = data.joystick[i].trackball[element_id].delta_x;
                } else {
                    current_value = data.joystick[i].trackball[element_id].delta_x;
                }
                break;
            case JOYSTICK_ELEMENT_HAT:
                switch
                    current_value = data.joystick[i].axis[element_id];
                break;
        }

        if (current_value == 0) {
            *value = 0;
            return;
        }
    }
}

static void get_joystick_input_for(mapping_action action, int *value, joystick_element *element)
{
    *value = 0;
    *element = JOYSTICK_ELEMENT_NONE;

    for (int i = 0; i < JOYSTICK_MAX_CONTROLLERS; ++i) {
        if (!data.joystick[i].connected) {
            continue;
        }
        if(get_input_for_mapping(i, &data.joystick[i].model->first_mapping[action], value, element) ||
           get_input_for_mapping(i, &data.joystick[i].model->second_mapping[action], value, element)) {
            return;
        }
    }
}

static int translate_mouse_cursor_position(void)
{
    int value_up, value_left, value_down, value_right;
    joystick_element element_up, element_left, element_down, element_right;

    get_joystick_input_for(MAPPING_ACTION_CURSOR_UP, &value_up, &element_up);
    get_joystick_input_for(MAPPING_ACTION_CURSOR_DOWN, &value_down, &element_down);
    get_joystick_input_for(MAPPING_ACTION_CURSOR_LEFT, &value_left, &element_left);
    get_joystick_input_for(MAPPING_ACTION_CURSOR_RIGHT, &value_right, &element_right);


}

int joystick_to_mouse_and_keyboard(void)
{
    int handled = 0;
    handled |= translate_mouse_cursor_position();
    handled |= translate_mouse_button_presses();
    handled |= translate_scrolling();
    handled |= translate_mapping_reset();
    return handled;
}
