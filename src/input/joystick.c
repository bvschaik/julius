#include "joystick.h"

#include "core/log.h"

#include <string.h>

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
        const joystick_model *model;
        int axis[JOYSTICK_MAX_AXIS];
        joystick_button button[JOYSTICK_MAX_BUTTONS];
        joystick_trackball trackball[JOYSTICK_MAX_TRACKBALLS];
        joystick_hat hat[JOYSTICK_MAX_HATS];
    } joystick[JOYSTICK_MAX_CONTROLLERS];
    int actions_used_this_frame[MAPPING_ACTION_MAX];
} data;

joystick_model *joystick_get_model_by_guid(const char *guid)
{
    for (int i = 0; i < JOYSTICK_MAX_CONTROLLERS; ++i) {
        joystick_model *model = joystick_get_model_by_index(i);
        if (strcmp(guid, model->guid) == 0) {
            return model;
        }
    }
    return 0;
}

joystick_model *joystick_get_model_by_index(int index)
{
    return &data.connected_models[index];
}

joystick_model *joystick_get_first_unused_model(void)
{
    for (int i = 0; i < JOYSTICK_MAX_CONTROLLERS; ++i) {
        joystick_model *model = joystick_get_model_by_index(i);
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
    update_button(&hat->top, position & JOYSTICK_HAT_TOP);
    update_button(&hat->left, position & JOYSTICK_HAT_LEFT);
    update_button(&hat->bottom, position & JOYSTICK_HAT_BOTTOM);
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

int joystick_add(int joystick_id, const joystick_model *model)
{
    int slot = get_free_joystick_slot();
    if (slot != JOYSTICK_NO_SLOT) {
        data.joystick[slot].id = joystick_id;
        data.joystick[slot].model = model;
    }
    return slot;
}

int joystick_remove(int joystick_id)
{
    int slot = get_joystick_slot_from_id(joystick_id);
    if (slot == JOYSTICK_NO_SLOT || !data.joystick[slot].connected) {
        return 0;
    }
    data.joystick[slot].connected = 0;
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

void joystick_to_mouse_and_keyboard(void)
{}
