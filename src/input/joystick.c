#include "joystick.h"

#include "core/log.h"
#include "core/time.h"
#include "game/system.h"
#include "graphics/window.h"
#include "input/hotkey.h"
#include "input/mouse.h"
#include "input/scroll.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef int joystick_axis;
typedef int joystick_button;

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
    joystick_axis axis[JOYSTICK_MAX_AXIS];
    joystick_button button[JOYSTICK_MAX_BUTTONS];
    joystick_trackball trackball[JOYSTICK_MAX_TRACKBALLS];
    joystick_hat hat[JOYSTICK_MAX_HATS];
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

#define JOYSTICK_HOTKEY_OFFSET MAPPING_ACTION_ROTATE_MAP_LEFT
#define JOYSTICK_MAX_HOTKEYS 6

static struct {
    joystick_model connected_models[JOYSTICK_MAX_CONTROLLERS];
    joystick_info joystick[JOYSTICK_MAX_CONTROLLERS];
    int connected_joysticks;
    struct {
        int x_delta;
        int y_delta;
        int left_button;
        int middle_button;
        int right_button;
        scroll_state scroll;
        time_millis last_scroll_time;
    } mouse;
    mapped_input map_scroll[NUM_DIRECTIONS];
    mapped_input joystick_hotkey[JOYSTICK_MAX_HOTKEYS];
} data;

static hotkey_action JOYSTICK_MAPPING_TO_HOTKEY_ACTION[JOYSTICK_MAX_HOTKEYS] = {
    HOTKEY_ROTATE_MAP_LEFT,
    HOTKEY_ROTATE_MAP_RIGHT,
    HOTKEY_INCREASE_GAME_SPEED,
    HOTKEY_DECREASE_GAME_SPEED,
    HOTKEY_TOGGLE_PAUSE,
    HOTKEY_CYCLE_LEGION
};

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

static void joystick_reset_all_button_states(joystick_info *joystick)
{
    memset(joystick->button, 0, sizeof(joystick_button) * JOYSTICK_MAX_BUTTONS);
}

static void joystick_reset_all_axis_states(joystick_info *joystick)
{
    memset(joystick->axis, 0, sizeof(int) * JOYSTICK_MAX_AXIS);
}

static void joystick_reset_all_hat_states(joystick_info *joystick)
{
    memset(joystick->hat, 0, sizeof(joystick_hat) * JOYSTICK_MAX_HATS);
}

static void joystick_reset_all_trackball_states(joystick_info *joystick)
{
    memset(joystick->trackball, 0, sizeof(joystick_trackball) * JOYSTICK_MAX_TRACKBALLS);
}

static void joystick_reset_state(joystick_info *joystick)
{
    joystick_reset_all_axis_states(joystick);
    joystick_reset_all_button_states(joystick);
    joystick_reset_all_trackball_states(joystick);
    joystick_reset_all_hat_states(joystick);
}

static joystick_info *get_free_joystick(void)
{
    for (int i = 0; i < JOYSTICK_MAX_CONTROLLERS; ++i) {
        if (!data.joystick[i].connected) {
            return &data.joystick[i];
        }
    }
    return 0;
}

static joystick_info *get_joystick_from_id(int joystick_id)
{
    for (int i = 0; i < JOYSTICK_MAX_CONTROLLERS; ++i) {
        if (data.joystick[i].id == joystick_id && data.joystick[i].connected) {
            return &data.joystick[i];
        }
    }
    return 0;
}

int joystick_add(int joystick_id, joystick_model *model)
{
    joystick_info *joystick = get_free_joystick();
    if (!joystick) {
        return 0;
    }
    joystick->id = joystick_id;
    joystick->model = model;
    joystick->connected = 1;
    model->connected_joysticks++;
    data.connected_joysticks++;
    return 1;
}

int joystick_is_listened(int joystick_id)
{
    return get_joystick_from_id(joystick_id) != 0;
}

int joystick_remove(int joystick_id)
{
    joystick_info *joystick = get_joystick_from_id(joystick_id);
    if (!joystick) {
        return 0;
    }
    joystick->connected = 0;
    joystick->model->connected_joysticks--;
    joystick->model = 0;
    joystick_reset_state(joystick);
    data.connected_joysticks--;
    return 1;
}

void joystick_update_element(int joystick_id, joystick_element element, int element_id, int value1, int value2)
{
    joystick_info *joystick = get_joystick_from_id(joystick_id);
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
            joystick->axis[element_id] = (abs(value1) > 1300) ? value1 : 0;
            break;
        case JOYSTICK_ELEMENT_TRACKBALL:
            update_trackball(&joystick->trackball[element_id], value1, value2);
            break;
        default:
            log_info("Trying to update wrong joystick element", 0, element);
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

static int get_joystick_input_for_action(mapping_action action, mapped_input *input)
{
    static mapped_input dummy_input;
    if (!input) {
        input = &dummy_input;
    }

    input->value = 0;
    input->element = JOYSTICK_ELEMENT_NONE;

    for (int i = 0; i < JOYSTICK_MAX_CONTROLLERS; ++i) {
        const joystick_info *joystick = &data.joystick[i];
        if (!joystick->connected) {
            continue;
        }
        if(get_input_for_mapping(joystick, &joystick->model->first_mapping[action], input) ||
           get_input_for_mapping(joystick, &joystick->model->second_mapping[action], input)) {
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
        switch(input->element)
        {
            case JOYSTICK_ELEMENT_TRACKBALL:
                input->value *= 5; // TODO is this a high enough value? I doubt it
                break;
            default:
                input->value *= 32767;
        }
    } else if (translated_element == JOYSTICK_ELEMENT_TRACKBALL) {
        switch (input->element) {
            case JOYSTICK_ELEMENT_AXIS:
                input->value /= 5; // TODO is this a high enough value? I doubt it
                break;
            default:
                input->value *= 5;
        }
    } else if (translated_element == JOYSTICK_ELEMENT_BUTTON || translated_element == JOYSTICK_ELEMENT_HAT) {
        input->value = (input->value != 0) ? 1 : 0;
    }
    input->element = translated_element;
}

static int rescale_axis(mapped_input *inputs)
{
    //radial and scaled dead_zone
    //http://www.third-helix.com/2013/04/12/doing-thumbstick-dead-zones-right.html
    //input and output values go from -32767...+32767;

    //the maximum is adjusted to account for SCE_CTRL_MODE_DIGITALANALOG_WIDE
    //where a reported maximum axis value corresponds to 80% of the full range
    //of motion of the analog stick
    const float max_axis = 32767.0f;
    float analog_x = (float) inputs[DIRECTION_RIGHT].value - inputs[DIRECTION_LEFT].value;
    float analog_y = (float) inputs[DIRECTION_DOWN].value - inputs[DIRECTION_UP].value;
    float dead_zone = 2000.0f;

    inputs[DIRECTION_UP].value = 0;
    inputs[DIRECTION_LEFT].value = 0;
    inputs[DIRECTION_DOWN].value = 0;
    inputs[DIRECTION_RIGHT].value = 0;

    float magnitude = sqrtf(analog_x * analog_x + analog_y * analog_y);
    if (magnitude < dead_zone) {
        return 0;
    }
    //adjust maximum magnitude
    float abs_analog_x = fabsf(analog_x);
    float abs_analog_y = fabsf(analog_y);
    float max_x;
    float max_y;
    if (abs_analog_x > abs_analog_y) {
        max_x = max_axis;
        max_y = (max_axis * analog_y) / abs_analog_x;
    } else {
        max_x = (max_axis * analog_x) / abs_analog_y;
        max_y = max_axis;
    }
    float maximum = sqrtf(max_x * max_x + max_y * max_y);
    if (maximum > 1.25f * max_axis) maximum = 1.25f * max_axis;
    if (maximum < magnitude) maximum = magnitude;

    // find scaled axis values with magnitudes between zero and maximum
    float scalingFactor = maximum / magnitude * (magnitude - dead_zone) / (maximum - dead_zone);
    analog_x = (analog_x * scalingFactor);
    analog_y = (analog_y * scalingFactor);

    // clamp to ensure results will never exceed the max_axis value
    float clamping_factor = 1.0f;
    abs_analog_x = fabsf(analog_x);
    abs_analog_y = fabsf(analog_y);
    if (abs_analog_x > max_axis || abs_analog_y > max_axis) {
        if (abs_analog_x > abs_analog_y) {
            clamping_factor = max_axis / abs_analog_x;
        } else {
            clamping_factor = max_axis / abs_analog_y;
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
    if (!get_joystick_input_for_action(MAPPING_ACTION_RESET_MAPPING, 0)) {
        return 0;
    }
    return 1;
}

static int translate_mouse_cursor_position(void)
{
    mapped_input cursor_input[NUM_DIRECTIONS];

    int handled = get_joystick_input_for_action(MAPPING_ACTION_MOUSE_CURSOR_UP, &cursor_input[DIRECTION_UP]);
    handled |= get_joystick_input_for_action(MAPPING_ACTION_MOUSE_CURSOR_LEFT, &cursor_input[DIRECTION_LEFT]);
    handled |= get_joystick_input_for_action(MAPPING_ACTION_MOUSE_CURSOR_DOWN, &cursor_input[DIRECTION_DOWN]);
    handled |= get_joystick_input_for_action(MAPPING_ACTION_MOUSE_CURSOR_RIGHT, &cursor_input[DIRECTION_RIGHT]);
    if (!handled) {
        return 0;
    }

    // All mouse cursor input will internally be treated as a joystick axis
    translate_input_for_element(&cursor_input[DIRECTION_UP], JOYSTICK_ELEMENT_AXIS);
    translate_input_for_element(&cursor_input[DIRECTION_LEFT], JOYSTICK_ELEMENT_AXIS);
    translate_input_for_element(&cursor_input[DIRECTION_DOWN], JOYSTICK_ELEMENT_AXIS);
    translate_input_for_element(&cursor_input[DIRECTION_RIGHT], JOYSTICK_ELEMENT_AXIS);

    if (!rescale_axis(cursor_input)) {
        return 0;
    }
    
    int slowdown = CURSOR_SLOWDOWN_NORMAL;
    if (get_joystick_input_for_action(MAPPING_ACTION_FASTER_MOUSE_CURSOR_SPEED, 0)) {
        slowdown = CURSOR_SLOWDOWN_FASTER;
    } else if (get_joystick_input_for_action(MAPPING_ACTION_SLOWER_MOUSE_CURSOR_SPEED, 0)) {
        slowdown = CURSOR_SLOWDOWN_SLOWER;
    }
    int delta_x = -cursor_input[DIRECTION_LEFT].value + cursor_input[DIRECTION_RIGHT].value;
    int delta_y = -cursor_input[DIRECTION_LEFT].value + cursor_input[DIRECTION_RIGHT].value;

    // Sub-pixel precision to allow slow mouse motion at speeds < 1 pixel/frame
    data.mouse.x_delta += cursor_input[DIRECTION_RIGHT].value - cursor_input[DIRECTION_LEFT].value;
    data.mouse.y_delta += cursor_input[DIRECTION_DOWN].value - cursor_input[DIRECTION_UP].value;

    if (data.mouse.x_delta == 0 && data.mouse.y_delta == 0) {
        return 1;
    }

    delta_x = data.mouse.x_delta / slowdown;
    delta_y = data.mouse.y_delta / slowdown;
    data.mouse.x_delta %= slowdown;
    data.mouse.y_delta %= slowdown;

    system_move_mouse_cursor(delta_x, delta_y);
    return 1;
}

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
        max_scroll_time = max_scroll_time * 32767 / scroll_up.value;
    } else if (scroll_down.value) {
        current_scroll = SCROLL_DOWN;
        max_scroll_time = max_scroll_time * 32767 / scroll_down.value;
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

static int translate_hotkeys(void)
{
    int handled = 0;
    for (int i = 0; i < JOYSTICK_MAX_HOTKEYS; ++i) {
        int value = get_joystick_input_for_action(JOYSTICK_HOTKEY_OFFSET + i, &data.joystick_hotkey[i]);
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
    }
    for (int i = 0; i < JOYSTICK_MAX_CONTROLLERS; ++i) {
        joystick_reset_all_trackball_states(&data.joystick[i]);
    }
    return handled;
}
