#include "text_input.h"

#include "core/array.h"
#include "core/image_group.h"
#include "core/log.h"
#include "core/string.h"
#include "graphics/graphics.h"
#include "graphics/image_button.h"
#include "graphics/panel.h"
#include "graphics/lang_text.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "widget/input_box.h"

#define MAX_VARIABLE_NAME_SIZE 50

static void input_number(int number);
static void input_accept(void);
static void button_cancel(int param1, int param2);

static char text[MAX_VARIABLE_NAME_SIZE] = "";
static input_box text_input = { 160, 208, 20, 2, FONT_NORMAL_WHITE, 1, (uint8_t *) text, MAX_VARIABLE_NAME_SIZE };

static struct {
    int x;
    int y;
    void (*callback)(char *);

    int focus_button_id;
} data;

static void init(int x, int y, void (*callback)(char *))
{
    data.x = x;
    data.y = y;
    text_input.x = data.x + 16;
    text_input.y = data.y + 16;
    
    data.callback = callback;
    data.focus_button_id = 0;
    memset(text, 0, MAX_VARIABLE_NAME_SIZE);

    input_box_start(&text_input);
}

static void close(void)
{
    input_box_stop();
    window_go_back();
}

static void draw_foreground(void)
{
    outer_panel_draw(data.x, data.y, 24, 8);

    graphics_fill_rect(text_input.x, text_input.y, text_input.width_blocks * 16, text_input.height_blocks * 16, COLOR_BLACK);

    input_box_draw(&text_input);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (input_box_handle_mouse(m_dialog, &text_input)) {
        return;
    }
    if (input_box_is_accepted()) {
        input_accept();
        return;
    }
    if (input_go_back_requested(m, h)) {
        button_cancel(0, 0);
    }
}

static void button_cancel(int param1, int param2)
{
    close();
}

static void input_accept(void)
{
    close();
    data.callback(text);
}

void window_text_input_show(int x, int y, void (*callback)(char *))
{
    window_type window = {
        WINDOW_NUMERIC_INPUT,
        window_draw_underlying_window,
        draw_foreground,
        handle_input,
    };
    init(x, y, callback);
    window_show(&window);
}
