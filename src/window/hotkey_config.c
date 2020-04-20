#include "config.h"

#include "core/hotkey_config.h"
#include "core/image_group.h"
#include "core/string.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/panel.h"
#include "graphics/scrollbar.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "window/hotkey_editor.h"
#include "window/main_menu.h"

#define HOTKEY_HEADER -1

#define NUM_VISIBLE_OPTIONS 14
#define NUM_BOTTOM_BUTTONS 3

static void on_scroll(void);

static void button_hotkey(int row, int is_alternative);
static void button_reset_defaults(int param1, int param2);
static void button_close(int save, int param2);

static scrollbar_type scrollbar = {580, 72, 352, on_scroll};

typedef struct {
    hotkey_action action;
    const char *name;
} hotkey_widget;

static hotkey_widget hotkey_widgets[] = {
    { HOTKEY_HEADER, "Arrow keys" },
    { HOTKEY_ARROW_UP, "Up" },
    { HOTKEY_ARROW_DOWN, "Down" },
    { HOTKEY_ARROW_LEFT, "Left" },
    { HOTKEY_ARROW_RIGHT, "Right" },
    { HOTKEY_HEADER, "Global hotkeys" },
    { HOTKEY_TOGGLE_FULLSCREEN, "Toggle fullscreen" },
    { HOTKEY_CENTER_SCREEN, "Center window" },
    { HOTKEY_RESIZE_TO_640, "Resize window to 640x480" },
    { HOTKEY_RESIZE_TO_800, "Resize window to 800x600" },
    { HOTKEY_RESIZE_TO_1024, "Resize window to 1024x768" },
    { HOTKEY_SAVE_SCREENSHOT, "Save screenshot" },
    { HOTKEY_SAVE_CITY_SCREENSHOT, "Save full city screenshot" },
    { HOTKEY_LOAD_FILE, "Load file" },
    { HOTKEY_SAVE_FILE, "Save file" },
    { HOTKEY_HEADER, "City hotkeys" },
    { HOTKEY_INCREASE_GAME_SPEED, "Increase game speed" },
    { HOTKEY_DECREASE_GAME_SPEED, "Decrease game speed" },
    { HOTKEY_TOGGLE_PAUSE, "Toggle pause" },
    { HOTKEY_CYCLE_LEGION, "Cycle through legions" },
    { HOTKEY_ROTATE_MAP_LEFT, "Rotate map left" },
    { HOTKEY_ROTATE_MAP_RIGHT, "Rotate map right" },
    { HOTKEY_HEADER, "Advisors" },
    { HOTKEY_SHOW_ADVISOR_LABOR, "Labor advisor" },
    { HOTKEY_SHOW_ADVISOR_MILITARY, "Military advisor" },
    { HOTKEY_SHOW_ADVISOR_IMPERIAL, "Imperial advisor" },
    { HOTKEY_SHOW_ADVISOR_RATINGS, "Ratings advisor" },
    { HOTKEY_SHOW_ADVISOR_TRADE, "Trade advisor" },
    { HOTKEY_SHOW_ADVISOR_POPULATION, "Population advisor" },
    { HOTKEY_SHOW_ADVISOR_HEALTH, "Health advisor" },
    { HOTKEY_SHOW_ADVISOR_EDUCATION, "Education advisor" },
    { HOTKEY_SHOW_ADVISOR_ENTERTAINMENT, "Entertainment advisor" },
    { HOTKEY_SHOW_ADVISOR_RELIGION, "Religion advisor" },
    { HOTKEY_SHOW_ADVISOR_FINANCIAL, "Financial advisor" },
    { HOTKEY_SHOW_ADVISOR_CHIEF, "Chief advisor" },
    { HOTKEY_HEADER, "Overlays" },
    { HOTKEY_TOGGLE_OVERLAY, "Toggle current overlay" },
    { HOTKEY_SHOW_OVERLAY_WATER, "Show water overlay" },
    { HOTKEY_SHOW_OVERLAY_FIRE, "Show fire overlay" },
    { HOTKEY_SHOW_OVERLAY_DAMAGE, "Damage overlay" },
    { HOTKEY_SHOW_OVERLAY_CRIME, "Crime overlay" },
    { HOTKEY_SHOW_OVERLAY_PROBLEMS, "Problems overlay" },
    { HOTKEY_HEADER, "City map bookmarks" },
    { HOTKEY_GO_TO_BOOKMARK_1, "Go to bookmark 1" },
    { HOTKEY_GO_TO_BOOKMARK_2, "Go to bookmark 2" },
    { HOTKEY_GO_TO_BOOKMARK_3, "Go to bookmark 3" },
    { HOTKEY_GO_TO_BOOKMARK_4, "Go to bookmark 4" },
    { HOTKEY_SET_BOOKMARK_1, "Set bookmark 1" },
    { HOTKEY_SET_BOOKMARK_2, "Set bookmark 2" },
    { HOTKEY_SET_BOOKMARK_3, "Set bookmark 3" },
    { HOTKEY_SET_BOOKMARK_4, "Set bookmark 4" },
    { HOTKEY_HEADER, "Editor" },
    { HOTKEY_EDITOR_TOGGLE_BATTLE_INFO, "Toggle battle info" },
};

#define HOTKEY_X_OFFSET_1 290
#define HOTKEY_X_OFFSET_2 430
#define HOTKEY_BTN_WIDTH 140
#define HOTKEY_BTN_HEIGHT 22

static generic_button hotkey_buttons[] = {
    { HOTKEY_X_OFFSET_1, 80 + 24 * 0, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 0, 1 },
    { HOTKEY_X_OFFSET_2, 80 + 24 * 0, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 0, 0 },
    { HOTKEY_X_OFFSET_1, 80 + 24 * 1, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 1, 0 },
    { HOTKEY_X_OFFSET_2, 80 + 24 * 1, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 1, 1 },
    { HOTKEY_X_OFFSET_1, 80 + 24 * 2, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 2, 0 },
    { HOTKEY_X_OFFSET_2, 80 + 24 * 2, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 2, 1 },
    { HOTKEY_X_OFFSET_1, 80 + 24 * 3, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 3, 0 },
    { HOTKEY_X_OFFSET_2, 80 + 24 * 3, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 3, 1 },
    { HOTKEY_X_OFFSET_1, 80 + 24 * 4, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 4, 0 },
    { HOTKEY_X_OFFSET_2, 80 + 24 * 4, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 4, 1 },
    { HOTKEY_X_OFFSET_1, 80 + 24 * 5, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 5, 0 },
    { HOTKEY_X_OFFSET_2, 80 + 24 * 5, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 5, 1 },
    { HOTKEY_X_OFFSET_1, 80 + 24 * 6, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 6, 0 },
    { HOTKEY_X_OFFSET_2, 80 + 24 * 6, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 6, 1 },
    { HOTKEY_X_OFFSET_1, 80 + 24 * 7, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 7, 0 },
    { HOTKEY_X_OFFSET_2, 80 + 24 * 7, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 7, 1 },
    { HOTKEY_X_OFFSET_1, 80 + 24 * 8, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 8, 0 },
    { HOTKEY_X_OFFSET_2, 80 + 24 * 8, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 8, 1 },
    { HOTKEY_X_OFFSET_1, 80 + 24 * 9, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 9, 0 },
    { HOTKEY_X_OFFSET_2, 80 + 24 * 9, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 9, 1 },
    { HOTKEY_X_OFFSET_1, 80 + 24 * 10, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 10, 0 },
    { HOTKEY_X_OFFSET_2, 80 + 24 * 10, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 10, 1 },
    { HOTKEY_X_OFFSET_1, 80 + 24 * 11, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 11, 0 },
    { HOTKEY_X_OFFSET_2, 80 + 24 * 11, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 11, 1 },
    { HOTKEY_X_OFFSET_1, 80 + 24 * 12, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 12, 0 },
    { HOTKEY_X_OFFSET_2, 80 + 24 * 12, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 12, 1 },
    { HOTKEY_X_OFFSET_1, 80 + 24 * 13, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 13, 0 },
    { HOTKEY_X_OFFSET_2, 80 + 24 * 13, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 13, 1 },
};

static generic_button bottom_buttons[] = {
    { 250, 430, 150, 30, button_reset_defaults, button_none },
    { 410, 430, 100, 30, button_close, button_none, 0 },
    { 520, 430, 100, 30, button_close, button_none, 1 },
};

static const char *bottom_button_texts[] = {
    "Reset defaults",
    "Cancel",
    "OK"
};

static struct {
    int focus_button;
    int bottom_focus_button;
    hotkey_mapping mappings[HOTKEY_MAX_ITEMS][2];
} data;

static void init(void)
{
    scrollbar_init(&scrollbar, 0, sizeof(hotkey_widgets) / sizeof(hotkey_widget) - NUM_VISIBLE_OPTIONS);

    for (int i = 0; i < HOTKEY_MAX_ITEMS; i++) {
        hotkey_mapping empty = {KEY_NONE, KEY_MOD_NONE, i};

        const hotkey_mapping *mapping = hotkey_for_action(i, 0);
        data.mappings[i][0] = mapping ? *mapping : empty;

        mapping = hotkey_for_action(i, 1);
        data.mappings[i][1] = mapping ? *mapping : empty;
    }
}

static void draw_background(void)
{
    graphics_clear_screen();

    image_draw_fullscreen_background(image_group(GROUP_CONFIG));

    graphics_in_dialog();
    outer_panel_draw(0, 0, 40, 30);

    text_draw_centered(string_from_ascii("Julius hotkey configuration"), 16, 16, 608, FONT_LARGE_BLACK, 0);

    text_draw_centered(string_from_ascii("Hotkey"), HOTKEY_X_OFFSET_1, 55, HOTKEY_BTN_WIDTH, FONT_NORMAL_BLACK, 0);
    text_draw_centered(string_from_ascii("Alternative"), HOTKEY_X_OFFSET_2, 55, HOTKEY_BTN_WIDTH, FONT_NORMAL_BLACK, 0);

    inner_panel_draw(20, 72, 35, 22);
    int y_base = 80;
    for (int i = 0; i < NUM_VISIBLE_OPTIONS; i++) {
        hotkey_widget *widget = &hotkey_widgets[i + scrollbar.scroll_position];
        int text_offset = y_base + 6 + 24 * i;
        if (widget->action == HOTKEY_HEADER) {
            text_draw(string_from_ascii(widget->name), 32, text_offset, FONT_NORMAL_WHITE, 0);
        } else {
            text_draw(string_from_ascii(widget->name), 32, text_offset, FONT_NORMAL_GREEN, 0);

            const hotkey_mapping *mapping1 = &data.mappings[widget->action][0];
            if (mapping1->key) {
                const uint8_t *keyname = key_combination_display_name(mapping1->key, mapping1->modifiers);
                graphics_set_clip_rectangle(HOTKEY_X_OFFSET_1, text_offset, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT);
                text_draw_centered(keyname, HOTKEY_X_OFFSET_1 + 3, text_offset, HOTKEY_BTN_WIDTH - 6, FONT_NORMAL_WHITE, 0);
                graphics_reset_clip_rectangle();
            }

            const hotkey_mapping *mapping2 = &data.mappings[widget->action][1];
            if (mapping2->key) {
                graphics_set_clip_rectangle(HOTKEY_X_OFFSET_2, text_offset, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT);
                const uint8_t *keyname = key_combination_display_name(mapping2->key, mapping2->modifiers);
                text_draw_centered(keyname, HOTKEY_X_OFFSET_2 + 3, text_offset, HOTKEY_BTN_WIDTH - 6, FONT_NORMAL_WHITE, 0);
                graphics_reset_clip_rectangle();
            }
        }
    }

    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        text_draw_centered(string_from_ascii(bottom_button_texts[i]), bottom_buttons[i].x, bottom_buttons[i].y + 9, bottom_buttons[i].width, FONT_NORMAL_BLACK, 0);
    }

    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    scrollbar_draw(&scrollbar);

    for (int i = 0; i < NUM_VISIBLE_OPTIONS; i++) {
        hotkey_widget *widget = &hotkey_widgets[i + scrollbar.scroll_position];
        if (widget->action != HOTKEY_HEADER) {
            generic_button *btn = &hotkey_buttons[2 * i];
            button_border_draw(btn->x, btn->y, btn->width, btn->height, data.focus_button == 1 + 2 * i);
            btn++;
            button_border_draw(btn->x, btn->y, btn->width, btn->height, data.focus_button == 2 + 2 * i);
        }
    }

    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        button_border_draw(bottom_buttons[i].x, bottom_buttons[i].y, bottom_buttons[i].width, bottom_buttons[i].height, data.bottom_focus_button == i + 1);
    }
    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (scrollbar_handle_mouse(&scrollbar, m_dialog)) {
        return;
    }

    int handled = 0;
    handled |= generic_buttons_handle_mouse(m_dialog, 0, 0, hotkey_buttons, NUM_VISIBLE_OPTIONS * 2, &data.focus_button);
    handled |= generic_buttons_handle_mouse(m_dialog, 0, 0, bottom_buttons, NUM_BOTTOM_BUTTONS, &data.bottom_focus_button);
    if (!handled && (m->right.went_up || h->escape_pressed)) {
        window_main_menu_show(0);
    }
}

static void set_hotkey(hotkey_action action, int index, key_type key, key_modifier_type modifiers)
{
    data.mappings[action][index].key = key;
    data.mappings[action][index].modifiers = modifiers;
}

static void button_hotkey(int row, int is_alternative)
{
    hotkey_widget *widget = &hotkey_widgets[row + scrollbar.scroll_position];
    if (widget->action == HOTKEY_HEADER) {
        return;
    }
    window_hotkey_editor_show(widget->action, is_alternative, set_hotkey);
}

static void button_reset_defaults(int param1, int param2)
{
    for (int action = 0; action < HOTKEY_MAX_ITEMS; action++) {
        for (int index = 0; index < 2; index++) {
            data.mappings[action][index] = *hotkey_default_for_action(action, index);
        }
    }
    window_invalidate();
}

static void on_scroll(void)
{
    window_invalidate();
}

static void button_close(int save, int param2)
{
    if (!save) {
        window_go_back();
        return;
    }
    hotkey_config_clear();
    for (int action = 0; action < HOTKEY_MAX_ITEMS; action++) {
        for (int index = 0; index < 2; index++) {
            if (data.mappings[action][index].key != KEY_NONE) {
                hotkey_config_add_mapping(&data.mappings[action][index]);
            }
        }
    }
    hotkey_config_save();
    window_go_back();
}

void window_hotkey_config_show(void)
{
    window_type window = {
        WINDOW_HOTKEY_CONFIG,
        draw_background,
        draw_foreground,
        handle_input
    };
    init();
    window_show(&window);
}
