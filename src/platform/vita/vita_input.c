#include "vita_input.h"
#include "vita_keyboard.h"
#include "vita.h"
#include <math.h>

#include "core/calc.h"
#include "core/encoding.h"
#include "core/string.h"
#include "game/system.h"
#include "input/mouse.h"

#define NO_MAPPING -1
#define MAX_VKBD_TEXT_SIZE 600

enum {
    VITA_PAD_TRIANGLE   = 0,
    VITA_PAD_CIRCLE     = 1,
    VITA_PAD_CROSS      = 2,
    VITA_PAD_SQUARE     = 3,
    VITA_PAD_L          = 4,
    VITA_PAD_R          = 5,
    VITA_PAD_DOWN       = 6,
    VITA_PAD_LEFT       = 7,
    VITA_PAD_UP         = 8,
    VITA_PAD_RIGHT      = 9,
    VITA_PAD_SELECT     = 10,
    VITA_PAD_START      = 11,
    VITA_NUM_BUTTONS    = 12
};

enum {
    ANALOG_DOWN = 0,
    ANALOG_LEFT = 1,
    ANALOG_UP = 2,
    ANALOG_RIGHT = 3,
    ANALOG_MAX = 4
};

static int can_change_touch_mode = 1;

static SDL_Joystick *joy = NULL;

static struct {
    char utf8_text[MAX_VKBD_TEXT_SIZE];
    int requested;
    int max_length;
} vkbd;

static int hires_dx = 0; // sub-pixel-precision counters to allow slow pointer motion of <1 pixel per frame
static int hires_dy = 0;
static int pressed_buttons[VITA_NUM_BUTTONS];
static int right_analog_state[ANALOG_MAX];
static SDL_Keycode map_vita_button_to_sdlkey[VITA_NUM_BUTTONS] =
{
    SDLK_PAGEUP,    // VITA_PAD_TRIANGLE
    NO_MAPPING,     // VITA_PAD_CIRCLE
    NO_MAPPING,     // VITA_PAD_CROSS
    SDLK_PAGEDOWN,  // VITA_PAD_SQUARE
    NO_MAPPING,     // VITA_PAD_L
    NO_MAPPING,     // VITA_PAD_R
    SDLK_DOWN,      // VITA_PAD_DOWN
    SDLK_LEFT,      // VITA_PAD_LEFT
    SDLK_UP,        // VITA_PAD_UP
    SDLK_RIGHT,     // VITA_PAD_RIGHT
    NO_MAPPING,     // VITA_SELECT
    NO_MAPPING      // VITA_START
};
#define ANALOG_DIRECTION_TO_SDLKEY_OFFSET 6

static uint8_t map_vita_button_to_sdlmousebutton[VITA_NUM_BUTTONS] =
{
    NO_MAPPING,          // VITA_PAD_TRIANGLE
    SDL_BUTTON_RIGHT,    // VITA_PAD_CIRCLE
    SDL_BUTTON_LEFT,     // VITA_PAD_CROSS
    NO_MAPPING,          // VITA_PAD_SQUARE
    SDL_BUTTON_RIGHT,    // VITA_PAD_L
    SDL_BUTTON_LEFT,     // VITA_PAD_R
    NO_MAPPING,          // VITA_PAD_DOWN
    NO_MAPPING,          // VITA_PAD_LEFT
    NO_MAPPING,          // VITA_PAD_UP
    NO_MAPPING,          // VITA_PAD_RIGHT
    NO_MAPPING,          // VITA_SELECT
    NO_MAPPING           // VITA_START
};

static void vita_start_text_input(void);
static void vita_rescale_analog(int *x, int *y, int dead);
static void vita_button_to_sdlkey_event(int vita_button, SDL_Event *event, uint32_t event_type);
static void vita_button_to_sdlmouse_event(int vita_button, SDL_Event *event, uint32_t event_type);

static void vita_create_and_push_sdlkey_event(uint32_t event_type, SDL_Scancode scan, SDL_Keycode key);
static void vita_create_key_event_for_direction(int direction, int key_pressed);

void platform_per_frame_callback(void)
{
    if (vkbd.requested) {
        vita_start_text_input();
        vkbd.requested = 0;
    }
    vita_handle_analog_sticks();
}

void platform_show_virtual_keyboard(const uint8_t *text, int max_length)
{
    vkbd.max_length = calc_bound(max_length, 0, MAX_VKBD_TEXT_SIZE);
    encoding_to_utf8(text, vkbd.utf8_text, MAX_VKBD_TEXT_SIZE, 0);
    vkbd.requested = 1;
}

void platform_hide_virtual_keyboard(void)
{}

int vita_poll_event(SDL_Event *event)
{
    int ret = SDL_PollEvent(event);
    if (event != NULL) {
        switch (event->type) {
            case SDL_JOYBUTTONDOWN:
                if (event->jbutton.which != 0) { // Only Joystick 0 controls the game
                    break;
                }
                switch (event->jbutton.button) {
                    case VITA_PAD_SQUARE:
                    case VITA_PAD_TRIANGLE:
                    case VITA_PAD_UP:
                    case VITA_PAD_DOWN:
                    case VITA_PAD_LEFT:
                    case VITA_PAD_RIGHT: // intentional fallthrough
                        vita_button_to_sdlkey_event(event->jbutton.button, event, SDL_KEYDOWN);
                        break;
                    case VITA_PAD_CROSS:
                    case VITA_PAD_R:
                    case VITA_PAD_CIRCLE:
                    case VITA_PAD_L: // intentional fallthrough
                        vita_button_to_sdlmouse_event(event->jbutton.button, event, SDL_MOUSEBUTTONDOWN);
                        break;
                    case VITA_PAD_START:
                        vkbd.requested = 1;
                        break;
                    case VITA_PAD_SELECT:
                        if (can_change_touch_mode) {
                            touch_cycle_mode();
                            can_change_touch_mode = 0;
                        }
                        break;
                    default:
                        break;
                }
                break;
            case SDL_JOYBUTTONUP:
                if (event->jbutton.which != 0) { // Only Joystick 0 controls the game
                    break;
                }
                switch (event->jbutton.button) {
                    case VITA_PAD_SQUARE:
                    case VITA_PAD_TRIANGLE:
                    case VITA_PAD_UP:
                    case VITA_PAD_DOWN:
                    case VITA_PAD_LEFT:
                    case VITA_PAD_RIGHT: // intentional fallthrough
                        vita_button_to_sdlkey_event(event->jbutton.button, event, SDL_KEYUP);
                        break;
                    case VITA_PAD_CROSS:
                    case VITA_PAD_R:
                    case VITA_PAD_CIRCLE:
                    case VITA_PAD_L: // intentional fallthrough
                        vita_button_to_sdlmouse_event(event->jbutton.button, event, SDL_MOUSEBUTTONUP);
                        break;
                    case VITA_PAD_SELECT:
                        can_change_touch_mode = 1;
                        break;
                    default:
                        break;
                }
            default:
                break;
        }
    }
    return ret;
}

void vita_handle_analog_sticks(void)
{
    if (!joy) {
        joy = SDL_JoystickOpen(0);
    }
    int left_x = SDL_JoystickGetAxis(joy, 0);
    int left_y = SDL_JoystickGetAxis(joy, 1);
    vita_rescale_analog(&left_x, &left_y, 3000);
    hires_dx += left_x; // sub-pixel precision to allow slow mouse motion at speeds < 1 pixel/frame
    hires_dy += left_y;

    const int slowdown = 4096;

    if (hires_dx != 0 || hires_dy != 0) {
        int xrel = hires_dx / slowdown;
        int yrel = hires_dy / slowdown;
        hires_dx %= slowdown;
        hires_dy %= slowdown;
        if (xrel != 0 || yrel != 0) {
            // limit joystick mouse to screen coords, same as physical mouse
            int last_mouse_x = mouse_get()->x;
            int last_mouse_y = mouse_get()->y;
            int x = last_mouse_x + xrel;
            int y = last_mouse_y + yrel;
            if (x < 0) {
                x = 0;
                xrel = 0 - last_mouse_x;
            }
            if (x >= VITA_DISPLAY_WIDTH) {
                x = VITA_DISPLAY_WIDTH - 1;
                xrel = x - last_mouse_x;
            }
            if (y < 0) {
                y = 0;
                yrel = 0 - last_mouse_y;
            }
            if (y >= VITA_DISPLAY_HEIGHT) {
                y = VITA_DISPLAY_HEIGHT - 1;
                yrel = y - last_mouse_y;
            }
            SDL_Event event;
            event.type = SDL_MOUSEMOTION;
            event.motion.x = x;
            event.motion.y = y;
            event.motion.xrel = xrel;
            event.motion.yrel = yrel;
            SDL_PushEvent(&event);
        }
    }

    // map right stick to cursor keys
    float right_x = SDL_JoystickGetAxis(joy, 2);
    float right_y = -1 * SDL_JoystickGetAxis(joy, 3);
    float right_joy_dead_zone_squared = 10240.0*10240.0;
    float slope = 0.414214f; // tangent of 22.5 degrees for size of angular zones

    int direction_states[ANALOG_MAX] = { 0, 0, 0, 0 };

    if (right_x * right_x + right_y * right_y > right_joy_dead_zone_squared) {
        if (right_y > 0 && right_x > 0) {
            // upper right quadrant
            if (right_y > slope *right_x) {
                direction_states[ANALOG_UP] = 1;
            }
            if (right_x > slope *right_y) {
                direction_states[ANALOG_RIGHT] = 1;
            }
        } else if (right_y > 0 && right_x <= 0) {
            // upper left quadrant
            if (right_y > slope * -right_x) {
                direction_states[ANALOG_UP] = 1;
            }
            if (-right_x > slope *right_y) {
                direction_states[ANALOG_LEFT] = 1;
            }
        } else if (right_y <= 0 && right_x > 0) {
            // lower right quadrant
            if (-right_y > slope *right_x) {
                direction_states[ANALOG_DOWN] = 1;
            }
            if (right_x > slope * -right_y) {
                direction_states[ANALOG_RIGHT] = 1;
            }
        } else if (right_y <= 0 && right_x <= 0) {
            // lower left quadrant
            if (-right_y > slope * -right_x) {
                direction_states[ANALOG_DOWN] = 1;
            }
            if (-right_x > slope * -right_y) {
                direction_states[ANALOG_LEFT] = 1;
            }
        }
    }

    for (int direction = 0; direction < ANALOG_MAX; ++direction) {
        if (right_analog_state[direction] != direction_states[direction]) {
            right_analog_state[direction] = direction_states[direction];
            vita_create_key_event_for_direction(direction, direction_states[direction]);
        }
    }
}

static void vita_start_text_input(void)
{
    char *text = vita_keyboard_get("Enter New Text:", vkbd.utf8_text, vkbd.max_length);
    if (text == NULL)  {
        return;
    }
    for (int i = 0; i < MAX_VKBD_TEXT_SIZE; i++) {
        vita_create_and_push_sdlkey_event(SDL_KEYDOWN, SDL_SCANCODE_BACKSPACE, SDLK_BACKSPACE);
        vita_create_and_push_sdlkey_event(SDL_KEYUP, SDL_SCANCODE_BACKSPACE, SDLK_BACKSPACE);
    }
    for (int i = 0; i < MAX_VKBD_TEXT_SIZE; i++) {
        vita_create_and_push_sdlkey_event(SDL_KEYDOWN, SDL_SCANCODE_DELETE, SDLK_DELETE);
        vita_create_and_push_sdlkey_event(SDL_KEYUP, SDL_SCANCODE_DELETE, SDLK_DELETE);
    }
    for (int i = 0; i < MAX_VKBD_TEXT_SIZE - 1 && text[i];) {
        int bytes_in_char = encoding_get_utf8_character_bytes(text[i]);
        SDL_Event textinput_event;
        textinput_event.type = SDL_TEXTINPUT;
        for (int n = 0; n < bytes_in_char; n++) {
            textinput_event.text.text[n] = text[i + n];
        }
        textinput_event.text.text[bytes_in_char] = 0;
        SDL_PushEvent(&textinput_event);
        i += bytes_in_char;
    }
}

static void vita_rescale_analog(int *x, int *y, int dead)
{
    //radial and scaled dead_zone
    //http://www.third-helix.com/2013/04/12/doing-thumbstick-dead-zones-right.html
    //input and output values go from -32767...+32767;

    //the maximum is adjusted to account for SCE_CTRL_MODE_DIGITALANALOG_WIDE
    //where a reported maximum axis value corresponds to 80% of the full range
    //of motion of the analog stick

    if (dead == 0) {
        return;
    }
    if (dead >= 32767) {
        *x = 0;
        *y = 0;
        return;
    }

    const float max_axis = 32767.0f;
    float analog_x = (float) *x;
    float analog_y = (float) *y;
    float dead_zone = (float) dead;

    float magnitude = sqrtf(analog_x * analog_x + analog_y * analog_y);
    if (magnitude >= dead_zone) {
        //adjust maximum magnitude
        float abs_analog_x = fabs(analog_x);
        float abs_analog_y = fabs(analog_y);
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
        abs_analog_x = fabs(analog_x);
        abs_analog_y = fabs(analog_y);
        if (abs_analog_x > max_axis || abs_analog_y > max_axis){
            if (abs_analog_x > abs_analog_y) {
                clamping_factor = max_axis / abs_analog_x;
            } else {
                clamping_factor = max_axis / abs_analog_y;
            }
        }
        *x = (int) (clamping_factor * analog_x);
        *y = (int) (clamping_factor * analog_y);
    } else {
        *x = 0;
        *y = 0;
    }
}

static void vita_button_to_sdlkey_event(int vita_button, SDL_Event *event, uint32_t event_type)
{
    event->type = event_type;
    event->key.keysym.sym = map_vita_button_to_sdlkey[vita_button];
    event->key.keysym.mod = 0;
    event->key.repeat = 0;

    if (event_type == SDL_KEYDOWN) {
        pressed_buttons[vita_button] = 1;
    }
    if (event_type == SDL_KEYUP) {
        pressed_buttons[vita_button] = 0;
    }
}

static void vita_button_to_sdlmouse_event(int vita_button, SDL_Event *event, uint32_t event_type)
{
    event->type = event_type;
    event->button.button = map_vita_button_to_sdlmousebutton[vita_button];
    if (event_type == SDL_MOUSEBUTTONDOWN) {
        event->button.state = SDL_PRESSED;
        pressed_buttons[vita_button] = 1;
    }
    if (event_type == SDL_MOUSEBUTTONUP) {
        event->button.state = SDL_RELEASED;
        pressed_buttons[vita_button] = 0;
    }
    event->button.x = mouse_get()->x;
    event->button.y = mouse_get()->y;
}

static void vita_create_and_push_sdlkey_event(uint32_t event_type, SDL_Scancode scan, SDL_Keycode key)
{
    SDL_Event event;
    event.type = event_type;
    event.key.keysym.scancode = scan;
    event.key.keysym.sym = key;
    event.key.keysym.mod = 0;
    SDL_PushEvent(&event);
}

static void vita_create_key_event_for_direction(int direction, int key_pressed)
{
    uint32_t event_type = key_pressed ? SDL_KEYDOWN : SDL_KEYUP;
    switch (direction) {
        case ANALOG_UP:
            vita_create_and_push_sdlkey_event(event_type, SDL_SCANCODE_UP, SDLK_UP);
            break;
        case ANALOG_DOWN:
            vita_create_and_push_sdlkey_event(event_type, SDL_SCANCODE_DOWN, SDLK_DOWN);
            break;
        case ANALOG_LEFT:
            vita_create_and_push_sdlkey_event(event_type, SDL_SCANCODE_LEFT, SDLK_LEFT);
            break;
        case ANALOG_RIGHT:
            vita_create_and_push_sdlkey_event(event_type, SDL_SCANCODE_RIGHT, SDLK_RIGHT);
            break;
    }
}
