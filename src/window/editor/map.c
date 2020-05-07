#include "map.h"

#include "city/view.h"
#include "core/config.h"
#include "editor/editor.h"
#include "editor/tool.h"
#include "game/game.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "widget/map_editor.h"
#include "widget/sidebar_editor.h"
#include "widget/top_menu_editor.h"
#include "window/file_dialog.h"
#include "window/popup_dialog.h"
#include "window/editor/attributes.h"

static int city_view_dirty;

static void draw_background(void)
{
    graphics_clear_screens();
    widget_sidebar_editor_draw_background();
    widget_top_menu_editor_draw();
}

static void draw_cancel_construction(void)
{
    if (!mouse_get()->is_touch || !editor_tool_is_active()) {
        return;
    }
    int x, y, width, height;
    city_view_get_unscaled_viewport(&x, &y, &width, &height);
    width -= 4 * 16;
    inner_panel_draw(width - 4, 40, 3, 2);
    image_draw(image_group(GROUP_OK_CANCEL_SCROLL_BUTTONS) + 4, width, 44);
    city_view_dirty = 1;
}

static void clear_city_view(void)
{
    if (config_get(CONFIG_UI_ZOOM) && city_view_dirty) {
        graphics_clear_city_viewport();
    }
    city_view_dirty = 0;
}

static void draw_foreground(void)
{
    clear_city_view();
    widget_sidebar_editor_draw_foreground();
    widget_map_editor_draw();
    if (window_is(WINDOW_EDITOR_MAP)) {
        draw_cancel_construction();
    }
}

static void handle_hotkeys(const hotkeys *h)
{
    if (h->load_file) {
        window_file_dialog_show(FILE_TYPE_SCENARIO, FILE_DIALOG_LOAD);
    }
    if (h->save_file) {
        window_file_dialog_show(FILE_TYPE_SCENARIO, FILE_DIALOG_SAVE);
    }
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    handle_hotkeys(h);
    if (widget_top_menu_editor_handle_input(m, h)) {
        return;
    }
    if (widget_sidebar_editor_handle_mouse(m)) {
        return;
    }
    widget_map_editor_handle_input(m, h);
}

void window_editor_map_draw_all(void)
{
    draw_background();
    draw_foreground();
}

void window_editor_map_draw_panels(void)
{
    draw_background();
}

void window_editor_map_draw(void)
{
    widget_map_editor_draw();
}

void window_editor_map_show(void)
{
    window_type window = {
        WINDOW_EDITOR_MAP,
        draw_background,
        draw_foreground,
        handle_input
    };
    window_show(&window);
}
