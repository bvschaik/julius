#include "virtual_keyboard.h"

#include "platform/definitions.h"

#include "SDL.h"

static struct
{
    int is_active;
    input_box input_location;
} data;

#ifdef SHOW_VIRTUAL_KEYBOARD
static int is_mouse_inside_input(const mouse *m)
{
    return (m->x > data.input_location.x && m->x < (data.input_location.x + data.input_location.w) &&
            m->y > data.input_location.y && m->y < (data.input_location.y + data.input_location.h));
}
#endif

static int is_input_location_valid(void)
{
    return (data.input_location.w != 0 && data.input_location.h != 0);
}

void virtual_keyboard_handle_mouse(const mouse *m)
{
    if (!data.is_active || !m->left.went_up) {
        return;
    }
#ifdef SHOW_VIRTUAL_KEYBOARD
    if (!SDL_IsTextInputActive() && is_mouse_inside_input(m)) {
        SDL_StartTextInput();
    } else if (SDL_IsTextInputActive() && !is_mouse_inside_input(m)) {
        SDL_StopTextInput();
    }
#endif
}

void platform_start_virtual_keyboard(const input_box *capture_box)
{
#ifdef SHOW_VIRTUAL_KEYBOARD
    SDL_StopTextInput();
#endif
    data.input_location.x = capture_box->x;
    data.input_location.y = capture_box->y;
    data.input_location.w = capture_box->w * INPUT_BOX_BLOCK_SIZE;
    data.input_location.h = capture_box->h * INPUT_BOX_BLOCK_SIZE;
    if (is_input_location_valid()) {
        data.is_active = 1;
    }
}

void platform_resume_virtual_keyboard(void)
{
    if (is_input_location_valid()) {
        data.is_active = 1;
    }
}

void platform_stop_virtual_keyboard(void)
{
#ifdef SHOW_VIRTUAL_KEYBOARD
    SDL_StopTextInput();
#endif
    data.is_active = 0;
}
