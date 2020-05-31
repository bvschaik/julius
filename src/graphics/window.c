#include "window.h"

#include "graphics/graphics.h"
#include "graphics/warning.h"
#include "input/cursor.h"
#include "input/hotkey.h"
#include "input/scroll.h"
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
static void noop_input(const mouse *m, const hotkeys *h)
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

static void reset_input(void)
{
    mouse_reset_button_state();
    reset_touches(1);
    scroll_stop();
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
    reset_input();
    increase_queue_index();
    data.window_queue[data.queue_index] = *window;
    data.current_window = &data.window_queue[data.queue_index];
    if (!data.current_window->draw_background) {
        data.current_window->draw_background = noop;
    }
    if (!data.current_window->draw_foreground) {
        data.current_window->draw_foreground = noop;
    }
    if (!data.current_window->handle_input) {
        data.current_window->handle_input = noop_input;
    }
    window_invalidate();
}

void window_go_back(void)
{
    reset_input();
    decrease_queue_index();
    data.current_window = &data.window_queue[data.queue_index];
    window_invalidate();
}

static void update_input_before(void)
{
    if (!touch_to_mouse()) {
        mouse_determine_button_state();  // touch overrides mouse
    }
    hotkey_handle_global_keys();
}

static void update_input_after(void)
{
    reset_touches(0);
    mouse_reset_scroll();
    input_cursor_update(data.current_window->id);
    hotkey_reset_state();
}

void window_draw(int force)
{
    update_input_before();
    window_type *w = data.current_window;
    if (force || data.refresh_on_draw) {
        graphics_clear_screen(CANVAS_UI);
        tooltip_invalidate();
        w->draw_background();
        data.refresh_on_draw = 0;
        data.refresh_immediate = 0;
    }
    w->draw_foreground();

    const mouse *m = mouse_get();
    const hotkeys *h = hotkey_state();
    w->handle_input(m, h);
    tooltip_handle(m, w->get_tooltip);
    warning_draw();
    update_input_after();
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
