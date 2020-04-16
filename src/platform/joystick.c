#include "joystick.h"

#include "input/joystick.h"

#include <stdio.h>
#include <string.h>

static mapping_element first_default_mapping[MAPPING_ACTION_MAX] = {
    { { { JOYSTICK_ELEMENT_AXIS, 1, JOYSTICK_AXIS_NEGATIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, 0, JOYSTICK_AXIS_NEGATIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, 1, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, 0, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, 5, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, 2, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_BUTTON, 0 } } },
    { JOYSTICK_ELEMENT_NONE },
    { { { JOYSTICK_ELEMENT_BUTTON, 1 } } }
};

static mapping_element second_default_mapping[MAPPING_ACTION_MAX] = {
    { { { JOYSTICK_ELEMENT_HAT, 0, JOYSTICK_HAT_UP } } },
    { { { JOYSTICK_ELEMENT_HAT, 0, JOYSTICK_HAT_LEFT } } },
    { { { JOYSTICK_ELEMENT_HAT, 0, JOYSTICK_HAT_DOWN } } },
    { { { JOYSTICK_ELEMENT_HAT, 0, JOYSTICK_HAT_RIGHT } } }
};

static joystick_model *get_joystick_model(SDL_Joystick *joystick, int instance_id)
{
    static char guid[JOYSTICK_MAX_GUID];
    SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(joystick), guid, JOYSTICK_MAX_GUID);
    joystick_model *model = joystick_get_model_by_guid(guid);
    if (model) {
        return model;
    }
    model = joystick_get_first_unused_model();
    if (!model) {
        return 0;
    }
    snprintf(model->guid, JOYSTICK_MAX_GUID, "%s", guid);
    const char *name = SDL_JoystickName(joystick);
    if (!name) {
        snprintf(model->name, JOYSTICK_MAX_NAME, "Joysyick %d", instance_id);
    } else {
        snprintf(model->name, JOYSTICK_MAX_NAME, "%s", name);
    }
    model->total_axis = SDL_JoystickNumAxes(joystick);
    model->total_hats = SDL_JoystickNumHats(joystick);
    model->total_trackballs = SDL_JoystickNumBalls(joystick);
    model->total_buttons = SDL_JoystickNumButtons(joystick);
    memcpy(model->first_mapping, first_default_mapping, sizeof(mapping_element) * MAPPING_ACTION_MAX);
    memcpy(model->second_mapping, second_default_mapping, sizeof(mapping_element) * MAPPING_ACTION_MAX);

    return model;
}

static void add_joystick(int index)
{
    SDL_Joystick *joystick = SDL_JoystickOpen(index);
    int instance_id = SDL_JoystickInstanceID(joystick);
    if (joystick_is_listened(instance_id)) {
        return;
    }
    joystick_model *model = get_joystick_model(joystick, instance_id);
    if (!model) {
        SDL_JoystickClose(joystick);
        return;
    }
    if (!joystick_add(instance_id, model)) {
        SDL_JoystickClose(joystick);
    }
}

static void remove_joystick(int instance_id)
{
    if (!joystick_is_listened(instance_id)) {
        return;
    }
    SDL_JoystickClose(SDL_JoystickFromInstanceID(instance_id));
    joystick_remove(instance_id);
}

void platform_joystick_init(void)
{
    if (SDL_JoystickEventState(SDL_ENABLE) != SDL_ENABLE) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Joystick events could not be enabled: %s", SDL_GetError());
    }
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        add_joystick(i);
    }
}

void platform_joystick_device_changed(int id, int is_connected)
{
    if (is_connected) {
        add_joystick(id);
    } else {
        remove_joystick(id);
    }
}

void platform_joystick_handle_axis(SDL_JoyAxisEvent *event)
{
    joystick_update_element(event->which, JOYSTICK_ELEMENT_AXIS, event->axis, event->value, 0);
}

void platform_joystick_handle_trackball(SDL_JoyBallEvent *event)
{
    joystick_update_element(event->which, JOYSTICK_ELEMENT_TRACKBALL, event->ball, event->xrel, event->yrel);
}

void platform_joystick_handle_hat(SDL_JoyHatEvent *event)
{
    joystick_hat_position position = JOYSTICK_HAT_CENTERED;
    if (event->value & SDL_HAT_UP) {
        position |= JOYSTICK_HAT_UP;
    }
    if (event->value & SDL_HAT_DOWN) {
        position |= JOYSTICK_HAT_DOWN;
    }
    if (event->value & SDL_HAT_LEFT) {
        position |= JOYSTICK_HAT_LEFT;
    }
    if (event->value & SDL_HAT_RIGHT) {
        position |= JOYSTICK_HAT_RIGHT;
    }
    joystick_update_element(event->which, JOYSTICK_ELEMENT_HAT, event->hat, position, 0);
}

void platform_joystick_handle_button(SDL_JoyButtonEvent *event, int is_down)
{
    joystick_update_element(event->which, JOYSTICK_ELEMENT_BUTTON, event->button, is_down, 0);
}
