#include "requests.h"

#include "core/image_group_editor.h"
#include "game/resource.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/editor.h"
#include "scenario/property.h"
#include "window/editor/attributes.h"
#include "window/editor/edit_request.h"
#include "window/editor/map.h"

static void button_request(int id, int param2);

static generic_button buttons[] = {
    {20, 42, 290, 25, button_request, button_none, 0, 0},
    {20, 72, 290, 25, button_request, button_none, 1, 0},
    {20, 102, 290, 25, button_request, button_none, 2, 0},
    {20, 132, 290, 25, button_request, button_none, 3, 0},
    {20, 162, 290, 25, button_request, button_none, 4, 0},
    {20, 192, 290, 25, button_request, button_none, 5, 0},
    {20, 222, 290, 25, button_request, button_none, 6, 0},
    {20, 252, 290, 25, button_request, button_none, 7, 0},
    {20, 282, 290, 25, button_request, button_none, 8, 0},
    {20, 312, 290, 25, button_request, button_none, 9, 0},
    {320, 42, 290, 25, button_request, button_none, 10, 0},
    {320, 72, 290, 25, button_request, button_none, 11, 0},
    {320, 102, 290, 25, button_request, button_none, 12, 0},
    {320, 132, 290, 25, button_request, button_none, 13, 0},
    {320, 162, 290, 25, button_request, button_none, 14, 0},
    {320, 192, 290, 25, button_request, button_none, 15, 0},
    {320, 222, 290, 25, button_request, button_none, 16, 0},
    {320, 252, 290, 25, button_request, button_none, 17, 0},
    {320, 282, 290, 25, button_request, button_none, 18, 0},
    {320, 312, 290, 25, button_request, button_none, 19, 0},
};

static int focus_button_id;

static void draw_background(void)
{
    window_editor_map_draw_all();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(0, 0, 40, 30);
    lang_text_draw(44, 14, 20, 12, FONT_LARGE_BLACK);
    lang_text_draw_centered(13, 3, 0, 456, 640, FONT_NORMAL_BLACK);
    lang_text_draw_multiline(152, 1, 32, 376, 576, FONT_NORMAL_BLACK);

    for (int i = 0; i < 20; i++) {
        int x, y;
        if (i < 10) {
            x = 20;
            y = 42 + 30 * i;
        } else {
            x = 320;
            y = 42 + 30 * (i - 10);
        }
        button_border_draw(x, y, 290, 25, focus_button_id == i + 1);
        editor_request request;
        scenario_editor_request_get(i, &request);
        if (request.resource) {
            text_draw_number(request.year, '+', " ", x + 20, y + 6, FONT_NORMAL_BLACK, 0);
            lang_text_draw_year(scenario_property_start_year() + request.year, x + 80, y + 6, FONT_NORMAL_BLACK);
            int width = text_draw_number(request.amount, '@', " ", x + 180, y + 6, FONT_NORMAL_BLACK, 0);
            image_draw(resource_get_data(request.resource)->image.editor.icon, x + 190 + width, y + 3,
                COLOR_MASK_NONE, SCALE_NONE);
        } else {
            lang_text_draw_centered(44, 23, x, y + 6, 290, FONT_NORMAL_BLACK);
        }
    }

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, buttons, 20, &focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_editor_attributes_show();
    }
}

static void button_request(int id, int param2)
{
    window_editor_edit_request_show(id);
}

void window_editor_requests_show(void)
{
    window_type window = {
        WINDOW_EDITOR_REQUESTS,
        draw_background,
        draw_foreground,
        handle_input
    };
    window_show(&window);
}
