#include "window.h"

#include "graphics/warning.h"
#include "input/cursor.h"
#include "window/city.h"

static void noop()
{
}

static window_type previous_window;
static window_type current_window;
static int refreshRequested;

void window_invalidate()
{
    refreshRequested = 1;
}

int window_is(window_id id)
{
    return current_window.id == id;
}

window_id window_get_id()
{
    return current_window.id;
}

void window_show(const window_type *window)
{
    previous_window = current_window;
    current_window = *window;
    if (!current_window.draw_background) {
        current_window.draw_background = noop;
    }
    if (!current_window.draw_foreground) {
        current_window.draw_foreground = noop;
    }
    if (!current_window.handle_mouse) {
        current_window.handle_mouse = noop;
    }
    window_invalidate();
}

void UI_Window_goBack()
{
    current_window = previous_window;
    window_invalidate();
}

static void updateMouseBefore()
{
    mouse_determine_button_state();
}

static void updateMouseAfter()
{
    mouse_set_scroll(SCROLL_NONE);
    input_cursor_update(current_window.id);
}

void window_draw(int force)
{
    updateMouseBefore();
    if (force || refreshRequested) {
        current_window.draw_background();
        refreshRequested = 0;
    }
    current_window.draw_foreground();

    const mouse *m = mouse_get();
    current_window.handle_mouse(m);
    tooltip_handle(m, current_window.get_tooltip);
    warning_draw();
    updateMouseAfter();
}
