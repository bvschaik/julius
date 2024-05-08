#include "tooltip.h"

#include "city/labor.h"
#include "city/ratings.h"
#include "city/view.h"
#include "core/calc.h"
#include "core/lang.h"
#include "core/string.h"
#include "core/time.h"
#include "game/settings.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/renderer.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "map/grid.h"
#include "map/property.h"
#include "scenario/criteria.h"
#include "scenario/property.h"
#include "translation/translation.h"
#include "window/advisors.h"

#include <stdlib.h>

#define COMPOSED_TOOLTIP_TEXT_MAX 1000

static const int DEFAULT_TEXT_GROUP = 68;
static const time_millis TOOLTIP_FADE_IN_MILLIS = 75;

typedef struct senate_data_t {
    int active;
    int unemployment_percentage;
    int workers_unemployed;
    int workers_needed;
    int rating_culture;
    int criteria_culture;
    int rating_prosperity;
    int criteria_prosperity;
    int rating_peace;
    int criteria_peace;
    int rating_favor;
    int criteria_favor;
} senate_data_t;

static time_millis last_update;
static uint8_t composed_tooltip_text[COMPOSED_TOOLTIP_TEXT_MAX];
static uint8_t *last_tooltip_text;
static senate_data_t senate_data;

static struct {
    int active;
    int x;
    int y;
} tile_data;

static void reset_timer(void)
{
    last_update = time_get_millis();
}

static int tooltip_is_visible(tooltip_context *c)
{
    if (c->type == TOOLTIP_NONE) {
        graphics_renderer()->set_tooltip_opacity(0);
        reset_timer();
        return 0;
    }
    if (!c->high_priority && setting_tooltips() != TOOLTIPS_FULL) {
        graphics_renderer()->set_tooltip_opacity(0);
        reset_timer();
        return 0;
    }
    if (time_get_millis() - last_update < TOOLTIP_FADE_IN_MILLIS) { // fade in tooltip
        int opacity = calc_percentage(time_get_millis() - last_update, TOOLTIP_FADE_IN_MILLIS);
        graphics_renderer()->set_tooltip_opacity(calc_bound(opacity, 0, 100));
        return opacity > 0;
    }
    graphics_renderer()->set_tooltip_opacity(100);
    return 1;
}

static void reset_tooltip(tooltip_context *c)
{
    if (c->type != TOOLTIP_NONE) {
        c->type = TOOLTIP_NONE;
    }
}

static const uint8_t *get_tooltip_text(const tooltip_context *c)
{
    const uint8_t* text;
    if (c->precomposed_text) {
        return c->precomposed_text;
    }
    if (c->translation_key) {
        text = translation_for(c->translation_key);
    } else {
        text = lang_get_string(c->text_group, c->text_id);
    }
    if (c->has_numeric_prefix) {
        int offset = string_from_int(composed_tooltip_text, c->numeric_prefix, 0);
        string_copy(text, &composed_tooltip_text[offset], COMPOSED_TOOLTIP_TEXT_MAX - offset);
        text = composed_tooltip_text;
    } else if (c->num_extra_texts > 0) {
        string_copy(text, composed_tooltip_text, COMPOSED_TOOLTIP_TEXT_MAX);
        int offset = string_length(composed_tooltip_text);
        int is_comma_separated = c->extra_text_type == TOOLTIP_EXTRA_TEXT_COMMA_SEPARATED;
        if (is_comma_separated) {
            composed_tooltip_text[offset++] = ':';
            composed_tooltip_text[offset++] = '\n';
        } else {
            composed_tooltip_text[offset++] = ' ';
        }
        for (int i = 0; i < c->num_extra_texts; i++) {
            if (i) {
                if (is_comma_separated) {
                    composed_tooltip_text[offset++] = ',';
                }
                composed_tooltip_text[offset++] = ' ';
            }
            const uint8_t *extra_value = lang_get_string(c->extra_text_groups[i], c->extra_text_ids[i]);
            string_copy(extra_value, &composed_tooltip_text[offset], COMPOSED_TOOLTIP_TEXT_MAX - offset);
            offset += string_length(extra_value);
        }
        text = composed_tooltip_text;
    }
    return text;
}

static void save_tooltip_text(const uint8_t *text)
{
    free(last_tooltip_text);
    last_tooltip_text = 0;

    if (!text || !*text) {
        return;
    }
    int length = string_length(text);
    last_tooltip_text = malloc(length + 1);
    if (!last_tooltip_text) {
        return;
    }
    string_copy(text, last_tooltip_text, length);
    last_tooltip_text[length] = '\0';
}

static void draw_button_tooltip(tooltip_context *c)
{
    const uint8_t *text = get_tooltip_text(c);
    int width = 200;
    int largest_width;
    int lines = text_measure_multiline(text, width - 16, FONT_SMALL_PLAIN, &largest_width);
    if (lines > 2) {
        width = 300;
        lines = text_measure_multiline(text, width - 16, FONT_SMALL_PLAIN, &largest_width);
    }
    int height = 16 * lines + 10;
    width = largest_width + 16;

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
    if (x + width > screen_width() - 20) {
        x = c->mouse_x - width - 20;
    }
    if (x < 20) {
        x = 20;
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

    if (y + height > screen_height() - 1) {
        y = screen_height() - 1 - height;
    }
    if (y < 20) {
        y = 20;
    }
    if (height > screen_height() - 20) {
        height = screen_height() - 20;
    }

    graphics_renderer()->set_tooltip_position(x, y);

    senate_data.active = 0;
    tile_data.active = 0;

    if (last_tooltip_text && graphics_renderer()->has_tooltip() && string_equals(text, last_tooltip_text)) {
        return;
    }

    if (!graphics_renderer()->start_tooltip_creation(width, height)) {
        save_tooltip_text(0);
        return;
    }

    graphics_draw_rect(0, 0, width, height, COLOR_BLACK);
    graphics_fill_rect(1, 1, width - 2, height - 2, COLOR_WHITE);
    text_draw_multiline(text, 8, 8, width - 15, 0, FONT_SMALL_PLAIN, COLOR_TOOLTIP);

    graphics_renderer()->finish_tooltip_creation();
}

static void draw_overlay_tooltip(tooltip_context *c)
{
    const uint8_t *text = get_tooltip_text(c);
    int width = 200;
    int largest_width;
    int lines = text_measure_multiline(text, width - 16, FONT_SMALL_PLAIN, &largest_width);
    if (lines > 2) {
        width = 300;
        lines = text_measure_multiline(text, width - 16, FONT_SMALL_PLAIN, &largest_width);
    }
    int height = 16 * lines + 10;
    width = largest_width + 16;

    int x, y;
    if (c->mouse_x < width + 20) {
        x = c->mouse_x + 20;
    } else {
        x = c->mouse_x - width - 20;
    }
    if (c->mouse_y < 200) {
        y = c->mouse_y + 50;
    } else if (c->mouse_y + height - 72 > screen_height()) {
        y = screen_height() - height;
    } else {
        y = c->mouse_y - 72;
    }

    graphics_renderer()->set_tooltip_position(x, y);

    senate_data.active = 0;
    tile_data.active = 0;

    if (last_tooltip_text && graphics_renderer()->has_tooltip() && string_equals(text, last_tooltip_text)) {
        return;
    }

    if (!graphics_renderer()->start_tooltip_creation(width, height)) {
        save_tooltip_text(0);
        return;
    }

    graphics_draw_rect(0, 0, width, height, COLOR_BLACK);
    graphics_fill_rect(1, 1, width - 2, height - 2, COLOR_WHITE);
    text_draw_multiline(text, 8, 8, width - 15, 0, FONT_SMALL_PLAIN, COLOR_TOOLTIP);

    graphics_renderer()->finish_tooltip_creation();

    save_tooltip_text(text);
}

static int update_senate_data(void)
{
    senate_data_t current_senate_data;
    current_senate_data.active = 1;
    current_senate_data.unemployment_percentage = city_labor_unemployment_percentage();
    current_senate_data.workers_unemployed = city_labor_workers_unemployed();
    current_senate_data.workers_needed = city_labor_workers_needed();
    current_senate_data.rating_culture = city_rating_culture();
    current_senate_data.criteria_culture = scenario_criteria_culture();
    current_senate_data.rating_prosperity = city_rating_prosperity();
    current_senate_data.criteria_prosperity = scenario_criteria_prosperity();
    current_senate_data.rating_peace = city_rating_peace();
    current_senate_data.criteria_peace = scenario_criteria_peace();
    current_senate_data.rating_favor = city_rating_favor();
    current_senate_data.criteria_favor = scenario_criteria_favor();

    if (!senate_data.active || senate_data.unemployment_percentage != current_senate_data.unemployment_percentage ||
        senate_data.workers_unemployed != current_senate_data.workers_unemployed ||
        senate_data.rating_culture != current_senate_data.rating_culture ||
        senate_data.criteria_culture != current_senate_data.criteria_culture ||
        senate_data.rating_prosperity != current_senate_data.rating_prosperity ||
        senate_data.criteria_prosperity != current_senate_data.criteria_prosperity ||
        senate_data.rating_peace != current_senate_data.rating_peace ||
        senate_data.criteria_peace != current_senate_data.criteria_peace ||
        senate_data.rating_favor != current_senate_data.rating_favor ||
        senate_data.criteria_favor != current_senate_data.criteria_favor) {
        senate_data = current_senate_data;

        return 1;
    }
    return 0;
}

static void draw_senate_tooltip(tooltip_context *c)
{
    int x, y;
    int width = 220;
    int height = 80;
    if (c->mouse_x < width + 20) {
        x = c->mouse_x + 20;
    } else {
        x = c->mouse_x - width - 20;
    }
    if (c->mouse_y < 200) {
        y = c->mouse_y + 10;
    } else if (c->mouse_y + height - 32 > screen_height()) {
        y = screen_height() - height;
    } else {
        y = c->mouse_y - 32;
    }

    graphics_renderer()->set_tooltip_position(x, y);

    save_tooltip_text(0);
    tile_data.active = 0;

    if (!update_senate_data() && graphics_renderer()->has_tooltip()) {
        return;
    }

    if (!graphics_renderer()->start_tooltip_creation(width, height)) {
        save_tooltip_text(0);
        return;
    }

    graphics_draw_rect(0, 0, width, height, COLOR_BLACK);
    graphics_fill_rect(1, 1, width - 2, height - 2, COLOR_WHITE);

    // unemployment
    lang_text_draw_colored(68, 148, 5, 5, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
    width = text_draw_number(senate_data.unemployment_percentage, '@', "%", 140, 5, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
    text_draw_number(senate_data.workers_unemployed - senate_data.workers_needed, '(', ")",
        140 + width, 5, FONT_SMALL_PLAIN, COLOR_TOOLTIP);

    // ratings
    lang_text_draw_colored(68, 149, 5, 19, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
    text_draw_number(senate_data.rating_culture, '@', " ", 140, 19, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
    if (!scenario_is_open_play() && scenario_criteria_culture_enabled()) {
        text_draw_number(senate_data.criteria_culture, '(', ")", 140 + width, 19, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
    }

    lang_text_draw_colored(68, 150, 5, 33, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
    text_draw_number(senate_data.rating_prosperity, '@', " ", 140, 33, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
    if (!scenario_is_open_play() && scenario_criteria_prosperity_enabled()) {
        text_draw_number(senate_data.criteria_prosperity, '(', ")", 140 + width, 33, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
    }

    lang_text_draw_colored(68, 151, 5, 47, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
    text_draw_number(senate_data.rating_peace, '@', " ", 140, 47, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
    if (!scenario_is_open_play() && scenario_criteria_peace_enabled()) {
        text_draw_number(senate_data.criteria_peace, '(', ")", 140 + width, 47, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
    }

    lang_text_draw_colored(68, 152, 5, 61, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
    text_draw_number(senate_data.rating_favor, '@', " ", 140, 61, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
    if (!scenario_is_open_play() && scenario_criteria_favor_enabled()) {
        text_draw_number(senate_data.criteria_favor, '(', ")", 140 + width, 61, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
    }

    graphics_renderer()->finish_tooltip_creation();
}

static void draw_tile_tooltip(tooltip_context *c)
{
    view_tile view;
    if (city_view_pixels_to_view_tile(c->mouse_x, c->mouse_y, &view)) {
        int grid_offset = city_view_tile_to_grid_offset(&view);
        city_view_set_selected_view_tile(&view);
        int x_tile = map_grid_offset_to_x(grid_offset);
        int y_tile = map_grid_offset_to_y(grid_offset);

        int x, y;
        int width = 60;
        int height = 40;
        if (c->mouse_x < width + 20) {
            x = c->mouse_x + 20;
        } else {
            x = c->mouse_x - width - 20;
        }
        if (c->mouse_y < 40) {
            y = c->mouse_y + 10;
        } else if (c->mouse_y + height - 32 > screen_height()) {
            y = screen_height() - height;
        } else {
            y = c->mouse_y - 32;
        }

        graphics_renderer()->set_tooltip_position(x, y);

        save_tooltip_text(0);
        senate_data.active = 0;

        if (graphics_renderer()->has_tooltip() && tile_data.active && x_tile == tile_data.x && y_tile == tile_data.y) {
            return;
        }

        if (!graphics_renderer()->start_tooltip_creation(width, height)) {
            return;
        }

        graphics_draw_rect(0, 0, width, height, COLOR_BLACK);
        graphics_fill_rect(1, 1, width - 2, height - 2, COLOR_WHITE);
        text_draw_label_and_number(string_from_ascii("x: "), x_tile, " ", 2, 5, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
        text_draw_label_and_number(string_from_ascii("y: "), y_tile, " ", 2, 19, FONT_SMALL_PLAIN, COLOR_TOOLTIP);

        graphics_renderer()->finish_tooltip_creation();
    
        save_tooltip_text(0);
        tile_data.active = 1;
        tile_data.x = x_tile;
        tile_data.y = y_tile;
    }
}

static void draw_tooltip(tooltip_context *c)
{
    if (c->type == TOOLTIP_BUTTON) {
        draw_button_tooltip(c);
    } else if (c->type == TOOLTIP_OVERLAY) {
        draw_overlay_tooltip(c);
    } else if(c->type == TOOLTIP_TILES) {
        draw_tile_tooltip(c);
    } else if (c->type == TOOLTIP_SENATE) {
        draw_senate_tooltip(c);
    }
}

void tooltip_handle(const mouse *m, void (*func)(tooltip_context *))
{
    if (m->is_touch && !m->left.is_down) {
        reset_timer();
        return;
    }
    tooltip_context context = {m->x, m->y};
    context.text_group = DEFAULT_TEXT_GROUP;
    if (setting_tooltips() && func) {
        func(&context);
    }
    if (tooltip_is_visible(&context)) {
        draw_tooltip(&context);
        reset_tooltip(&context);
    }
}
