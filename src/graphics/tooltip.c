#include "tooltip.h"

#include "city/labor.h"
#include "city/ratings.h"
#include "core/lang.h"
#include "core/string.h"
#include "core/time.h"
#include "game/settings.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/rich_text.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "window/advisors.h"

#include <stdlib.h>

static const int DEFAULT_TEXT_GROUP = 68;
static const time_millis TOOLTIP_DELAY_MILLIS = 150;

static time_millis last_update = 0;
static uint8_t overlay_string[1000];
static struct {
    int is_active;
    int x;
    int y;
    int width;
    int height;
    int buffer_size;
    color_t *buffer;
} button_tooltip_info;

static void reset_timer(void)
{
    last_update = time_get_millis();
}

static int should_draw_tooltip(tooltip_context* c)
{
    if (c->type == TOOLTIP_NONE) {
        reset_timer();
        return 0;
    }
    if (!c->high_priority && setting_tooltips() != TOOLTIPS_FULL) {
        reset_timer();
        return 0;
    }
    if (time_get_millis() - last_update < TOOLTIP_DELAY_MILLIS) { // delay drawing tooltip
        return 0;
    }
    return 1;
}

static void reset_tooltip(tooltip_context *c)
{
    if (c->type != TOOLTIP_NONE) {
        c->type = TOOLTIP_NONE;
    }
}

static void restore_button_tooltip_window_buffer(void)
{
    if (button_tooltip_info.is_active) {
        graphics_draw_from_buffer(button_tooltip_info.x, button_tooltip_info.y, button_tooltip_info.width, button_tooltip_info.height, button_tooltip_info.buffer);
    }
}

static void button_tooltip_buffer_window(int x, int y, int width, int height)
{
    if (button_tooltip_info.is_active &&
        x == button_tooltip_info.x && y == button_tooltip_info.y &&
        width == button_tooltip_info.width && height == button_tooltip_info.height) {
        return;
    }
    restore_button_tooltip_window_buffer();
    button_tooltip_info.is_active = 1;
    button_tooltip_info.x = x;
    button_tooltip_info.y = y;
    button_tooltip_info.width = width;
    button_tooltip_info.height = height;
    int buffer_size = width * height;
    if (buffer_size > button_tooltip_info.buffer_size) {
        button_tooltip_info.buffer_size = buffer_size;
        free(button_tooltip_info.buffer);
        button_tooltip_info.buffer = (color_t *)malloc(buffer_size * sizeof(color_t));
    }
    graphics_save_to_buffer(x, y, width, height, button_tooltip_info.buffer);
}

static void draw_button_tooltip(tooltip_context *c)
{
    const uint8_t *text = lang_get_string(c->text_group, c->text_id);
    rich_text_set_fonts(FONT_SMALL_PLAIN, FONT_SMALL_PLAIN);

    int width = 200;
    int lines = rich_text_draw(text, 0, 0, width - 5, 30, 1);
    if (lines > 2) {
        width = 300;
        lines = rich_text_draw(text, 0, 0, width - 5, 30, 1);
    }
    int height = 16 * lines + 10;

    int x, y;
    if (c->mouse_x < screen_dialog_offset_x() + width + 100) {
        if (window_is(WINDOW_ADVISORS)) {
            x = c->mouse_x + 50;
        } else {
            x = c->mouse_x + 20;
        }
    } else {
        x = c->mouse_x - width - 20;
    }
    
    switch (window_get_id()) {
        case WINDOW_ADVISORS:
            if (c->mouse_y < screen_dialog_offset_y() + 432) {
                y = c->mouse_y;
                switch (window_advisors_get_advisor()) {
                    case ADVISOR_LABOR: y -= 74; break;
                    case ADVISOR_TRADE: y -= 54; break;
                    case ADVISOR_POPULATION: y -= 58; break;
                    default: y -= 64; break;
                }
            } else {
                y = screen_dialog_offset_y() + 432;
            }
            break;
        case WINDOW_TRADE_PRICES: // FIXED used to cause ghosting
            y = c->mouse_y - 42;
            break;
        case WINDOW_DONATE_TO_CITY:
            y = c->mouse_y - 52;
            break;
        case WINDOW_LABOR_PRIORITY:
            x = c->mouse_x - width / 2 - 10;
            if (c->mouse_y < screen_dialog_offset_y() + 200) {
                y = c->mouse_y + 40;
            } else {
                y = c->mouse_y - 72;
            }
            break;
        default:
            if (c->mouse_y < screen_dialog_offset_y() + 200) {
                y = c->mouse_y + 40;
            } else {
                y = c->mouse_y - 62;
            }
            break;
    }

    button_tooltip_buffer_window(x, y, width, height);

    graphics_draw_rect(x, y, width, height, COLOR_BLACK);
    graphics_fill_rect(x + 1, y + 1, width - 2, height - 2, COLOR_WHITE);
    rich_text_draw_colored(text, x + 5, y + 7, width - 5, lines, COLOR_TOOLTIP);
}

static void draw_overlay_tooltip(tooltip_context *c)
{
    const uint8_t *text = lang_get_string(c->text_group, c->text_id);
    if (c->has_numeric_prefix) {
        int offset = string_from_int(overlay_string, c->numeric_prefix, 0);
        string_copy(text, &overlay_string[offset], 1000);
        text = overlay_string;
    }
    rich_text_set_fonts(FONT_SMALL_PLAIN, FONT_SMALL_PLAIN);

    int width = 200;
    int lines = rich_text_draw(text, 0, 0, width - 5, 30, 1);
    if (lines > 2) {
        width = 300;
        lines = rich_text_draw(text, 0, 0, width - 5, 30, 1);
    }
    int height = 16 * lines + 10;

    int x, y;
    if (c->mouse_x < width + 20) {
        x = c->mouse_x + 20;
    } else {
        x = c->mouse_x - width - 20;
    }
    if (c->mouse_y < 200) {
        y = c->mouse_y + 50;
    } else {
        y = c->mouse_y - 72;
    }

    graphics_draw_rect(x, y, width, height, COLOR_BLACK);
    graphics_fill_rect(x + 1, y + 1, width - 2, height - 2, COLOR_WHITE);
    rich_text_draw_colored(text, x + 5, y + 7, width - 5, lines, COLOR_TOOLTIP);
}

static void draw_senate_tooltip(tooltip_context *c)
{
    int x, y;
    int width = 180;
    int height = 80;
    if (c->mouse_x < width + 20) {
        x = c->mouse_x + 20;
    } else {
        x = c->mouse_x - width - 20;
    }
    if (c->mouse_y < 200) {
        y = c->mouse_y + 10;
    } else {
        y = c->mouse_y - 32;
    }
    
    graphics_draw_rect(x, y, width, height, COLOR_BLACK);
    graphics_fill_rect(x + 1, y + 1, width - 2, height - 2, COLOR_WHITE);
    
    // unemployment
    lang_text_draw_colored(68, 148, x + 5, y + 5, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
    text_draw_number_colored(city_labor_unemployment_percentage(), '@', "%",
        x + 140, y + 5, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
    
    // ratings
    lang_text_draw_colored(68, 149, x + 5, y + 19, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
    text_draw_number_colored(city_rating_culture(), '@', " ",
        x + 140, y + 19, FONT_SMALL_PLAIN, COLOR_TOOLTIP);

    lang_text_draw_colored(68, 150, x + 5, y + 33, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
    text_draw_number_colored(city_rating_prosperity(), '@', " ",
        x + 140, y + 33, FONT_SMALL_PLAIN, COLOR_TOOLTIP);

    lang_text_draw_colored(68, 151, x + 5, y + 47, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
    text_draw_number_colored(city_rating_peace(), '@', " ",
        x + 140, y + 47, FONT_SMALL_PLAIN, COLOR_TOOLTIP);

    lang_text_draw_colored(68, 152, x + 5, y + 61, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
    text_draw_number_colored(city_rating_favor(), '@', " ",
        x + 140, y + 61, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
}

static void draw_tooltip(tooltip_context *c)
{
    if (c->type == TOOLTIP_BUTTON) {
        draw_button_tooltip(c);
    } else if (c->type == TOOLTIP_OVERLAY) {
        draw_overlay_tooltip(c);
    } else if (c->type == TOOLTIP_SENATE) {
        draw_senate_tooltip(c);
    }
}

void tooltip_invalidate(void)
{
    button_tooltip_info.is_active = 0;
}

void tooltip_handle(const mouse *m, void (*func)(tooltip_context *))
{
    if (m->is_touch && !m->left.is_down) {
        reset_timer();
        return;
    }
    tooltip_context context = {m->x, m->y, 0, 0, 0, 0, 0, 0};
    context.text_group = DEFAULT_TEXT_GROUP;
    if (setting_tooltips() && func) {
        func(&context);
    }
    if (should_draw_tooltip(&context)) {
        rich_text_save();
        draw_tooltip(&context);
        reset_tooltip(&context);
        rich_text_restore();
    } else {
        restore_button_tooltip_window_buffer();
        button_tooltip_info.is_active = 0;
    }
}
