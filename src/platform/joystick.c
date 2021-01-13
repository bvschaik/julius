#include "joystick.h"

#include "input/joystick.h"
#include "input/touch.h"
#include "platform/platform.h"

#include <stdio.h>
#include <string.h>

#ifdef __vita__
#include "platform/vita/pad.h"

static mapping_element first_default_mapping[MAPPING_ACTION_MAX] = {
    { { { JOYSTICK_ELEMENT_AXIS, VITA_LEFT_ANALOG_Y, JOYSTICK_AXIS_NEGATIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, VITA_LEFT_ANALOG_X, JOYSTICK_AXIS_NEGATIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, VITA_LEFT_ANALOG_Y, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, VITA_LEFT_ANALOG_X, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_BUTTON, VITA_PAD_L } } },
    { { { JOYSTICK_ELEMENT_BUTTON, VITA_PAD_R } } },
    { { { JOYSTICK_ELEMENT_BUTTON, VITA_PAD_CROSS } } },
    { { { JOYSTICK_ELEMENT_BUTTON, VITA_PAD_CIRCLE } } },
    { { { JOYSTICK_ELEMENT_AXIS, VITA_RIGHT_ANALOG_Y, JOYSTICK_AXIS_NEGATIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, VITA_RIGHT_ANALOG_Y, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, VITA_RIGHT_ANALOG_Y, JOYSTICK_AXIS_NEGATIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, VITA_RIGHT_ANALOG_X, JOYSTICK_AXIS_NEGATIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, VITA_RIGHT_ANALOG_Y, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, VITA_RIGHT_ANALOG_X, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_NONE } } },
    { { { JOYSTICK_ELEMENT_NONE } } },
    { { { JOYSTICK_ELEMENT_BUTTON, VITA_PAD_TRIANGLE } } },
    { { { JOYSTICK_ELEMENT_BUTTON, VITA_PAD_SQUARE } } },
    { { { JOYSTICK_ELEMENT_NONE } } },
    { { { JOYSTICK_ELEMENT_NONE } } },
    { { { JOYSTICK_ELEMENT_BUTTON, VITA_PAD_START } } },
    { { { JOYSTICK_ELEMENT_BUTTON, VITA_PAD_SELECT } } },
    { { { JOYSTICK_ELEMENT_BUTTON, VITA_PAD_L }, { JOYSTICK_ELEMENT_BUTTON, VITA_PAD_R } } },
};
static mapping_element second_default_mapping[MAPPING_ACTION_MAX] = {
    { { { JOYSTICK_ELEMENT_BUTTON, VITA_PAD_UP } } },
    { { { JOYSTICK_ELEMENT_BUTTON, VITA_PAD_LEFT } } },
    { { { JOYSTICK_ELEMENT_BUTTON, VITA_PAD_DOWN } } },
    { { { JOYSTICK_ELEMENT_BUTTON, VITA_PAD_RIGHT } } }
};
#elif defined(__SWITCH__)
#include "platform/switch/pad.h"

static mapping_element first_default_mapping[MAPPING_ACTION_MAX] = {
    { { { JOYSTICK_ELEMENT_AXIS, SWITCH_LEFT_ANALOG_Y, JOYSTICK_AXIS_NEGATIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, SWITCH_LEFT_ANALOG_X, JOYSTICK_AXIS_NEGATIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, SWITCH_LEFT_ANALOG_Y, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, SWITCH_LEFT_ANALOG_X, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_L } } },
    { { { JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_R } } },
    { { { JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_A } } },
    { { { JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_B } } },
    { { { JOYSTICK_ELEMENT_AXIS, SWITCH_RIGHT_ANALOG_Y, JOYSTICK_AXIS_NEGATIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, SWITCH_RIGHT_ANALOG_Y, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, SWITCH_RIGHT_ANALOG_Y, JOYSTICK_AXIS_NEGATIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, SWITCH_RIGHT_ANALOG_X, JOYSTICK_AXIS_NEGATIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, SWITCH_RIGHT_ANALOG_Y, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, SWITCH_RIGHT_ANALOG_X, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_ZL } } },
    { { { JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_ZR } } },
    { { { JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_X } } },
    { { { JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_Y } } },
    { { { JOYSTICK_ELEMENT_BUTTON, SWITCH_RSTICK } } },
    { { { JOYSTICK_ELEMENT_BUTTON, SWITCH_LSTICK } } },
    { { { JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_PLUS } } },
    { { { JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_MINUS } } },
    { { { JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_L }, { JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_R } } },
};
static mapping_element second_default_mapping[MAPPING_ACTION_MAX] = {
    { { { JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_UP } } },
    { { { JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_LEFT } } },
    { { { JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_DOWN } } },
    { { { JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_RIGHT } } }
};
#else
static mapping_element first_default_mapping[MAPPING_ACTION_MAX] = {
    { { { JOYSTICK_ELEMENT_AXIS, 1, JOYSTICK_AXIS_NEGATIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, 0, JOYSTICK_AXIS_NEGATIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, 1, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, 0, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, 5, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, 2, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_BUTTON, 0 } } },
    { { { JOYSTICK_ELEMENT_BUTTON, 1 } } },
    { { { JOYSTICK_ELEMENT_AXIS, 4, JOYSTICK_AXIS_NEGATIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, 4, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, 4, JOYSTICK_AXIS_NEGATIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, 3, JOYSTICK_AXIS_NEGATIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, 4, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_AXIS, 3, JOYSTICK_AXIS_POSITIVE } } },
    { { { JOYSTICK_ELEMENT_BUTTON, 5 } } },
    { { { JOYSTICK_ELEMENT_BUTTON, 4 } } },
    { { { JOYSTICK_ELEMENT_BUTTON, 2 } } },
    { { { JOYSTICK_ELEMENT_BUTTON, 3 } } }
};

static mapping_element second_default_mapping[MAPPING_ACTION_MAX] = {
    { { { JOYSTICK_ELEMENT_HAT, 0, JOYSTICK_HAT_UP } } },
    { { { JOYSTICK_ELEMENT_HAT, 0, JOYSTICK_HAT_LEFT } } },
    { { { JOYSTICK_ELEMENT_HAT, 0, JOYSTICK_HAT_DOWN } } },
    { { { JOYSTICK_ELEMENT_HAT, 0, JOYSTICK_HAT_RIGHT } } }
};
#endif

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
    SDL_Joystick *joystick = 0;
#if SDL_VERSION_ATLEAST(2, 0, 4)
    if (platform_sdl_version_at_least(2, 0, 4)) {
        joystick = SDL_JoystickFromInstanceID(instance_id);
    } else {
#endif
        for (int i = 0; i < SDL_NumJoysticks(); ++i) {
            SDL_Joystick *current_joystick = SDL_JoystickOpen(i);
            if (SDL_JoystickInstanceID(current_joystick) == instance_id) {
                joystick = current_joystick;
                break;
            }
        }
#if SDL_VERSION_ATLEAST(2, 0, 4)
    }
#endif
    if (joystick) {
        SDL_JoystickClose(joystick);
    }
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