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
#define JOYSTICK_NO_SLOT -1

typedef enum {
    JOYSTICK_ELEMENT_NONE = 0,
    JOYSTICK_ELEMENT_AXIS = 1,
    JOYSTICK_ELEMENT_BUTTON = 2,
    JOYSTICK_ELEMENT_TRACKBALL = 3,
    JOYSTICK_ELEMENT_HAT = 4
} joystick_element;

typedef enum {
    JOYSTICK_HAT_CENTERED = 0,
    JOYSTICK_HAT_TOP = 1,
    JOYSTICK_HAT_LEFT = 2,
    JOYSTICK_HAT_BOTTOM = 4,
    JOYSTICK_HAT_RIGHT = 8
} joystick_hat_position;

typedef enum {
    MAPPING_ACTION_CURSOR_UP = 0,
    MAPPING_ACTION_CURSOR_LEFT = 1,
    MAPPING_ACTION_CURSOR_DOWN = 2,
    MAPPING_ACTION_CURSOR_RIGHT = 3,
    MAPPING_ACTION_LEFT_MOUSE_BUTTON = 4,
    MAPPING_ACTION_MIDDLE_MOUSE_BUTTON = 5,
    MAPPING_ACTION_RIGHT_MOUSE_BUTTON = 6,
    MAPPING_ACTION_SCROLL_UP = 7,
    MAPPING_ACTION_SCROLL_LEFT = 8,
    MAPPING_ACTION_SCROLL_DOWN = 9,
    MAPPING_ACTION_SCROLL_RIGHT = 10,
    MAPPING_ACTION_MAX = 11
} mapping_action;

typedef struct {
    joystick_element type;
    int id;
} mapping_element;

typedef struct {
    int connected_joysticks;
    char name[JOYSTICK_MAX_NAME];
    char guid[JOYSTICK_MAX_GUID];
    int total_axis;
    int total_buttons;
    int total_balls;
    int total_hats;
    mapping_element mapping[MAPPING_ACTION_MAX];
} joystick_model;

joystick_model *joystick_get_model_by_guid(const char *guid);
joystick_model *joystick_get_model_by_index(int index);
joystick_model *joystick_get_first_unused_model(void);

int joystick_add(int joystick_id, const joystick_model *model);
int joystick_remove(int joystick_id);
void joystick_update_element(int joystick_id, joystick_element element, int element_id, int value1, int value2);

void joystick_to_mouse_and_keyboard(void);

#endif // INPUT_JOYSTICK_H
