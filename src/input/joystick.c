#include "joystick.h"

#include "city/warning.h"
#include "core/log.h"
#include "core/speed.h"
#include "core/time.h"
#include "game/settings.h"
#include "game/system.h"
#include "graphics/window.h"
#include "input/hotkey.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "input/scroll.h"
#include "window/empire.h"
#include "window/file_dialog.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOTKEY_OFFSET MAPPING_ACTION_ROTATE_MAP_LEFT
#define MAX_HOTKEYS 6
#define MAX_AXIS 8
#define MAX_BUTTONS 20
#define MAX_TRACKBALLS 4
#define MAX_HATS 4
#define MAX_CONTROLLERS 6

#define DEADZONE 2000.0f
#define AXIS_MAX_THRESHOLD 1300
#define AXIS_MAX_VALUE 32767

// Note: this ratio is untested and may be under or oversensitive
#define TRACKBALL_TO_AXIS_RATIO 5

// Axis elements (e.g. triggers) bound to button-like actions must be pressed at least this far
#define DIGITAL_AXIS_THRESHOLD 12000

#ifdef __ANDROID__
#define USE_ANALOG_CURSOR
#endif

/*
 * Analog cursor parameters (used when USE_ANALOG_CURSOR is defined)
 *
 * JOYSTICK_CURSOR_THRESHOLD:
 *     stick deflection (0.0 - 1.0) below which the cursor does not move at all
 * JOYSTICK_CURSOR_CURVE:
 *     response curve exponent; values above 1 give finer control for small stick deflections
 * JOYSTICK_CURSOR_MIN_SPEED / JOYSTICK_CURSOR_MAX_SPEED:
 *     cursor speed in pixels per second just past the threshold / at full deflection
 * JOYSTICK_CURSOR_START_SPEED_FACTOR:
 *     fraction of the speed applied when the stick is first moved (start of acceleration)
 * JOYSTICK_CURSOR_ACCELERATION_TIME:
 *     time in milliseconds for the cursor to ramp up from the start speed to full speed
 */
#define JOYSTICK_CURSOR_THRESHOLD 0.15f
#define JOYSTICK_CURSOR_CURVE 2.0f
#define JOYSTICK_CURSOR_MIN_SPEED 30.0f
#define JOYSTICK_CURSOR_MAX_SPEED 700.0f
#define JOYSTICK_CURSOR_START_SPEED_FACTOR 0.3f
#define JOYSTICK_CURSOR_ACCELERATION_TIME 700

#ifdef __ANDROID__
/*
 * Map scrolling emulates the mouse cursor pushed against one of the 4 screen edges
 *
 * JOYSTICK_MAP_SCROLL_THRESHOLD:
 *     stick deflection (0.0 - 1.0) needed to start scrolling
 * JOYSTICK_MAP_SCROLL_RELEASE_THRESHOLD:
 *     deflection below which scrolling stops again (lower than the start threshold to avoid flicker)
 * JOYSTICK_MAP_SCROLL_HYSTERESIS:
 *     extra angle (degrees) past the 45 degree border before the scroll direction changes
 */
#define USE_EDGE_SCROLL_EMULATION
#define JOYSTICK_MAP_SCROLL_THRESHOLD 0.5f
#define JOYSTICK_MAP_SCROLL_RELEASE_THRESHOLD 0.4f
#define JOYSTICK_MAP_SCROLL_HYSTERESIS 15.0f
#endif

#define NUM_QUICK_KEYS 12

typedef int joystick_axis;
typedef int joystick_button;

enum {
    JOYSTICK_TRACKBALL_X_POSITIVE = 0,
    JOYSTICK_TRACKBALL_X_NEGATIVE = 1,
    JOYSTICK_TRACKBALL_Y_POSITIVE = 2,
    JOYSTICK_TRACKBALL_Y_NEGATIVE = 3,
};

enum {
    DIRECTION_UP = 0,
    DIRECTION_LEFT = 1,
    DIRECTION_DOWN = 2,
    DIRECTION_RIGHT = 3,
    NUM_DIRECTIONS = 4
};

typedef enum {
    INPUT_STATE_IS_UP,
    INPUT_STATE_WENT_DOWN,
    INPUT_STATE_IS_DOWN,
    INPUT_STATE_WENT_UP
} input_state;

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

typedef struct {
    int id;
    int connected;
    joystick_model *model;
    joystick_axis axis[MAX_AXIS];
    joystick_button button[MAX_BUTTONS];
    joystick_trackball trackball[MAX_TRACKBALLS];
    joystick_hat hat[MAX_HATS];
} joystick_info;

typedef struct {
    joystick_element element;
    int value;
    input_state state;
} mapped_input;

enum {
    CURSOR_SLOWDOWN_FASTER = 1024,
    CURSOR_SLOWDOWN_NORMAL = 4096,
    CURSOR_SLOWDOWN_SLOWER = 8192
};

static struct {
    joystick_model connected_models[MAX_CONTROLLERS];
    joystick_info joystick[MAX_CONTROLLERS];
    int connected_joysticks;
    struct {
        speed_type x_speed;
        speed_type y_speed;
        int left_button;
        int middle_button;
        int right_button;
        scroll_state scroll;
        time_millis last_scroll_time;
    } mouse;
    mapped_input map_scroll[NUM_DIRECTIONS];
    int map_scroll_edge;
    mapped_input joystick_hotkey[MAX_HOTKEYS];
    mapped_input virtual_keyboard;
    mapped_input touch_mode;
    mapped_input emulated_keys[MAPPING_ACTION_MAX];
    mapped_input slow_speed_toggle;
    mapped_input show_empire;
    mapped_input save_file;
    struct {
        int selected;
        mapped_input previous;
        mapped_input next;
    } quick_key;
    struct {
        int active;
        time_millis last_time;
        time_millis move_start_time;
        float remainder_x;
        float remainder_y;
    } cursor;
} data;

typedef struct {
    mapping_action action;
    key_type key;
    key_modifier_type modifiers;
} emulated_key;

static const emulated_key EMULATED_KEYS[] = {
    {MAPPING_ACTION_SHOW_OVERLAY_PROBLEMS, KEY_TYPE_T, KEY_MOD_NONE},
    {MAPPING_ACTION_SHOW_OVERLAY_FIRE, KEY_TYPE_F, KEY_MOD_NONE},
    {MAPPING_ACTION_SHOW_OVERLAY_DAMAGE, KEY_TYPE_D, KEY_MOD_NONE},
    {MAPPING_ACTION_SHOW_OVERLAY_CRIME, KEY_TYPE_C, KEY_MOD_NONE},
    {MAPPING_ACTION_SHOW_OVERLAY_WATER, KEY_TYPE_W, KEY_MOD_NONE},
};

#define NUM_EMULATED_KEYS (sizeof(EMULATED_KEYS) / sizeof(emulated_key))

static const struct {
    key_type key;
    const char *name;
} QUICK_KEYS[NUM_QUICK_KEYS] = {
    {KEY_TYPE_1, "1"}, {KEY_TYPE_2, "2"}, {KEY_TYPE_3, "3"}, {KEY_TYPE_4, "4"},
    {KEY_TYPE_5, "5"}, {KEY_TYPE_6, "6"}, {KEY_TYPE_7, "7"}, {KEY_TYPE_8, "8"},
    {KEY_TYPE_9, "9"}, {KEY_TYPE_0, "0"}, {KEY_TYPE_MINUS, "-"}, {KEY_TYPE_EQUALS, "="}
};

static hotkey_action JOYSTICK_MAPPING_TO_HOTKEY_ACTION[MAX_HOTKEYS] = {
    HOTKEY_ROTATE_MAP_LEFT,
    HOTKEY_ROTATE_MAP_RIGHT,
    HOTKEY_INCREASE_GAME_SPEED,
    HOTKEY_DECREASE_GAME_SPEED,
    HOTKEY_TOGGLE_PAUSE,
    HOTKEY_CYCLE_LEGION
};

static void update_hat(joystick_hat *hat, joystick_hat_position position)
{
    hat->top = (position & JOYSTICK_HAT_UP) ? 1 : 0;
    hat->left = (position & JOYSTICK_HAT_LEFT) ? 1 : 0;
    hat->bottom = (position & JOYSTICK_HAT_DOWN) ? 1 : 0;
    hat->right = (position & JOYSTICK_HAT_RIGHT) ? 1 : 0;
}

static void update_trackball(joystick_trackball *trackball, int delta_x, int delta_y)
{
    trackball->delta_x += delta_x;
    trackball->delta_y += delta_y;
}

static void reset_joystick_state(joystick_info *joystick)
{
    memset(joystick->axis, 0, sizeof(int) * MAX_AXIS);
    memset(joystick->button, 0, sizeof(joystick_button) * MAX_BUTTONS);
    memset(joystick->trackball, 0, sizeof(joystick_trackball) * MAX_TRACKBALLS);
    memset(joystick->hat, 0, sizeof(joystick_hat) * MAX_HATS);
}

static joystick_info *get_free_joystick(void)
{
    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        if (!data.joystick[i].connected) {
            return &data.joystick[i];
        }
    }
    return 0;
}

static joystick_model *get_model_by_guid(const char *guid)
{
    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        joystick_model *model = &data.connected_models[i];
        if (strcmp(guid, model->guid) == 0) {
            return model;
        }
    }
    return 0;
}

int joystick_has_model(const char *guid)
{
    return get_model_by_guid(guid) != 0;
}

void joystick_add_model(const joystick_model *model)
{
    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        if (!data.connected_models[i].connected_joysticks) {
            memcpy(&data.connected_models[i], model, sizeof(joystick_model));
            return;
        }
    }
}

int joystick_add(int joystick_id, const char *guid)
{
    joystick_info *joystick = get_free_joystick();
    joystick_model *model = get_model_by_guid(guid);
    if (!joystick || !model) {
        return 0;
    }
    joystick->id = joystick_id;
    joystick->model = model;
    joystick->connected = 1;
    model->connected_joysticks++;
    data.connected_joysticks++;
    log_info("Joystick added with name", model->name, 0);
    return 1;
}

static joystick_info *get_joystick_by_id(int joystick_id)
{
    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        if (data.joystick[i].id == joystick_id && data.joystick[i].connected) {
            return &data.joystick[i];
        }
    }
    return 0;
}

int joystick_is_active(int joystick_id)
{
    return get_joystick_by_id(joystick_id) != 0;
}

int joystick_remove(int joystick_id)
{
    joystick_info *joystick = get_joystick_by_id(joystick_id);
    if (!joystick) {
        return 0;
    }
    joystick->connected = 0;
    joystick->model->connected_joysticks--;
    const char *name = joystick->model->name;
    joystick->model = 0;
    reset_joystick_state(joystick);
    data.connected_joysticks--;
    if (data.map_scroll_edge) {
        data.map_scroll_edge = 0;
        scroll_set_virtual_edge(0, 0, 0, 0);
    }
    log_info("Joystick removed with name", name, 0);
    return 1;
}

void joystick_update_element(int joystick_id, joystick_element element, int element_id, int value1, int value2)
{
    joystick_info *joystick = get_joystick_by_id(joystick_id);
    if (!joystick) {
        return;
    }
    switch (element) {
        case JOYSTICK_ELEMENT_BUTTON:
            joystick->button[element_id] = value1;
            break;
        case JOYSTICK_ELEMENT_HAT:
            update_hat(&joystick->hat[element_id], value1);
            break;
        case JOYSTICK_ELEMENT_AXIS:
            joystick->axis[element_id] = (abs(value1) > AXIS_MAX_THRESHOLD) ? value1 : 0;
            break;
        case JOYSTICK_ELEMENT_TRACKBALL:
            update_trackball(&joystick->trackball[element_id], value1, value2);
            break;
        default:
            log_info("Trying to update wrong joystick element", 0, element);
            break;
    }
}

static int get_input_for_mapping(const joystick_info *joystick, const mapping_element *mapping, mapped_input *input)
{
    for (int j = 0; j < JOYSTICK_MAPPING_ELEMENTS_MAX; ++j) {
        joystick_element current_element = mapping->element[j].type;
        int current_value = 0;
        int element_id = mapping->element[j].id;
        int element_position = mapping->element[j].position;
        switch (current_element) {
            case JOYSTICK_ELEMENT_AXIS:
                if (element_position == JOYSTICK_AXIS_POSITIVE) {
                    current_value = (joystick->axis[element_id] > 0) ? joystick->axis[element_id] : 0;
                } else {
                    current_value = (joystick->axis[element_id] < 0) ? -joystick->axis[element_id] : 0;
                }
                break;
            case JOYSTICK_ELEMENT_TRACKBALL:
                switch (element_position) {
                    case JOYSTICK_TRACKBALL_X_POSITIVE:
                        if (joystick->trackball[element_id].delta_x > 0) {
                            current_value = joystick->trackball[element_id].delta_x;
                        }
                        break;
                    case JOYSTICK_TRACKBALL_X_NEGATIVE:
                        if (joystick->trackball[element_id].delta_x < 0) {
                            current_value = -joystick->trackball[element_id].delta_x;
                        }
                        break;
                    case JOYSTICK_TRACKBALL_Y_POSITIVE:
                        if (joystick->trackball[element_id].delta_y > 0) {
                            current_value = joystick->trackball[element_id].delta_y;
                        }
                        break;
                    case JOYSTICK_TRACKBALL_Y_NEGATIVE:
                        if (joystick->trackball[element_id].delta_y < 0) {
                            current_value = -joystick->trackball[element_id].delta_y;
                        }
                        break;
                }
                break;
            case JOYSTICK_ELEMENT_BUTTON:
                current_value = joystick->button[element_id];
                break;
            case JOYSTICK_ELEMENT_HAT:
            {
                const joystick_hat *hat = &joystick->hat[element_id];
                switch (element_position) {
                    case JOYSTICK_HAT_UP:
                        current_value = hat->top;
                        break;
                    case JOYSTICK_HAT_LEFT:
                        current_value = hat->left;
                        break;
                    case JOYSTICK_HAT_DOWN:
                        current_value = hat->bottom;
                        break;
                    case JOYSTICK_HAT_RIGHT:
                        current_value = hat->right;
                        break;
                    default:
                        current_value = 0;
                        log_info("Invalid hat value for hat", 0, element_id);
                        break;
                }
                break;
            }
            default:
                continue;
        }
        if (current_value == 0) {
            input->element = JOYSTICK_ELEMENT_NONE;
            input->value = 0;
            return 0;
        }
        // Element precedence. When two joystick elements are required for the input
        // to be registered, the value is obtained from the highest priority element.
        // Element priority in descending order: axis, trackball, button, hat
        if (input->element == JOYSTICK_ELEMENT_NONE || current_element < input->element) {
            input->element = current_element;
            input->value = current_value;
        }
    }
    return input->value != 0;
}

static void set_input_state(mapped_input *input)
{
    if (input->value) {
        if (input->state == INPUT_STATE_IS_UP || input->state == INPUT_STATE_WENT_UP) {
            input->state = INPUT_STATE_WENT_DOWN;
        } else {
            input->state = INPUT_STATE_IS_DOWN;
        }
    } else {
        if (input->state == INPUT_STATE_IS_DOWN || input->state == INPUT_STATE_WENT_DOWN) {
            input->state = INPUT_STATE_WENT_UP;
        } else {
            input->state = INPUT_STATE_IS_UP;
        }
    }
}

static int is_analog_action(mapping_action action)
{
    switch (action) {
        case MAPPING_ACTION_MOUSE_CURSOR_UP:
        case MAPPING_ACTION_MOUSE_CURSOR_LEFT:
        case MAPPING_ACTION_MOUSE_CURSOR_DOWN:
        case MAPPING_ACTION_MOUSE_CURSOR_RIGHT:
        case MAPPING_ACTION_SCROLL_WINDOW_UP:
        case MAPPING_ACTION_SCROLL_WINDOW_DOWN:
        case MAPPING_ACTION_SCROLL_MAP_UP:
        case MAPPING_ACTION_SCROLL_MAP_LEFT:
        case MAPPING_ACTION_SCROLL_MAP_DOWN:
        case MAPPING_ACTION_SCROLL_MAP_RIGHT:
            return 1;
        default:
            return 0;
    }
}

static int get_joystick_input_for_action(mapping_action action, mapped_input *input)
{
    static mapped_input dummy_input = {0};
    if (!input) {
        input = &dummy_input;
    }

    input->value = 0;
    input->element = JOYSTICK_ELEMENT_NONE;

    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        const joystick_info *joystick = &data.joystick[i];
        if (!joystick->connected) {
            continue;
        }
        const joystick_model *model = joystick->model;
        for (int j = 0; j < model->num_mappings; j++) {
            if (model->mapping[j].action != action || !get_input_for_mapping(joystick, &model->mapping[j], input)) {
                continue;
            }
            if (input->element == JOYSTICK_ELEMENT_AXIS && input->value < DIGITAL_AXIS_THRESHOLD &&
                !is_analog_action(action)) {
                input->value = 0;
                input->element = JOYSTICK_ELEMENT_NONE;
                continue;
            }
            set_input_state(input);
            return 1;
        }
    }
    set_input_state(input);
    return 0;
}

static void translate_input_for_element(mapped_input *input, joystick_element translated_element)
{
    if (input->element == translated_element) {
        return;
    }
    if (translated_element == JOYSTICK_ELEMENT_AXIS) {
        switch (input->element)
        {
            case JOYSTICK_ELEMENT_TRACKBALL:
                input->value *= TRACKBALL_TO_AXIS_RATIO;
                break;
            default:
                input->value *= AXIS_MAX_VALUE;
                break;
        }
    } else if (translated_element == JOYSTICK_ELEMENT_TRACKBALL) {
        switch (input->element) {
            case JOYSTICK_ELEMENT_AXIS:
                input->value /= TRACKBALL_TO_AXIS_RATIO;
                break;
            default:
                input->value *= TRACKBALL_TO_AXIS_RATIO;
                break;
        }
    } else if (translated_element == JOYSTICK_ELEMENT_BUTTON || translated_element == JOYSTICK_ELEMENT_HAT) {
        input->value = (input->value != 0) ? 1 : 0;
    }
    input->element = translated_element;
}

static int rescale_axis(mapped_input *inputs)
{
    // Radial and scaled dead_zone
    // http://www.third-helix.com/2013/04/12/doing-thumbstick-dead-zones-right.html
    // Input and output values go from -AXIS_MAX_VALUE to AXIS_MAX_VALUE

    // The maximum is adjusted to account for SCE_CTRL_MODE_DIGITALANALOG_WIDE
    // where a reported maximum axis value corresponds to 80% of the full range
    // of motion of the analog stick
    float analog_x = (float) inputs[DIRECTION_RIGHT].value - inputs[DIRECTION_LEFT].value;
    float analog_y = (float) inputs[DIRECTION_DOWN].value - inputs[DIRECTION_UP].value;

    inputs[DIRECTION_UP].value = 0;
    inputs[DIRECTION_LEFT].value = 0;
    inputs[DIRECTION_DOWN].value = 0;
    inputs[DIRECTION_RIGHT].value = 0;

    float magnitude = sqrtf(analog_x * analog_x + analog_y * analog_y);
    if (magnitude < DEADZONE) {
        return 0;
    }
    // Adjust maximum magnitude
    float abs_analog_x = fabsf(analog_x);
    float abs_analog_y = fabsf(analog_y);
    float max_x;
    float max_y;
    if (abs_analog_x > abs_analog_y) {
        max_x = AXIS_MAX_VALUE;
        max_y = (AXIS_MAX_VALUE * analog_y) / abs_analog_x;
    } else {
        max_x = (AXIS_MAX_VALUE * analog_x) / abs_analog_y;
        max_y = AXIS_MAX_VALUE;
    }
    float maximum = sqrtf(max_x * max_x + max_y * max_y);
    if (maximum > 1.25f * AXIS_MAX_VALUE) {
        maximum = 1.25f * AXIS_MAX_VALUE;
    }
    if (maximum < magnitude) {
        maximum = magnitude;
    }

    // Find scaled axis values with magnitudes between zero and maximum
    float scaling_factor = maximum / magnitude * (magnitude - DEADZONE) / (maximum - DEADZONE);
    analog_x = (analog_x * scaling_factor);
    analog_y = (analog_y * scaling_factor);

    // Clamp to ensure results will never exceed the max_axis value
    float clamping_factor = 1.0f;
    abs_analog_x = fabsf(analog_x);
    abs_analog_y = fabsf(analog_y);
    if (abs_analog_x > AXIS_MAX_VALUE || abs_analog_y > AXIS_MAX_VALUE) {
        if (abs_analog_x > abs_analog_y) {
            clamping_factor = AXIS_MAX_VALUE / abs_analog_x;
        } else {
            clamping_factor = AXIS_MAX_VALUE / abs_analog_y;
        }
    }
    if (analog_y > 0.0f) {
        inputs[DIRECTION_DOWN].value = (int) (clamping_factor * analog_y);
    } else if (analog_y < 0.0f) {
        inputs[DIRECTION_UP].value = (int) (clamping_factor * -analog_y);
    }
    if (analog_x > 0.0f) {
        inputs[DIRECTION_RIGHT].value = (int) (clamping_factor * analog_x);
    } else if (analog_x < 0.0f) {
        inputs[DIRECTION_LEFT].value = (int) (clamping_factor * -analog_x);
    }
    return 1;
}

static joystick_element get_highest_priority_element(mapped_input *inputs, int total_inputs)
{
    joystick_element highest_priority = inputs[0].element;
    for (int i = 1; i < total_inputs; ++i) {
        if (inputs[i].element < highest_priority && inputs[i].element != JOYSTICK_ELEMENT_NONE) {
            highest_priority = inputs[i].element;
        }
    }
    return highest_priority;
}

static int translate_mapping_reset(void)
{
    return get_joystick_input_for_action(MAPPING_ACTION_RESET_MAPPING, 0);
}

#ifdef USE_ANALOG_CURSOR
static void reset_analog_cursor(void)
{
    data.cursor.move_start_time = 0;
    data.cursor.remainder_x = 0.0f;
    data.cursor.remainder_y = 0.0f;
}

static int translate_mouse_cursor_position(void)
{
    mapped_input cursor_input[NUM_DIRECTIONS] = {0};

    time_millis now = time_get_millis();
    time_millis elapsed = data.cursor.last_time ? now - data.cursor.last_time : 0;
    data.cursor.last_time = now;
    if (elapsed > 100) {
        elapsed = 100;
    }

    int handled = get_joystick_input_for_action(MAPPING_ACTION_MOUSE_CURSOR_UP, &cursor_input[DIRECTION_UP]);
    handled |= get_joystick_input_for_action(MAPPING_ACTION_MOUSE_CURSOR_LEFT, &cursor_input[DIRECTION_LEFT]);
    handled |= get_joystick_input_for_action(MAPPING_ACTION_MOUSE_CURSOR_DOWN, &cursor_input[DIRECTION_DOWN]);
    handled |= get_joystick_input_for_action(MAPPING_ACTION_MOUSE_CURSOR_RIGHT, &cursor_input[DIRECTION_RIGHT]);
    if (!handled) {
        reset_analog_cursor();
        return 0;
    }
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        translate_input_for_element(&cursor_input[i], JOYSTICK_ELEMENT_AXIS);
    }

    float x = (float) (cursor_input[DIRECTION_RIGHT].value - cursor_input[DIRECTION_LEFT].value) / AXIS_MAX_VALUE;
    float y = (float) (cursor_input[DIRECTION_DOWN].value - cursor_input[DIRECTION_UP].value) / AXIS_MAX_VALUE;
    float magnitude = sqrtf(x * x + y * y);
    if (magnitude <= JOYSTICK_CURSOR_THRESHOLD) {
        reset_analog_cursor();
        return 0;
    }
    // Unit direction vector of the stick
    float dir_x = x / magnitude;
    float dir_y = y / magnitude;
    if (magnitude > 1.0f) {
        magnitude = 1.0f;
    }
    if (!data.cursor.move_start_time) {
        data.cursor.move_start_time = now;
    }

    // Deflection past the threshold, rescaled to 0..1 and shaped by the response curve
    float deflection = (magnitude - JOYSTICK_CURSOR_THRESHOLD) / (1.0f - JOYSTICK_CURSOR_THRESHOLD);
    float speed = JOYSTICK_CURSOR_MIN_SPEED +
        (JOYSTICK_CURSOR_MAX_SPEED - JOYSTICK_CURSOR_MIN_SPEED) * powf(deflection, JOYSTICK_CURSOR_CURVE);

    // Acceleration: the cursor starts slow and speeds up while the stick is held
    float acceleration = 1.0f;
    time_millis held = now - data.cursor.move_start_time;
    if (held < JOYSTICK_CURSOR_ACCELERATION_TIME) {
        acceleration = JOYSTICK_CURSOR_START_SPEED_FACTOR +
            (1.0f - JOYSTICK_CURSOR_START_SPEED_FACTOR) * held / JOYSTICK_CURSOR_ACCELERATION_TIME;
    }

    float distance = speed * acceleration * elapsed / 1000.0f;
    float move_x = data.cursor.remainder_x + dir_x * distance;
    float move_y = data.cursor.remainder_y + dir_y * distance;
    int delta_x = (int) move_x;
    int delta_y = (int) move_y;
    data.cursor.remainder_x = move_x - delta_x;
    data.cursor.remainder_y = move_y - delta_y;

    if (delta_x || delta_y) {
        system_move_mouse_cursor(delta_x, delta_y);
    }
    return 1;
}
#else
static int translate_mouse_cursor_position(void)
{
    mapped_input cursor_input[NUM_DIRECTIONS] = {0};

    int handled = get_joystick_input_for_action(MAPPING_ACTION_MOUSE_CURSOR_UP, &cursor_input[DIRECTION_UP]);
    handled |= get_joystick_input_for_action(MAPPING_ACTION_MOUSE_CURSOR_LEFT, &cursor_input[DIRECTION_LEFT]);
    handled |= get_joystick_input_for_action(MAPPING_ACTION_MOUSE_CURSOR_DOWN, &cursor_input[DIRECTION_DOWN]);
    handled |= get_joystick_input_for_action(MAPPING_ACTION_MOUSE_CURSOR_RIGHT, &cursor_input[DIRECTION_RIGHT]);
    if (!handled) {
        speed_clear(&data.mouse.x_speed);
        speed_clear(&data.mouse.y_speed);
        return 0;
    }

    // All mouse cursor input will internally be treated as a joystick axis
    translate_input_for_element(&cursor_input[DIRECTION_UP], JOYSTICK_ELEMENT_AXIS);
    translate_input_for_element(&cursor_input[DIRECTION_LEFT], JOYSTICK_ELEMENT_AXIS);
    translate_input_for_element(&cursor_input[DIRECTION_DOWN], JOYSTICK_ELEMENT_AXIS);
    translate_input_for_element(&cursor_input[DIRECTION_RIGHT], JOYSTICK_ELEMENT_AXIS);

    if (!rescale_axis(cursor_input)) {
        speed_clear(&data.mouse.x_speed);
        speed_clear(&data.mouse.y_speed);
        return 0;
    }
    
    double slowdown = CURSOR_SLOWDOWN_NORMAL;
    if (get_joystick_input_for_action(MAPPING_ACTION_FASTER_MOUSE_CURSOR_SPEED, 0)) {
        slowdown = CURSOR_SLOWDOWN_FASTER;
    } else if (get_joystick_input_for_action(MAPPING_ACTION_SLOWER_MOUSE_CURSOR_SPEED, 0)) {
        slowdown = CURSOR_SLOWDOWN_SLOWER;
    }
    int delta_x = cursor_input[DIRECTION_RIGHT].value - cursor_input[DIRECTION_LEFT].value;
    int delta_y = cursor_input[DIRECTION_DOWN].value - cursor_input[DIRECTION_UP].value;

    speed_set_target(&data.mouse.x_speed, delta_x / slowdown, SPEED_CHANGE_IMMEDIATE, 1);
    speed_set_target(&data.mouse.y_speed, delta_y / slowdown, SPEED_CHANGE_IMMEDIATE, 1);

    delta_x = speed_get_delta(&data.mouse.x_speed);
    delta_y = speed_get_delta(&data.mouse.y_speed);

    if (!delta_x && !delta_y) {
        return 1;
    }

    system_move_mouse_cursor(delta_x, delta_y);
    return 1;
}
#endif

static int translate_mouse_button_presses(void)
{
    int handled = 0;
    int button = get_joystick_input_for_action(MAPPING_ACTION_LEFT_MOUSE_BUTTON, 0);
    if (button != data.mouse.left_button) {
        data.mouse.left_button = button;
        mouse_set_left_down(button);
        handled = 1;
    }
    button = get_joystick_input_for_action(MAPPING_ACTION_RIGHT_MOUSE_BUTTON, 0);
    if (button != data.mouse.right_button) {
        data.mouse.right_button = button;
        mouse_set_right_down(button);
        handled = 1;
    }
    return handled;
}

static int translate_mouse(void)
{
    int handled = 0;
    handled |= translate_mouse_cursor_position();
    handled |= translate_mouse_button_presses();
    return handled;
}

static int window_has_map_scrolling(void)
{
    return window_is(WINDOW_CITY) ||
        window_is(WINDOW_CITY_MILITARY) ||
        window_is(WINDOW_EDITOR_MAP) ||
        window_is(WINDOW_EMPIRE) ||
        window_is(WINDOW_EDITOR_EMPIRE);
}

static int translate_window_scrolling(void)
{
    if (window_has_map_scrolling()) {
        return 0;
    }
    int handled = 0;
    scroll_state current_scroll = SCROLL_NONE;
    mapped_input scroll_up, scroll_down;

    handled |= get_joystick_input_for_action(MAPPING_ACTION_SCROLL_WINDOW_UP, &scroll_up);
    handled |= get_joystick_input_for_action(MAPPING_ACTION_SCROLL_WINDOW_DOWN, &scroll_down);

    if (!handled) {
        return 0;
    }

    // All mouse scrolling input will internally be treated as a joystick axis
    translate_input_for_element(&scroll_up, JOYSTICK_ELEMENT_AXIS);
    translate_input_for_element(&scroll_down, JOYSTICK_ELEMENT_AXIS);

    unsigned int max_scroll_time = 50;

    if (scroll_up.value) {
        current_scroll = SCROLL_UP;
        max_scroll_time = max_scroll_time * AXIS_MAX_VALUE / scroll_up.value;
    } else if (scroll_down.value) {
        current_scroll = SCROLL_DOWN;
        max_scroll_time = max_scroll_time * AXIS_MAX_VALUE / scroll_down.value;
    }

    if (current_scroll != SCROLL_NONE) {
        time_millis current_time = time_get_millis();
        if (current_time - data.mouse.last_scroll_time > max_scroll_time) {
            data.mouse.last_scroll_time = current_time;
            mouse_set_scroll(current_scroll);
        }
        return 1;
    } else {
        data.mouse.last_scroll_time = 0;
        return 0;
    }
}

#ifdef USE_EDGE_SCROLL_EMULATION
/*
 * The stick acts exactly like the mouse cursor pushed against the middle of one of the
 * four screen edges: only 4 directions, at the regular edge scrolling speed.
 * The current edge is kept until the stick clearly points elsewhere (hysteresis).
 */
static int get_edge_for_stick(float x, float y)
{
    float magnitude = sqrtf(x * x + y * y);
    int current = data.map_scroll_edge;
    float threshold = current ? JOYSTICK_MAP_SCROLL_RELEASE_THRESHOLD : JOYSTICK_MAP_SCROLL_THRESHOLD;
    if (magnitude < threshold) {
        return 0;
    }
    float angle = atan2f(y, x) * 180.0f / 3.14159265f; // 0 = right, 90 = down
    if (angle < 0.0f) {
        angle += 360.0f;
    }
    // Edges: 1 = right, 2 = bottom, 3 = left, 4 = top
    int nearest = ((int) ((angle + 45.0f) / 90.0f)) % 4 + 1;
    if (current && current != nearest) {
        float center = (current - 1) * 90.0f;
        float diff = fabsf(fmodf(angle - center + 540.0f, 360.0f) - 180.0f);
        if (diff <= 45.0f + JOYSTICK_MAP_SCROLL_HYSTERESIS) {
            return current;
        }
    }
    return nearest;
}

static int translate_map_scrolling(void)
{
    if (!window_has_map_scrolling()) {
        if (data.map_scroll_edge) {
            data.map_scroll_edge = 0;
            scroll_set_virtual_edge(0, 0, 0, 0);
        }
        return 0;
    }
    mapped_input inputs[NUM_DIRECTIONS] = {0};
    int handled = get_joystick_input_for_action(MAPPING_ACTION_SCROLL_MAP_UP, &inputs[DIRECTION_UP]);
    handled |= get_joystick_input_for_action(MAPPING_ACTION_SCROLL_MAP_LEFT, &inputs[DIRECTION_LEFT]);
    handled |= get_joystick_input_for_action(MAPPING_ACTION_SCROLL_MAP_DOWN, &inputs[DIRECTION_DOWN]);
    handled |= get_joystick_input_for_action(MAPPING_ACTION_SCROLL_MAP_RIGHT, &inputs[DIRECTION_RIGHT]);

    int edge = 0;
    if (handled) {
        for (int i = 0; i < NUM_DIRECTIONS; i++) {
            translate_input_for_element(&inputs[i], JOYSTICK_ELEMENT_AXIS);
        }
        float x = (float) (inputs[DIRECTION_RIGHT].value - inputs[DIRECTION_LEFT].value) / AXIS_MAX_VALUE;
        float y = (float) (inputs[DIRECTION_DOWN].value - inputs[DIRECTION_UP].value) / AXIS_MAX_VALUE;
        edge = get_edge_for_stick(x, y);
    }
    if (edge != data.map_scroll_edge) {
        data.map_scroll_edge = edge;
        scroll_set_virtual_edge(edge == 4, edge == 3, edge == 2, edge == 1);
    }
    return edge != 0;
}
#else
static int translate_map_scrolling(void)
{
    if (!window_has_map_scrolling()) {
        return 0;
    }

    int handled = get_joystick_input_for_action(MAPPING_ACTION_SCROLL_MAP_UP, &data.map_scroll[DIRECTION_UP]);
    handled |= get_joystick_input_for_action(MAPPING_ACTION_SCROLL_MAP_LEFT, &data.map_scroll[DIRECTION_LEFT]);
    handled |= get_joystick_input_for_action(MAPPING_ACTION_SCROLL_MAP_DOWN, &data.map_scroll[DIRECTION_DOWN]);
    handled |= get_joystick_input_for_action(MAPPING_ACTION_SCROLL_MAP_RIGHT, &data.map_scroll[DIRECTION_RIGHT]);

    if (!handled) {
        int stopped_scrolling = 0;
        for (int direction = 0; direction < NUM_DIRECTIONS; ++direction) {
            if (data.map_scroll[direction].state == INPUT_STATE_IS_DOWN ||
                data.map_scroll[direction].state == INPUT_STATE_WENT_DOWN) {
                stopped_scrolling = 0;
                break;
            } else if (data.map_scroll[direction].state == INPUT_STATE_WENT_UP) {
                stopped_scrolling |= 1;
            }
        }
        if (stopped_scrolling) {
            scroll_arrow_up(0);
            scroll_arrow_left(0);
            scroll_arrow_down(0);
            scroll_arrow_right(0);
        }
        return 0;
    }

    joystick_element base_element = get_highest_priority_element(data.map_scroll, 4);

    translate_input_for_element(&data.map_scroll[DIRECTION_UP], base_element);
    translate_input_for_element(&data.map_scroll[DIRECTION_LEFT], base_element);
    translate_input_for_element(&data.map_scroll[DIRECTION_DOWN], base_element);
    translate_input_for_element(&data.map_scroll[DIRECTION_RIGHT], base_element);

    if (base_element == JOYSTICK_ELEMENT_AXIS) {
        rescale_axis(data.map_scroll);
    }

    scroll_arrow_up(data.map_scroll[DIRECTION_UP].value);
    scroll_arrow_left(data.map_scroll[DIRECTION_LEFT].value);
    scroll_arrow_down(data.map_scroll[DIRECTION_DOWN].value);
    scroll_arrow_right(data.map_scroll[DIRECTION_RIGHT].value);

    return 1;
}
#endif

static int translate_hotkeys(void)
{
    int handled = 0;
    for (int i = 0; i < MAX_HOTKEYS; ++i) {
        int value = get_joystick_input_for_action(HOTKEY_OFFSET + i, &data.joystick_hotkey[i]);
        handled |= value;
        if (value) {
            if (data.joystick_hotkey[i].state == INPUT_STATE_WENT_DOWN) {
                hotkey_set_value_for_action(JOYSTICK_MAPPING_TO_HOTKEY_ACTION[i], 1);
            }
        } else if (data.joystick_hotkey[i].state == INPUT_STATE_WENT_UP) {
            hotkey_set_value_for_action(JOYSTICK_MAPPING_TO_HOTKEY_ACTION[i], 0);
        }
    }
    return handled;
}

static int translate_emulated_keys(void)
{
    int handled = 0;
    for (unsigned int i = 0; i < NUM_EMULATED_KEYS; i++) {
        const emulated_key *k = &EMULATED_KEYS[i];
        mapped_input *input = &data.emulated_keys[k->action];
        if (get_joystick_input_for_action(k->action, input)) {
            handled = 1;
            if (input->state == INPUT_STATE_WENT_DOWN) {
                hotkey_key_pressed(k->key, k->modifiers, 0);
            }
        } else if (input->state == INPUT_STATE_WENT_UP) {
            hotkey_key_released(k->key, k->modifiers);
        }
    }
    return handled;
}

static void show_city_message(const char *text)
{
    if (!window_is(WINDOW_CITY) && !window_is(WINDOW_CITY_MILITARY)) {
        return;
    }
    city_warning_clear_all();
    city_warning_show_custom((const uint8_t *) text);
}

static void select_quick_key(int offset)
{
    data.quick_key.selected = (data.quick_key.selected + offset + NUM_QUICK_KEYS) % NUM_QUICK_KEYS;
    // Selecting a key also presses it
    key_type key = QUICK_KEYS[data.quick_key.selected].key;
    hotkey_key_pressed(key, KEY_MOD_NONE, 0);
    hotkey_key_released(key, KEY_MOD_NONE);
    char text[32];
    snprintf(text, sizeof(text), "Key: %s", QUICK_KEYS[data.quick_key.selected].name);
    show_city_message(text);
}

static int translate_quick_keys(void)
{
    int handled = 0;
    if (get_joystick_input_for_action(MAPPING_ACTION_PREVIOUS_QUICK_KEY, &data.quick_key.previous)) {
        handled = 1;
        if (data.quick_key.previous.state == INPUT_STATE_WENT_DOWN) {
            select_quick_key(-1);
        }
    }
    if (get_joystick_input_for_action(MAPPING_ACTION_NEXT_QUICK_KEY, &data.quick_key.next)) {
        handled = 1;
        if (data.quick_key.next.state == INPUT_STATE_WENT_DOWN) {
            select_quick_key(1);
        }
    }
    mapped_input *press = &data.emulated_keys[MAPPING_ACTION_PRESS_QUICK_KEY];
    key_type key = QUICK_KEYS[data.quick_key.selected].key;
    if (get_joystick_input_for_action(MAPPING_ACTION_PRESS_QUICK_KEY, press)) {
        handled = 1;
        if (press->state == INPUT_STATE_WENT_DOWN) {
            hotkey_key_pressed(key, KEY_MOD_NONE, 0);
        }
    } else if (press->state == INPUT_STATE_WENT_UP) {
        hotkey_key_released(key, KEY_MOD_NONE);
    }
    return handled;
}

static int translate_game_speed_toggle(void)
{
    if (!get_joystick_input_for_action(MAPPING_ACTION_TOGGLE_SLOW_GAME_SPEED, &data.slow_speed_toggle)) {
        return 0;
    }
    if (data.slow_speed_toggle.state == INPUT_STATE_WENT_DOWN) {
        int speed = setting_game_speed() == 70 ? 100 : 70;
        setting_reset_speeds(speed, setting_scroll_speed());
        char text[32];
        snprintf(text, sizeof(text), "Speed: %d%%", speed);
        show_city_message(text);
    }
    return 1;
}

static int is_city_window(void)
{
    return window_is(WINDOW_CITY) || window_is(WINDOW_CITY_MILITARY);
}

static int translate_show_empire(void)
{
    if (!get_joystick_input_for_action(MAPPING_ACTION_SHOW_EMPIRE, &data.show_empire)) {
        return 0;
    }
    if (data.show_empire.state == INPUT_STATE_WENT_DOWN && is_city_window()) {
        window_empire_show();
    }
    return 1;
}

static int translate_save_file(void)
{
    if (!get_joystick_input_for_action(MAPPING_ACTION_SAVE_FILE, &data.save_file)) {
        return 0;
    }
    if (data.save_file.state != INPUT_STATE_WENT_DOWN) {
        return 1;
    }
    if (window_is(WINDOW_FILE_DIALOG)) {
        // Second press in the dialog confirms, same as pressing Enter
        keyboard_return();
    } else if (is_city_window()) {
        char name[32];
        time_t now = time(0);
        struct tm *local = localtime(&now);
        if (!local || !strftime(name, sizeof(name), "%Y-%m-%d %H-%M-%S", local)) {
            snprintf(name, sizeof(name), "quicksave");
        }
        window_file_dialog_show_with_name(FILE_TYPE_SAVED_GAME, FILE_DIALOG_SAVE, name);
    }
    return 1;
}

static int translate_system_functions(void)
{
    if (get_joystick_input_for_action(MAPPING_ACTION_SHOW_VIRTUAL_KEYBOARD, &data.virtual_keyboard)) {
        if (data.virtual_keyboard.state == INPUT_STATE_WENT_DOWN) {
            if (keyboard_is_capturing()) {
                system_keyboard_show();
            }
        }
        return 1;
    }
    if (get_joystick_input_for_action(MAPPING_ACTION_CYCLE_TOUCH_TYPE, &data.touch_mode)) {
        if (data.touch_mode.state == INPUT_STATE_WENT_DOWN) {
            touch_cycle_mode();
        }
        return 1;
    }
    return 0;
}

int joystick_to_mouse_and_keyboard(void)
{
    if (data.connected_joysticks == 0) {
        return 0;
    }
    int handled = 0;
    if (!translate_mapping_reset()) {
        handled |= translate_mouse();
        handled |= translate_window_scrolling();
        handled |= translate_map_scrolling();
        handled |= translate_hotkeys();
        handled |= translate_emulated_keys();
        handled |= translate_quick_keys();
        handled |= translate_game_speed_toggle();
        handled |= translate_show_empire();
        handled |= translate_save_file();
        handled |= translate_system_functions();
    }
    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        memset(data.joystick[i].trackball, 0, sizeof(joystick_trackball) * MAX_TRACKBALLS);
    }
    if (handled) {
        mouse_remove_touch();
        data.cursor.active = 1;
    }
    return handled;
}

int joystick_is_cursor_active(void)
{
    return data.connected_joysticks > 0 && data.cursor.active;
}

void joystick_deactivate_cursor(void)
{
    data.cursor.active = 0;
}
