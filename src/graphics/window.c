#include "window.h"

#include "graphics/warning.h"
#include "input/cursor.h"
#include "input/touch.h"
#include "window/city.h"

#define MAX_QUEUE 3

static struct {
    window_type window_queue[MAX_QUEUE];
    int queue_index;
    window_type *current_window;
    int refresh_immediate;
    int refresh_on_draw;
    int underlying_windows_redrawing;
} data;

static void noop(void)
{
}
static void noop_mouse(const mouse *m)
{
}

static void increase_queue_index(void)
{
    data.queue_index++;
    if (data.queue_index >= MAX_QUEUE) {
        data.queue_index = 0;
    }
}

static void decrease_queue_index(void)
{
    data.queue_index--;
    if (data.queue_index < 0) {
        data.queue_index = MAX_QUEUE - 1;
    }
}

void window_invalidate(void)
{
    data.refresh_immediate = 1;
    data.refresh_on_draw = 1;
}

int window_is_invalid(void)
{
    return data.refresh_immediate;
}

void window_request_refresh(void)
{
    data.refresh_on_draw = 1;
}

int window_is(window_id id)
{
    return data.current_window->id == id;
}

window_id window_get_id(void)
{
    return data.current_window->id;
}

void window_show(const window_type *window)
{
    mouse_reset_button_state();
    reset_touches(1);
    increase_queue_index();
    data.window_queue[data.queue_index] = *window;
    data.current_window = &data.window_queue[data.queue_index];
    if (!data.current_window->draw_background) {
        data.current_window->draw_background = noop;
    }
    if (!data.current_window->draw_foreground) {
        data.current_window->draw_foreground = noop;
    }
    if (!data.current_window->handle_mouse) {
        data.current_window->handle_mouse = noop_mouse;
    }
    window_invalidate();
}

void window_go_back(void)
{
    decrease_queue_index();
    data.current_window = &data.window_queue[data.queue_index];
    window_invalidate();
}

static void update_mouse_before(void)
{
    if (!touch_to_mouse()) {
        mouse_determine_button_state();  // touch overrides mouse
    }
}

static void update_mouse_after(void)
{
    reset_touches(0);
    mouse_reset_scroll();
    input_cursor_update(data.current_window->id);
}

void window_draw(int force)
{
    update_mouse_before();
    window_type *w = data.current_window;
    if (force || data.refresh_on_draw) {
        tooltip_invalidate();
        w->draw_background();
        data.refresh_on_draw = 0;
        data.refresh_immediate = 0;
    }
    w->draw_foreground();

    const mouse *m = mouse_get();
    w->handle_mouse(m);
    tooltip_handle(m, w->get_tooltip);
    warning_draw();
    update_mouse_after();
}

void window_draw_underlying_window(void)
{
    if (data.underlying_windows_redrawing < MAX_QUEUE) {
        ++data.underlying_windows_redrawing;
        decrease_queue_index();
        window_type *window_behind = &data.window_queue[data.queue_index];
        if (window_behind->draw_background) {
            window_behind->draw_background();
        }
        if (window_behind->draw_foreground) {
            window_behind->draw_foreground();
        }
        increase_queue_index();
        --data.underlying_windows_redrawing;
    }
}
