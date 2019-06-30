#include "map.h"

#include "editor/editor.h"
#include "game/game.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "widget/city_editor.h"
#include "widget/sidebar_editor.h"
#include "widget/top_menu_editor.h"
#include "window/popup_dialog.h"
#include "window/editor/attributes.h"

static void draw_background(void)
{
    graphics_clear_screen();
    widget_sidebar_editor_draw_background();
    widget_top_menu_editor_draw();
}

static void draw_foreground(void)
{
    widget_sidebar_editor_draw_foreground();
    widget_city_editor_draw();
}

static void handle_mouse(const mouse *m)
{
    if (widget_top_menu_editor_handle_mouse(m)) {
        return;
    }
    if (widget_sidebar_editor_handle_mouse(m)) {
        return;
    }
    widget_city_editor_handle_mouse(m);
}

void window_editor_map_draw_all(void)
{
    draw_background();
    draw_foreground();
}

void window_editor_map_show(void)
{
    window_type window = {
        WINDOW_EDITOR_MAP,
        draw_background,
        draw_foreground,
        handle_mouse
    };
    window_show(&window);
}
