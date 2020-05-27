#ifndef INPUT_JOYSTICK_H
#define INPUT_JOYSTICK_H

#include "input/mouse.h"

#define JOYSTICK_MAX_NAME 64
#define JOYSTICK_MAX_GUID 33
#define JOYSTICK_MAX_AXIS 8
#define JOYSTICK_MAX_BUTTONS 20
#define JOYSTICK_MAX_TRACKBALLS 4
#define JOYSTICK_MAX_HATS 4
#define JOYSTICK_MAX_CONTROLLERS 6

#define JOYSTICK_MAPPING_ELEMENTS_MAX 2

typedef enum {
    JOYSTICK_ELEMENT_NONE = 0,
    JOYSTICK_ELEMENT_AXIS = 1,
    JOYSTICK_ELEMENT_TRACKBALL = 2,
    JOYSTICK_ELEMENT_BUTTON = 3,
    JOYSTICK_ELEMENT_HAT = 4
} joystick_element;

typedef enum {
    JOYSTICK_AXIS_POSITIVE = 0,
    JOYSTICK_AXIS_NEGATIVE = 1
} joystick_axis_position;

typedef enum {
    JOYSTICK_HAT_CENTERED = 0,
    JOYSTICK_HAT_UP = 1,
    JOYSTICK_HAT_LEFT = 2,
    JOYSTICK_HAT_DOWN = 4,
    JOYSTICK_HAT_RIGHT = 8
} joystick_hat_position;

typedef enum {
    JOYSTICK_TRACKBALL_X_POSITIVE = 0,
    JOYSTICK_TRACKBALL_X_NEGATIVE = 1,
    JOYSTICK_TRACKBALL_Y_POSITIVE = 2,
    JOYSTICK_TRACKBALL_Y_NEGATIVE = 3,
} joystick_trackball_position;

typedef enum {
    MAPPING_ACTION_MOUSE_CURSOR_UP,
    MAPPING_ACTION_MOUSE_CURSOR_LEFT,
    MAPPING_ACTION_MOUSE_CURSOR_DOWN,
    MAPPING_ACTION_MOUSE_CURSOR_RIGHT,
    MAPPING_ACTION_FASTER_MOUSE_CURSOR_SPEED,
    MAPPING_ACTION_SLOWER_MOUSE_CURSOR_SPEED,
    MAPPING_ACTION_LEFT_MOUSE_BUTTON,
    MAPPING_ACTION_RIGHT_MOUSE_BUTTON,
    MAPPING_ACTION_SCROLL_WINDOW_UP,
    MAPPING_ACTION_SCROLL_WINDOW_DOWN,
    MAPPING_ACTION_SCROLL_MAP_UP,
    MAPPING_ACTION_SCROLL_MAP_LEFT,
    MAPPING_ACTION_SCROLL_MAP_DOWN,
    MAPPING_ACTION_SCROLL_MAP_RIGHT,
    MAPPING_ACTION_ROTATE_MAP_LEFT,
    MAPPING_ACTION_ROTATE_MAP_RIGHT,
    MAPPING_ACTION_INCREASE_GAME_SPEED,
    MAPPING_ACTION_DECREASE_GAME_SPEED,
    MAPPING_ACTION_TOGGLE_PAUSE,
    MAPPING_ACTION_CYCLE_LEGION,
    MAPPING_ACTION_RESET_MAPPING,
    MAPPING_ACTION_MAX
} mapping_action;

typedef struct {
    struct {
        joystick_element type;
        int id;
        int position;
    } element[JOYSTICK_MAPPING_ELEMENTS_MAX];
} mapping_element;

typedef struct {
    int connected_joysticks;
    char name[JOYSTICK_MAX_NAME];
    char guid[JOYSTICK_MAX_GUID];
    int total_axis;
    int total_buttons;
    int total_trackballs;
    int total_hats;
    mapping_element first_mapping[MAPPING_ACTION_MAX];
    mapping_element second_mapping[MAPPING_ACTION_MAX];
} joystick_model;

joystick_model *joystick_get_model_by_guid(const char *guid);
joystick_model *joystick_get_first_unused_model(void);

int joystick_add(int joystick_id, joystick_model *model);
int joystick_is_listened(int joystick_id);
int joystick_remove(int joystick_id);
void joystick_update_element(int joystick_id, joystick_element element, int element_id, int value1, int value2);

int joystick_to_mouse_and_keyboard(void);

#endif // INPUT_JOYSTICK_H
