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

#define JOYSTICK_MAPPING_ELEMENTS_MAX 2

typedef enum {
    JOYSTICK_ELEMENT_NONE = 0,
    JOYSTICK_ELEMENT_AXIS = 1,
    JOYSTICK_ELEMENT_BUTTON = 2,
    JOYSTICK_ELEMENT_TRACKBALL = 3,
    JOYSTICK_ELEMENT_HAT = 4
} joystick_element;

typedef enum {
    JOYSTICK_HAT_CENTERED = 0,
    JOYSTICK_HAT_UP = 1,
    JOYSTICK_HAT_LEFT = 2,
    JOYSTICK_HAT_DOWN = 4,
    JOYSTICK_HAT_RIGHT = 8
} joystick_hat_position;

typedef enum {
    JOYSTICK_TRACKBALL_X = 0,
    JOYSTICK_TRACKBALL_Y = 1
} joystick_trackball_position;

typedef enum {
    MAPPING_ACTION_CURSOR_UP,
    MAPPING_ACTION_CURSOR_LEFT,
    MAPPING_ACTION_CURSOR_DOWN,
    MAPPING_ACTION_CURSOR_RIGHT,
    MAPPING_ACTION_LEFT_MOUSE_BUTTON,
    MAPPING_ACTION_MIDDLE_MOUSE_BUTTON,
    MAPPING_ACTION_RIGHT_MOUSE_BUTTON,
    MAPPING_ACTION_SCROLL_UP,
    MAPPING_ACTION_SCROLL_LEFT,
    MAPPING_ACTION_SCROLL_DOWN,
    MAPPING_ACTION_SCROLL_RIGHT,
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
