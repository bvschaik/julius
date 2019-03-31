#include "switch_input.h"
#include "switch.h"
#include <math.h>

#include "switch_keyboard.h"
#define NO_MAPPING -1

enum {
    SWITCH_PAD_A        = 0,
    SWITCH_PAD_B        = 1,
    SWITCH_PAD_X        = 2,
    SWITCH_PAD_Y        = 3,
    SWITCH_LSTICK       = 4,
    SWITCH_RSTICK       = 5,
    SWITCH_PAD_L        = 6,
    SWITCH_PAD_R        = 7,
    SWITCH_PAD_ZL       = 8,
    SWITCH_PAD_ZR       = 9,
    SWITCH_PAD_PLUS     = 10,
    SWITCH_PAD_MINUS    = 11,
    SWITCH_PAD_LEFT     = 12,
    SWITCH_PAD_UP       = 13,
    SWITCH_PAD_RIGHT    = 14,
    SWITCH_PAD_DOWN     = 15,
    SWITCH_NUM_BUTTONS  = 16
};

int last_mouse_x = 0;
int last_mouse_y = 0;

static SDL_Joystick *joy = NULL;

static int hires_dx = 0; // sub-pixel-precision counters to allow slow pointer motion of <1 pixel per frame 
static int hires_dy = 0;
static int vkbd_requested = 0;
static int fast_mouse = 0;
static int slow_mouse = 0;
static int pressed_buttons[SWITCH_NUM_BUTTONS] = { 0 };
static SDL_Keycode map_switch_button_to_sdlkey[SWITCH_NUM_BUTTONS] =
{ 
    NO_MAPPING,     // SWITCH_PAD_A
    NO_MAPPING,     // SWITCH_PAD_B
    SDLK_PAGEUP,    // SWITCH_PAD_X
    SDLK_PAGEDOWN,  // SWITCH_PAD_Y
    NO_MAPPING,     // SWITCH_PAD_LSTICK
    NO_MAPPING,     // SWITCH_PAD_RSTICK
    NO_MAPPING,     // SWITCH_PAD_L
    NO_MAPPING,     // SWITCH_PAD_R
    NO_MAPPING,     // SWITCH_PAD_ZL
    NO_MAPPING,     // SWITCH_PAD_ZR
    NO_MAPPING,     // SWITCH_PAD_PLUS
    NO_MAPPING,     // SWITCH_PAD_MINUS
    SDLK_LEFT,      // SWITCH_PAD_LEFT
    SDLK_UP,        // SWITCH_PAD_UP
    SDLK_RIGHT,     // SWITCH_PAD_RIGHT
    SDLK_DOWN       // SWITCH_PAD_DOWN
};

static uint8_t map_switch_button_to_sdlmousebutton[SWITCH_NUM_BUTTONS] =
{
    SDL_BUTTON_LEFT,    // SWITCH_PAD_A
    SDL_BUTTON_RIGHT,   // SWITCH_PAD_B
    NO_MAPPING,         // SWITCH_PAD_X
    NO_MAPPING,         // SWITCH_PAD_Y
    NO_MAPPING,         // SWITCH_PAD_LSTICK
    NO_MAPPING,         // SWITCH_PAD_RSTICK
    SDL_BUTTON_RIGHT,   // SWITCH_PAD_L
    SDL_BUTTON_LEFT,    // SWITCH_PAD_R
    NO_MAPPING,         // SWITCH_PAD_ZL
    NO_MAPPING,         // SWITCH_PAD_ZR
    NO_MAPPING,         // SWITCH_PAD_PLUS
    NO_MAPPING,         // SWITCH_PAD_MINUS
    NO_MAPPING,         // SWITCH_PAD_LEFT
    NO_MAPPING,         // SWITCH_PAD_UP
    NO_MAPPING,         // SWITCH_PAD_RIGHT
    NO_MAPPING          // SWITCH_PAD_DOWN
};

static void switch_start_text_input(char *initial_text, int multiline);
static void switch_rescale_analog(int *x, int *y, int dead);
static void switch_button_to_sdlkey_event(int switch_button, SDL_Event *event, uint32_t event_type); 
static void switch_button_to_sdlmouse_event(int switch_button, SDL_Event *event, uint32_t event_type);

static void switch_create_and_push_sdlkey_event(uint32_t event_type, SDL_Keycode key);

int switch_poll_event(SDL_Event *event)
{
    int ret = SDL_PollEvent(event);
    if(event != NULL) {
        switch (event->type) {
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
                    case SWITCH_PAD_Y:
                    case SWITCH_PAD_X:
                    case SWITCH_PAD_UP:
                    case SWITCH_PAD_DOWN:
                    case SWITCH_PAD_LEFT:
                    case SWITCH_PAD_RIGHT: // intentional fallthrough
                        switch_button_to_sdlkey_event(event->jbutton.button, event, SDL_KEYDOWN);
                        break;
                    case SWITCH_PAD_B:
                    case SWITCH_PAD_R:
                    case SWITCH_PAD_A:
                    case SWITCH_PAD_L: // intentional fallthrough
                        switch_button_to_sdlmouse_event(event->jbutton.button, event, SDL_MOUSEBUTTONDOWN);
                        break;
                    case SWITCH_PAD_ZL:
                        fast_mouse = 1;
                        hires_dx = 0;
                        hires_dy = 0;
                        break;
                    case SWITCH_PAD_ZR:
                        slow_mouse = 1;
                        hires_dx = 0;
                        hires_dy = 0;
                        break;
                    case SWITCH_PAD_PLUS:
                        vkbd_requested = 1;
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
                    case SWITCH_PAD_Y:
                    case SWITCH_PAD_X:
                    case SWITCH_PAD_UP:
                    case SWITCH_PAD_DOWN:
                    case SWITCH_PAD_LEFT:
                    case SWITCH_PAD_RIGHT: // intentional fallthrough
                        switch_button_to_sdlkey_event(event->jbutton.button, event, SDL_KEYUP);
                        break;
                    case SWITCH_PAD_B:
                    case SWITCH_PAD_R:
                    case SWITCH_PAD_A:
                    case SWITCH_PAD_L: // intentional fallthrough
                        switch_button_to_sdlmouse_event(event->jbutton.button, event, SDL_MOUSEBUTTONUP);
                        break;
                    case SWITCH_PAD_ZL:
                        fast_mouse = 0;
                        hires_dx = 0;
                        hires_dy = 0;
                        break;
                    case SWITCH_PAD_ZR:
                        slow_mouse = 0;
                        hires_dx = 0;
                        hires_dy = 0;
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


void switch_handle_repeat_keys(void)
{
    if (pressed_buttons[SWITCH_PAD_UP]) {
        switch_create_and_push_sdlkey_event(SDL_KEYDOWN, SDLK_UP);
    } else if (pressed_buttons[SWITCH_PAD_DOWN]) {
        switch_create_and_push_sdlkey_event(SDL_KEYDOWN, SDLK_DOWN);
    }
    if (pressed_buttons[SWITCH_PAD_LEFT]) {
        switch_create_and_push_sdlkey_event(SDL_KEYDOWN, SDLK_LEFT);
    } else if (pressed_buttons[SWITCH_PAD_RIGHT]) {
        switch_create_and_push_sdlkey_event(SDL_KEYDOWN, SDLK_RIGHT);
    }
}

void switch_handle_analog_sticks(void)
{
    if (!joy) {
        joy = SDL_JoystickOpen(0);
    }
    int left_x = SDL_JoystickGetAxis(joy, 0);
    int left_y = SDL_JoystickGetAxis(joy, 1);
    switch_rescale_analog(&left_x, &left_y, 2000);
    hires_dx += left_x; // sub-pixel precision to allow slow mouse motion at speeds < 1 pixel/frame
    hires_dy += left_y;

    int slowdown = 4096;

    if (fast_mouse) {
        slowdown /= 3;
    }

    if (slow_mouse) {
        slowdown *= 8;
    }

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
            if (x > SWITCH_DISPLAY_WIDTH) {
                x = SWITCH_DISPLAY_WIDTH;
                xrel = SWITCH_DISPLAY_WIDTH - last_mouse_x;
            }
            if (y < 0) {
                y = 0;
                yrel = 0 - last_mouse_y;
            }
            if (y > SWITCH_DISPLAY_HEIGHT) {
                y = SWITCH_DISPLAY_HEIGHT;
                yrel = SWITCH_DISPLAY_HEIGHT - last_mouse_y;
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

    // upper right quadrant
    if (right_y > 0 && right_x > 0)
    {
        if (right_y > slope * right_x)
            up = 1;
        if (right_x > slope * right_y)
            right = 1;
    }
    // upper left quadrant
    else if (right_y > 0 && right_x <= 0)
    {
        if (right_y > slope * (-right_x))
            up = 1;
        if ((-right_x) > slope * right_y)
            left = 1;
    }
    // lower right quadrant
    else if (right_y <= 0 && right_x > 0)
    {
        if ((-right_y) > slope * right_x)
            down = 1;
        if (right_x > slope * (-right_y))
            right = 1;
    }
    // lower left quadrant
    else if (right_y <= 0 && right_x <= 0)
    {
        if ((-right_y) > slope * (-right_x))
            down = 1;
        if ((-right_x) > slope * (-right_y))
            left = 1;
    }

    if (!pressed_buttons[SWITCH_PAD_UP] && up) {
        switch_create_and_push_sdlkey_event(SDL_KEYDOWN, SDLK_UP);
    } else if (!pressed_buttons[SWITCH_PAD_DOWN] && down) {
        switch_create_and_push_sdlkey_event(SDL_KEYDOWN, SDLK_DOWN);
    }
    if (!pressed_buttons[SWITCH_PAD_LEFT] && left) {
        switch_create_and_push_sdlkey_event(SDL_KEYDOWN, SDLK_LEFT);
    } else if (!pressed_buttons[SWITCH_PAD_RIGHT] && right) {
        switch_create_and_push_sdlkey_event(SDL_KEYDOWN, SDLK_RIGHT);
    }
}

void switch_handle_virtual_keyboard(void)
{
    if (vkbd_requested) {
        vkbd_requested = 0;
        switch_start_text_input("", 0);
    }
}

void switch_start_text_input(char *initial_text, int multiline)
{
    char text[601] = {'\0'};
    switch_keyboard_get("Enter New Text:", initial_text, 600, multiline, text);
    if (text == NULL)  {
        return;
    }
    for (int i = 0; i < 600; i++) {
        switch_create_and_push_sdlkey_event(SDL_KEYDOWN, SDLK_BACKSPACE);
        switch_create_and_push_sdlkey_event(SDL_KEYUP, SDLK_BACKSPACE);
    }
    for (int i = 0; i < 600; i++) {
        switch_create_and_push_sdlkey_event(SDL_KEYDOWN, SDLK_DELETE);
        switch_create_and_push_sdlkey_event(SDL_KEYUP, SDLK_DELETE);
    }
    for (int i=0; i < 599; i++) {
        if (text[i] == 0) {
            break;
        }
        SDL_Event textinput_event;
        textinput_event.type = SDL_TEXTINPUT;
        textinput_event.text.text[0] = text[i];
        textinput_event.text.text[1] = 0;
        SDL_PushEvent(&textinput_event);
    }
}

void switch_rescale_analog(int *x, int *y, int dead)
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

void switch_button_to_sdlkey_event(int switch_button, SDL_Event *event, uint32_t event_type) 
{
    event->type = event_type;
    event->key.keysym.sym = map_switch_button_to_sdlkey[switch_button];
    event->key.keysym.mod = 0;
    event->key.repeat = 0;

    if (event_type == SDL_KEYDOWN) {
        pressed_buttons[switch_button] = 1;
    }
    if (event_type == SDL_KEYUP) {
        pressed_buttons[switch_button] = 0;
    }
}

void switch_button_to_sdlmouse_event(int switch_button, SDL_Event *event, uint32_t event_type)
{
    event->type = event_type;
    event->button.button = map_switch_button_to_sdlmousebutton[switch_button];
    if (event_type == SDL_MOUSEBUTTONDOWN) {
        event->button.state = SDL_PRESSED;
        pressed_buttons[switch_button] = 1;
    }
    if (event_type == SDL_MOUSEBUTTONUP) {
        event->button.state = SDL_RELEASED;
        pressed_buttons[switch_button] = 0;
    }
    event->button.x = last_mouse_x;
    event->button.y = last_mouse_y;
}

void switch_create_and_push_sdlkey_event(uint32_t event_type, SDL_Keycode key) 
{
    SDL_Event event;
    event.type = event_type;
    event.key.keysym.sym = key;
    event.key.keysym.mod = 0;
    SDL_PushEvent(&event);
}
