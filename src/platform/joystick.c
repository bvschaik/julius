#include "joystick.h"

#include "input/joystick.h"
#include "input/touch.h"
#include "platform/platform.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    mapping_action action;
    struct {
        SDL_GameControllerBindType type;
        int id;
        int position;
    } element[JOYSTICK_MAPPING_ELEMENTS_MAX];
} controller_mapping;

static int enabled;

#ifdef __vita__
#include "platform/vita/pad.h"

static mapping_element default_mapping[] = {
    {MAPPING_ACTION_MOUSE_CURSOR_UP, {{JOYSTICK_ELEMENT_AXIS, VITA_LEFT_ANALOG_Y, JOYSTICK_AXIS_NEGATIVE}}},
    {MAPPING_ACTION_MOUSE_CURSOR_LEFT, {{JOYSTICK_ELEMENT_AXIS, VITA_LEFT_ANALOG_X, JOYSTICK_AXIS_NEGATIVE}}},
    {MAPPING_ACTION_MOUSE_CURSOR_DOWN, {{JOYSTICK_ELEMENT_AXIS, VITA_LEFT_ANALOG_Y, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_MOUSE_CURSOR_RIGHT, {{JOYSTICK_ELEMENT_AXIS, VITA_LEFT_ANALOG_X, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_FASTER_MOUSE_CURSOR_SPEED, {{JOYSTICK_ELEMENT_BUTTON, VITA_PAD_L}}},
    {MAPPING_ACTION_SLOWER_MOUSE_CURSOR_SPEED, {{JOYSTICK_ELEMENT_BUTTON, VITA_PAD_R}}},
    {MAPPING_ACTION_LEFT_MOUSE_BUTTON, {{JOYSTICK_ELEMENT_BUTTON, VITA_PAD_CROSS}}},
    {MAPPING_ACTION_RIGHT_MOUSE_BUTTON, {{JOYSTICK_ELEMENT_BUTTON, VITA_PAD_CIRCLE}}},
    {MAPPING_ACTION_SCROLL_WINDOW_UP, {{JOYSTICK_ELEMENT_AXIS, VITA_RIGHT_ANALOG_Y, JOYSTICK_AXIS_NEGATIVE}}},
    {MAPPING_ACTION_SCROLL_WINDOW_DOWN, {{JOYSTICK_ELEMENT_AXIS, VITA_RIGHT_ANALOG_Y, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_SCROLL_MAP_UP, {{JOYSTICK_ELEMENT_AXIS, VITA_RIGHT_ANALOG_Y, JOYSTICK_AXIS_NEGATIVE}}},
    {MAPPING_ACTION_SCROLL_MAP_LEFT, {{JOYSTICK_ELEMENT_AXIS, VITA_RIGHT_ANALOG_X, JOYSTICK_AXIS_NEGATIVE}}},
    {MAPPING_ACTION_SCROLL_MAP_DOWN, {{JOYSTICK_ELEMENT_AXIS, VITA_RIGHT_ANALOG_Y, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_SCROLL_MAP_RIGHT, {{JOYSTICK_ELEMENT_AXIS, VITA_RIGHT_ANALOG_X, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_INCREASE_GAME_SPEED, {{JOYSTICK_ELEMENT_BUTTON, VITA_PAD_TRIANGLE}}},
    {MAPPING_ACTION_DECREASE_GAME_SPEED, {{JOYSTICK_ELEMENT_BUTTON, VITA_PAD_SQUARE}}},
    {MAPPING_ACTION_SHOW_VIRTUAL_KEYBOARD, {{JOYSTICK_ELEMENT_BUTTON, VITA_PAD_START}}},
    {MAPPING_ACTION_CYCLE_TOUCH_TYPE, {{JOYSTICK_ELEMENT_BUTTON, VITA_PAD_SELECT}}},
    {MAPPING_ACTION_MOUSE_CURSOR_UP, {{JOYSTICK_ELEMENT_BUTTON, VITA_PAD_UP}}},
    {MAPPING_ACTION_MOUSE_CURSOR_LEFT, {{JOYSTICK_ELEMENT_BUTTON, VITA_PAD_LEFT}}},
    {MAPPING_ACTION_MOUSE_CURSOR_DOWN, {{JOYSTICK_ELEMENT_BUTTON, VITA_PAD_DOWN}}},
    {MAPPING_ACTION_MOUSE_CURSOR_RIGHT, {{JOYSTICK_ELEMENT_BUTTON, VITA_PAD_RIGHT}}},
    {MAPPING_ACTION_RESET_MAPPING,
        {{JOYSTICK_ELEMENT_BUTTON, VITA_PAD_L}, {JOYSTICK_ELEMENT_BUTTON, VITA_PAD_R}
    }},
};
#elif defined(__SWITCH__)
#include "platform/switch/pad.h"

static mapping_element default_mapping[] = {
    {MAPPING_ACTION_MOUSE_CURSOR_UP, {{JOYSTICK_ELEMENT_AXIS, SWITCH_LEFT_ANALOG_Y, JOYSTICK_AXIS_NEGATIVE}}},
    {MAPPING_ACTION_MOUSE_CURSOR_LEFT, {{JOYSTICK_ELEMENT_AXIS, SWITCH_LEFT_ANALOG_X, JOYSTICK_AXIS_NEGATIVE}}},
    {MAPPING_ACTION_MOUSE_CURSOR_DOWN, {{JOYSTICK_ELEMENT_AXIS, SWITCH_LEFT_ANALOG_Y, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_MOUSE_CURSOR_RIGHT, {{JOYSTICK_ELEMENT_AXIS, SWITCH_LEFT_ANALOG_X, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_FASTER_MOUSE_CURSOR_SPEED, {{JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_L}}},
    {MAPPING_ACTION_SLOWER_MOUSE_CURSOR_SPEED, {{JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_R}}},
    {MAPPING_ACTION_LEFT_MOUSE_BUTTON, {{JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_A}}},
    {MAPPING_ACTION_RIGHT_MOUSE_BUTTON, {{JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_B}}},
    {MAPPING_ACTION_SCROLL_WINDOW_UP, {{JOYSTICK_ELEMENT_AXIS, SWITCH_RIGHT_ANALOG_Y, JOYSTICK_AXIS_NEGATIVE}}},
    {MAPPING_ACTION_SCROLL_WINDOW_DOWN, {{JOYSTICK_ELEMENT_AXIS, SWITCH_RIGHT_ANALOG_Y, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_SCROLL_MAP_UP, {{JOYSTICK_ELEMENT_AXIS, SWITCH_RIGHT_ANALOG_Y, JOYSTICK_AXIS_NEGATIVE}}},
    {MAPPING_ACTION_SCROLL_MAP_LEFT, {{JOYSTICK_ELEMENT_AXIS, SWITCH_RIGHT_ANALOG_X, JOYSTICK_AXIS_NEGATIVE}}},
    {MAPPING_ACTION_SCROLL_MAP_DOWN, {{JOYSTICK_ELEMENT_AXIS, SWITCH_RIGHT_ANALOG_Y, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_SCROLL_MAP_RIGHT, {{JOYSTICK_ELEMENT_AXIS, SWITCH_RIGHT_ANALOG_X, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_ROTATE_MAP_LEFT, {{JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_ZL}}},
    {MAPPING_ACTION_ROTATE_MAP_RIGHT, {{JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_ZR}}},
    {MAPPING_ACTION_INCREASE_GAME_SPEED, {{JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_X}}},
    {MAPPING_ACTION_DECREASE_GAME_SPEED, {{JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_Y}}},
    {MAPPING_ACTION_TOGGLE_PAUSE, {{JOYSTICK_ELEMENT_BUTTON, SWITCH_RSTICK}}},
    {MAPPING_ACTION_CYCLE_LEGION, {{JOYSTICK_ELEMENT_BUTTON, SWITCH_LSTICK}}},
    {MAPPING_ACTION_SHOW_VIRTUAL_KEYBOARD, {{JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_PLUS}}},
    {MAPPING_ACTION_CYCLE_TOUCH_TYPE, {{JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_MINUS}}},
    {MAPPING_ACTION_MOUSE_CURSOR_UP, {{JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_UP}}},
    {MAPPING_ACTION_MOUSE_CURSOR_LEFT, {{JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_LEFT}}},
    {MAPPING_ACTION_MOUSE_CURSOR_DOWN, {{JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_DOWN}}},
    {MAPPING_ACTION_MOUSE_CURSOR_RIGHT, {{JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_RIGHT}}},
    {MAPPING_ACTION_RESET_MAPPING, {
        {JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_L}, {JOYSTICK_ELEMENT_BUTTON, SWITCH_PAD_R}
    }}
};
#else
static mapping_element default_mapping[] = {
    {MAPPING_ACTION_MOUSE_CURSOR_UP, {{JOYSTICK_ELEMENT_AXIS, 1, JOYSTICK_AXIS_NEGATIVE}}},
    {MAPPING_ACTION_MOUSE_CURSOR_LEFT, {{JOYSTICK_ELEMENT_AXIS, 0, JOYSTICK_AXIS_NEGATIVE}}},
    {MAPPING_ACTION_MOUSE_CURSOR_DOWN, {{JOYSTICK_ELEMENT_AXIS, 1, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_MOUSE_CURSOR_RIGHT, {{JOYSTICK_ELEMENT_AXIS, 0, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_FASTER_MOUSE_CURSOR_SPEED, {{JOYSTICK_ELEMENT_AXIS, 5, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_SLOWER_MOUSE_CURSOR_SPEED, {{JOYSTICK_ELEMENT_AXIS, 2, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_LEFT_MOUSE_BUTTON, {{JOYSTICK_ELEMENT_BUTTON, 0}}},
    {MAPPING_ACTION_RIGHT_MOUSE_BUTTON, {{JOYSTICK_ELEMENT_BUTTON, 1}}},
    {MAPPING_ACTION_SCROLL_WINDOW_UP, {{JOYSTICK_ELEMENT_AXIS, 4, JOYSTICK_AXIS_NEGATIVE}}},
    {MAPPING_ACTION_SCROLL_WINDOW_DOWN, {{JOYSTICK_ELEMENT_AXIS, 4, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_SCROLL_MAP_UP, {{JOYSTICK_ELEMENT_AXIS, 4, JOYSTICK_AXIS_NEGATIVE}}},
    {MAPPING_ACTION_SCROLL_MAP_LEFT, {{JOYSTICK_ELEMENT_AXIS, 3, JOYSTICK_AXIS_NEGATIVE}}},
    {MAPPING_ACTION_SCROLL_MAP_DOWN, {{JOYSTICK_ELEMENT_AXIS, 4, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_SCROLL_MAP_RIGHT, {{JOYSTICK_ELEMENT_AXIS, 3, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_ROTATE_MAP_LEFT, {{JOYSTICK_ELEMENT_BUTTON, 5}}},
    {MAPPING_ACTION_ROTATE_MAP_RIGHT, {{JOYSTICK_ELEMENT_BUTTON, 4}}},
    {MAPPING_ACTION_INCREASE_GAME_SPEED, {{JOYSTICK_ELEMENT_BUTTON, 2}}},
    {MAPPING_ACTION_DECREASE_GAME_SPEED, {{JOYSTICK_ELEMENT_BUTTON, 3}}},
    {MAPPING_ACTION_MOUSE_CURSOR_UP, {{JOYSTICK_ELEMENT_HAT, 0, JOYSTICK_HAT_UP}}},
    {MAPPING_ACTION_MOUSE_CURSOR_LEFT, {{JOYSTICK_ELEMENT_HAT, 0, JOYSTICK_HAT_LEFT}}},
    {MAPPING_ACTION_MOUSE_CURSOR_DOWN, {{JOYSTICK_ELEMENT_HAT, 0, JOYSTICK_HAT_DOWN}}},
    {MAPPING_ACTION_MOUSE_CURSOR_RIGHT, {{JOYSTICK_ELEMENT_HAT, 0, JOYSTICK_HAT_RIGHT}}}
};
#endif

static controller_mapping controller_mappings[] = {
    {MAPPING_ACTION_MOUSE_CURSOR_UP, {{SDL_CONTROLLER_BINDTYPE_AXIS, SDL_CONTROLLER_AXIS_LEFTY, JOYSTICK_AXIS_NEGATIVE}}},
    {MAPPING_ACTION_MOUSE_CURSOR_LEFT, {{SDL_CONTROLLER_BINDTYPE_AXIS, SDL_CONTROLLER_AXIS_LEFTX, JOYSTICK_AXIS_NEGATIVE}}},
    {MAPPING_ACTION_MOUSE_CURSOR_DOWN, {{SDL_CONTROLLER_BINDTYPE_AXIS, SDL_CONTROLLER_AXIS_LEFTY, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_MOUSE_CURSOR_RIGHT, {{SDL_CONTROLLER_BINDTYPE_AXIS, SDL_CONTROLLER_AXIS_LEFTX, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_FASTER_MOUSE_CURSOR_SPEED, {{SDL_CONTROLLER_BINDTYPE_AXIS, SDL_CONTROLLER_AXIS_TRIGGERLEFT, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_SLOWER_MOUSE_CURSOR_SPEED, {{SDL_CONTROLLER_BINDTYPE_AXIS, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_LEFT_MOUSE_BUTTON, {{SDL_CONTROLLER_BINDTYPE_BUTTON, SDL_CONTROLLER_BUTTON_A}}},
    {MAPPING_ACTION_RIGHT_MOUSE_BUTTON, {{SDL_CONTROLLER_BINDTYPE_BUTTON, SDL_CONTROLLER_BUTTON_B}}},
    {MAPPING_ACTION_SCROLL_WINDOW_UP, {{SDL_CONTROLLER_BINDTYPE_AXIS, SDL_CONTROLLER_AXIS_RIGHTY, JOYSTICK_AXIS_NEGATIVE}}},
    {MAPPING_ACTION_SCROLL_WINDOW_DOWN, {{SDL_CONTROLLER_BINDTYPE_AXIS, SDL_CONTROLLER_AXIS_RIGHTY, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_SCROLL_MAP_UP, {{SDL_CONTROLLER_BINDTYPE_AXIS, SDL_CONTROLLER_AXIS_RIGHTY, JOYSTICK_AXIS_NEGATIVE}}},
    {MAPPING_ACTION_SCROLL_MAP_LEFT, {{SDL_CONTROLLER_BINDTYPE_AXIS, SDL_CONTROLLER_AXIS_RIGHTX, JOYSTICK_AXIS_NEGATIVE}}},
    {MAPPING_ACTION_SCROLL_MAP_DOWN, {{SDL_CONTROLLER_BINDTYPE_AXIS, SDL_CONTROLLER_AXIS_RIGHTY, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_SCROLL_MAP_RIGHT, {{SDL_CONTROLLER_BINDTYPE_AXIS, SDL_CONTROLLER_AXIS_RIGHTX, JOYSTICK_AXIS_POSITIVE}}},
    {MAPPING_ACTION_ROTATE_MAP_LEFT, {{SDL_CONTROLLER_BINDTYPE_BUTTON, SDL_CONTROLLER_BUTTON_LEFTSHOULDER}}},
    {MAPPING_ACTION_ROTATE_MAP_RIGHT, {{SDL_CONTROLLER_BINDTYPE_BUTTON, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER}}},
    {MAPPING_ACTION_INCREASE_GAME_SPEED, {{SDL_CONTROLLER_BINDTYPE_BUTTON, SDL_CONTROLLER_BUTTON_Y}}},
    {MAPPING_ACTION_DECREASE_GAME_SPEED, {{SDL_CONTROLLER_BINDTYPE_BUTTON, SDL_CONTROLLER_BUTTON_X}}},
    {MAPPING_ACTION_MOUSE_CURSOR_UP, {{SDL_CONTROLLER_BINDTYPE_BUTTON, SDL_CONTROLLER_BUTTON_DPAD_UP}}},
    {MAPPING_ACTION_MOUSE_CURSOR_LEFT, {{SDL_CONTROLLER_BINDTYPE_BUTTON, SDL_CONTROLLER_BUTTON_DPAD_LEFT}}},
    {MAPPING_ACTION_MOUSE_CURSOR_DOWN, {{SDL_CONTROLLER_BINDTYPE_BUTTON, SDL_CONTROLLER_BUTTON_DPAD_DOWN}}},
    {MAPPING_ACTION_MOUSE_CURSOR_RIGHT, {{SDL_CONTROLLER_BINDTYPE_BUTTON, SDL_CONTROLLER_BUTTON_DPAD_RIGHT}}}
};

#define NUM_CONTROLLER_MAPPINGS (sizeof(controller_mappings) / sizeof(controller_mapping))

static int use_joystick(void)
{
#if defined(__vita__) || defined(__SWITCH__)
    return 1;
#else
    return enabled;
#endif
}

static joystick_hat_position convert_hat_position(int value)
{
    joystick_hat_position position = JOYSTICK_HAT_CENTERED;
    if (value & SDL_HAT_UP) {
        position |= JOYSTICK_HAT_UP;
    }
    if (value & SDL_HAT_DOWN) {
        position |= JOYSTICK_HAT_DOWN;
    }
    if (value & SDL_HAT_LEFT) {
        position |= JOYSTICK_HAT_LEFT;
    }
    if (value & SDL_HAT_RIGHT) {
        position |= JOYSTICK_HAT_RIGHT;
    }
    return position;
}

static void set_default_controller_mapping(joystick_model *model, SDL_GameController *controller)
{
    for (int i = 0; i < NUM_CONTROLLER_MAPPINGS; i++) {
        const controller_mapping *mapping = &controller_mappings[i];
        SDL_GameControllerButtonBind binds[JOYSTICK_MAPPING_ELEMENTS_MAX] = { 0 };
        int bind_failed = 0;
        for (int j = 0; j < JOYSTICK_MAPPING_ELEMENTS_MAX; j++) {
            int has_bind = 0;
            if (mapping->element[j].type == SDL_CONTROLLER_BINDTYPE_AXIS) {
                has_bind = 1;
                binds[j] = SDL_GameControllerGetBindForAxis(controller, mapping->element[j].id);
            } else if (mapping->element[j].type == SDL_CONTROLLER_BINDTYPE_BUTTON) {
                has_bind = 1;
                binds[j] = SDL_GameControllerGetBindForButton(controller, mapping->element[j].id);
            }
            if (has_bind && binds[j].bindType == SDL_CONTROLLER_BINDTYPE_NONE) {
                bind_failed = 1;
                break;
            }
        }
        if (bind_failed) {
            continue;
        }
        mapping_element *action = &model->mapping[model->num_mappings];
        action->action = mapping->action;
        for (int j = 0; j < JOYSTICK_MAPPING_ELEMENTS_MAX; j++) {
            switch (binds[j].bindType) {
                case SDL_CONTROLLER_BINDTYPE_AXIS:
                    action->element[j].type = JOYSTICK_ELEMENT_AXIS;
                    action->element[j].id = binds[j].value.axis;
                    action->element[j].position = mapping->element[j].position;
                    break;
                case SDL_CONTROLLER_BINDTYPE_BUTTON:
                    action->element[j].type = JOYSTICK_ELEMENT_BUTTON;
                    action->element[j].id = binds[j].value.button;
                    break;
                case SDL_CONTROLLER_BINDTYPE_HAT:
                    action->element[j].type = JOYSTICK_ELEMENT_HAT;
                    action->element[j].id = binds[j].value.hat.hat;
                    action->element[j].position = convert_hat_position(binds[j].value.hat.hat_mask);
                    break;
                default:
                    break;
            }
        }
        model->num_mappings++;
    }
}

static void create_new_model(const char *guid, const char *name, int instance_id, SDL_GameController *controller)
{
    joystick_model model;
    memset(&model, 0, sizeof(model));
    snprintf(model.guid, JOYSTICK_MAX_GUID, "%s", guid);
    if (!name) {
        snprintf(model.name, JOYSTICK_MAX_NAME, "Joystick %d", instance_id);
    } else {
        snprintf(model.name, JOYSTICK_MAX_NAME, "Joystick %s", name);
    }
    if (controller) {
        set_default_controller_mapping(&model, controller);
    } else {
        size_t size = sizeof(default_mapping);
        if (size > sizeof(model.mapping)) {
            size = sizeof(model.mapping);
        }
        memcpy(model.mapping, default_mapping, size);
        model.num_mappings = size / sizeof(mapping_element);
    }
    joystick_add_model(&model);
}

static void add_joystick(int index)
{
    SDL_GameController *controller = 0;
    SDL_Joystick *joystick = 0;

    if (SDL_IsGameController(index)) {
        controller = SDL_GameControllerOpen(index);
        joystick = SDL_GameControllerGetJoystick(controller);
        SDL_Log("Game controller found. Setting default gamepad mapping.");
    } else {
        joystick = SDL_JoystickOpen(index);    
    }
    int instance_id = SDL_JoystickInstanceID(joystick);
    if (joystick_is_active(instance_id)) {
        if (controller) {
            SDL_GameControllerClose(controller);
        } else {
            SDL_JoystickClose(joystick);
        }
        return;
    }

    static char guid[JOYSTICK_MAX_GUID];
    SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(joystick), guid, JOYSTICK_MAX_GUID);
    if (!joystick_has_model(guid)) {
        create_new_model(guid, SDL_JoystickName(joystick), instance_id, controller);
    }
    if (!joystick_add(instance_id, guid)) {
        if (controller) {
            SDL_GameControllerClose(controller);
        } else {
            SDL_JoystickClose(joystick);
        }
    }
}

static void remove_joystick(int instance_id)
{
    if (!joystick_is_active(instance_id)) {
        return;
    }
    SDL_GameController *controller = 0;
    SDL_Joystick *joystick = 0;
#if SDL_VERSION_ATLEAST(2, 0, 4)
    if (platform_sdl_version_at_least(2, 0, 4)) {
        controller = SDL_GameControllerFromInstanceID(instance_id);
        if (!controller) {
            joystick = SDL_JoystickFromInstanceID(instance_id);
        }
    } else {
#endif
        for (int i = 0; i < SDL_NumJoysticks(); ++i) {
            if (SDL_IsGameController(i)) {
                SDL_GameController *current_controller = SDL_GameControllerOpen(i);
                SDL_Joystick *current_joystick = SDL_GameControllerGetJoystick(current_controller);
                if (SDL_JoystickInstanceID(current_joystick) == instance_id) {
                    controller = current_controller;
                    SDL_GameControllerClose(current_controller);
                    break;
                }
                SDL_GameControllerClose(current_controller);
            } else {
                SDL_Joystick *current_joystick = SDL_JoystickOpen(i);
                if (SDL_JoystickInstanceID(current_joystick) == instance_id) {
                    joystick = current_joystick;
                    SDL_JoystickClose(current_joystick);
                    break;
                }
                SDL_JoystickClose(current_joystick);
            }
        }
#if SDL_VERSION_ATLEAST(2, 0, 4)
    }
#endif
    if (controller) {
        SDL_GameControllerClose(controller);
    } else if (joystick) {
        SDL_JoystickClose(joystick);
    }
    joystick_remove(instance_id);
}

void platform_joystick_init(int force_enable)
{
    enabled = force_enable;

    if (!use_joystick()) {
        return;
    }
    if (SDL_JoystickEventState(SDL_ENABLE) != SDL_ENABLE) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Joystick events could not be enabled: %s", SDL_GetError());
    }
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        add_joystick(i);
    }
}

void platform_joystick_device_changed(int id, int is_connected)
{
    if (!use_joystick()) {
        return;
    }
    if (is_connected) {
        add_joystick(id);
    } else {
        remove_joystick(id);
    }
}

void platform_joystick_handle_axis(SDL_JoyAxisEvent *event)
{
    if (!use_joystick()) {
        return;
    }
    joystick_update_element(event->which, JOYSTICK_ELEMENT_AXIS, event->axis, event->value, 0);
}

void platform_joystick_handle_trackball(SDL_JoyBallEvent *event)
{
    if (!use_joystick()) {
        return;
    }
    joystick_update_element(event->which, JOYSTICK_ELEMENT_TRACKBALL, event->ball, event->xrel, event->yrel);
}

void platform_joystick_handle_hat(SDL_JoyHatEvent *event)
{
    if (!use_joystick()) {
        return;
    }
    joystick_update_element(event->which, JOYSTICK_ELEMENT_HAT, event->hat, convert_hat_position(event->value), 0);
}

void platform_joystick_handle_button(SDL_JoyButtonEvent *event, int is_down)
{
    if (!use_joystick()) {
        return;
    }
    joystick_update_element(event->which, JOYSTICK_ELEMENT_BUTTON, event->button, is_down, 0);
}
