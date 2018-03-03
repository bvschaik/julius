#include "window.h"

#include "graphics/warning.h"
#include "input/cursor.h"
#include "window/city.h"

#define MAX_QUEUE 3

static window_type window_queue[MAX_QUEUE];
static int queue_index = 0;
static window_type *current_window = 0;
static int refresh_requested;

static void noop()
{
}

static void increase_queue_index()
{
    queue_index++;
    if (queue_index >= MAX_QUEUE) {
        queue_index = 0;
    }
}

static void decrease_queue_index()
{
    queue_index--;
    if (queue_index < 0) {
        queue_index = MAX_QUEUE - 1;
    }
}

void window_invalidate()
{
    refresh_requested = 1;
}

int window_is(window_id id)
{
    return current_window->id == id;
}

window_id window_get_id()
{
    return current_window->id;
}

void window_show(const window_type *window)
{
    increase_queue_index();
    window_queue[queue_index] = *window;
    current_window = &window_queue[queue_index];
    if (!current_window->draw_background) {
        current_window->draw_background = noop;
    }
    if (!current_window->draw_foreground) {
        current_window->draw_foreground = noop;
    }
    if (!current_window->handle_mouse) {
        current_window->handle_mouse = noop;
    }
    window_invalidate();
}

void window_go_back()
{
    decrease_queue_index();
    current_window = &window_queue[queue_index];
    window_invalidate();
}

static void update_mouse_before()
{
    mouse_determine_button_state();
}

static void update_mouse_after()
{
    mouse_set_scroll(SCROLL_NONE);
    input_cursor_update(current_window->id);
}

void window_draw(int force)
{
    update_mouse_before();
    if (force || refresh_requested) {
        current_window->draw_background();
        refresh_requested = 0;
    }
    current_window->draw_foreground();

    const mouse *m = mouse_get();
    current_window->handle_mouse(m);
    tooltip_handle(m, current_window->get_tooltip);
    warning_draw();
    update_mouse_after();
}
