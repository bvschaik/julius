#include "vita_input.h"
#include "vita_touch.h"
#include "vita.h"
#include <math.h>

#include "vita_keyboard.h"
#define NO_MAPPING -1

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

int last_mouse_x = 0;
int last_mouse_y = 0;
int touch_mode = TOUCH_MODE_TOUCHPAD;
static bool can_change_touch_mode = true;

static SDL_Joystick *joy = NULL;

static int hires_dx = 0; // sub-pixel-precision counters to allow slow pointer motion of <1 pixel per frame
static int hires_dy = 0;
static int vkbd_requested = 0;
static int pressed_buttons[VITA_NUM_BUTTONS] = { 0 };
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

static void vita_start_text_input(char *initial_text, int multiline);
static void vita_rescale_analog(int *x, int *y, int dead);
static void vita_button_to_sdlkey_event(int vita_button, SDL_Event *event, uint32_t event_type);
static void vita_button_to_sdlmouse_event(int vita_button, SDL_Event *event, uint32_t event_type);

static void vita_create_and_push_sdlkey_event(uint32_t event_type, SDL_Scancode scan, SDL_Keycode key);

int vita_poll_event(SDL_Event *event)
{
    int ret = SDL_PollEvent(event);
    if (event != NULL) {
        if (touch_mode != TOUCH_MODE_ORIGINAL) {
            vita_handle_touch(event);
        }
        switch (event->type) {
            case SDL_FINGERDOWN:
            case SDL_FINGERUP:
            case SDL_FINGERMOTION: // intentional fallthrough
                // ignore rear panel touch events
                if (event->tfinger.touchId != 0) {
                    event->type = SDL_USEREVENT;
                    event->user.code = -1; // ensure that this event is ignored
                }
                break;
            case SDL_MOUSEMOTION:
                // update joystick / touch mouse coords
                last_mouse_x = event->motion.x;
                last_mouse_y = event->motion.y;
                break;
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
                        vkbd_requested = 1;
                        break;
                    case VITA_PAD_SELECT:
                        if (can_change_touch_mode) {
                            touch_mode++;
                            touch_mode %= NUM_TOUCH_MODES;
                            can_change_touch_mode = false;
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
                        can_change_touch_mode = true;
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


void vita_handle_repeat_keys(void)
{
    if (pressed_buttons[VITA_PAD_UP]) {
        vita_create_and_push_sdlkey_event(SDL_KEYDOWN, SDL_SCANCODE_UP, SDLK_UP);
    } else if (pressed_buttons[VITA_PAD_DOWN]) {
        vita_create_and_push_sdlkey_event(SDL_KEYDOWN, SDL_SCANCODE_DOWN, SDLK_DOWN);
    }
    if (pressed_buttons[VITA_PAD_LEFT]) {
        vita_create_and_push_sdlkey_event(SDL_KEYDOWN, SDL_SCANCODE_LEFT, SDLK_LEFT);
    } else if (pressed_buttons[VITA_PAD_RIGHT]) {
        vita_create_and_push_sdlkey_event(SDL_KEYDOWN, SDL_SCANCODE_RIGHT, SDLK_RIGHT);
    }
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
            int x = last_mouse_x + xrel;
            int y = last_mouse_y + yrel;
            if (x < 0) {
                x = 0;
                xrel = 0 - last_mouse_x;
            }
            if (x > VITA_DISPLAY_WIDTH) {
                x = VITA_DISPLAY_WIDTH;
                xrel = VITA_DISPLAY_WIDTH - last_mouse_x;
            }
            if (y < 0) {
                y = 0;
                yrel = 0 - last_mouse_y;
            }
            if (y > VITA_DISPLAY_HEIGHT) {
                y = VITA_DISPLAY_HEIGHT;
                yrel = VITA_DISPLAY_HEIGHT - last_mouse_y;
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

    if ((right_x * right_x + right_y * right_y) <= right_joy_dead_zone_squared) {
        return;
    }

    int up = 0;
    int down = 0;
    int left = 0;
    int right = 0;

    if (right_y > 0 && right_x > 0) {
        // upper right quadrant
        if (right_y > slope * right_x) {
            up = 1;
        }
        if (right_x > slope * right_y) {
            right = 1;
        }
    } else if (right_y > 0 && right_x <= 0) {
        // upper left quadrant
        if (right_y > slope * (-right_x)) {
            up = 1;
        }
        if ((-right_x) > slope * right_y) {
            left = 1;
        }
    } else if (right_y <= 0 && right_x > 0) {
        // lower right quadrant
        if ((-right_y) > slope * right_x) {
            down = 1;
        }
        if (right_x > slope * (-right_y)) {
            right = 1;
        }
    } else if (right_y <= 0 && right_x <= 0) {
        // lower left quadrant
        if ((-right_y) > slope * (-right_x)) {
            down = 1;
        }
        if ((-right_x) > slope * (-right_y)) {
            left = 1;
        }
    }

    if (!pressed_buttons[VITA_PAD_UP] && up) {
        vita_create_and_push_sdlkey_event(SDL_KEYDOWN, SDL_SCANCODE_UP, SDLK_UP);
    } else if (!pressed_buttons[VITA_PAD_DOWN] && down) {
        vita_create_and_push_sdlkey_event(SDL_KEYDOWN, SDL_SCANCODE_DOWN, SDLK_DOWN);
    }
    if (!pressed_buttons[VITA_PAD_LEFT] && left) {
        vita_create_and_push_sdlkey_event(SDL_KEYDOWN, SDL_SCANCODE_LEFT, SDLK_LEFT);
    } else if (!pressed_buttons[VITA_PAD_RIGHT] && right) {
        vita_create_and_push_sdlkey_event(SDL_KEYDOWN, SDL_SCANCODE_RIGHT, SDLK_RIGHT);
    }
}

void vita_handle_virtual_keyboard(void)
{
    if (vkbd_requested) {
        vkbd_requested = 0;
        vita_start_text_input("", 0);
    }
}

static int get_utf8_character_bytes(const uint8_t *uc)
{
    if (uc[0] < 0x80) {
        return 1;
    } else if ((uc[0] & 0xe0) == 0xc0 && (uc[1] & 0xc0) == 0x80) {
        return 2;
    } else if ((uc[0] & 0xf0) == 0xe0 && (uc[1] & 0xc0) == 0x80 && (uc[2] & 0xc0) == 0x80) {
        return 3;
    } else if ((uc[0] & 0xf8) == 0xf0 && (uc[1] & 0xc0) == 0x80 && (uc[2] & 0xc0) == 0x80 && (uc[3] & 0xc0) == 0x80) {
        return 4;
    } else {
        return 1;
    }
}

static void vita_start_text_input(char *initial_text, int multiline)
{
    char *text = vita_keyboard_get("Enter New Text:", initial_text, 600, multiline);
    if (text == NULL)  {
        return;
    }
    for (int i = 0; i < 600; i++) {
        vita_create_and_push_sdlkey_event(SDL_KEYDOWN, SDL_SCANCODE_BACKSPACE, SDLK_BACKSPACE);
        vita_create_and_push_sdlkey_event(SDL_KEYUP, SDL_SCANCODE_BACKSPACE, SDLK_BACKSPACE);
    }
    for (int i = 0; i < 600; i++) {
        vita_create_and_push_sdlkey_event(SDL_KEYDOWN, SDL_SCANCODE_DELETE, SDLK_DELETE);
        vita_create_and_push_sdlkey_event(SDL_KEYUP, SDL_SCANCODE_DELETE, SDLK_DELETE);
    }
    const uint8_t *utf8_text = (uint8_t*) text;
    for (int i = 0; i < 599 && utf8_text[i];) {
        int bytes_in_char = get_utf8_character_bytes(&utf8_text[i]);
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
    event->button.x = last_mouse_x;
    event->button.y = last_mouse_y;
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
