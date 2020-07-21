#include "population.h"

#include "core/config.h"
#include "city/finance.h"
#include "city/migration.h"
#include "city/population.h"
#include "city/ratings.h"
#include "city/resource.h"
#include "game/time.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/property.h"
#include "translation/translation.h"
#include "window/advisors.h"

#define ADVISOR_HEIGHT 27
#define HOUSING_ADVISOR_ID 19

static void button_graph(int param1, int param2);

static generic_button graph_buttons[] = {
    {503,  61, 104, 55, button_graph, button_none, 0, 0},
    {503, 161, 104, 55, button_graph, button_none, 1, 0},
    {545, 260, 60, 51, button_graph, button_none, 0, 1}
};

static int focus_button_id;

static void get_y_axis(int max_value, int *y_max, int *y_shift)
{
    if (max_value <= 100) {
        *y_max = 100;
        *y_shift = -1;
    } else if (max_value <= 200) {
        *y_max = 200;
        *y_shift = 0;
    } else if (max_value <= 400) {
        *y_max = 400;
        *y_shift = 1;
    } else if (max_value <= 800) {
        *y_max = 800;
        *y_shift = 2;
    } else if (max_value <= 1600) {
        *y_max = 1600;
        *y_shift = 3;
    } else if (max_value <= 3200) {
        *y_max = 3200;
        *y_shift = 4;
    } else if (max_value <= 6400) {
        *y_max = 6400;
        *y_shift = 5;
    } else if (max_value <= 12800) {
        *y_max = 12800;
        *y_shift = 6;
    } else if (max_value <= 25600) {
        *y_max = 25600;
        *y_shift = 7;
    } else {
        *y_max = 51200;
        *y_shift = 8;
    }
}

static void get_min_max_month_year(int max_months, int *start_month, int *start_year, int *end_month, int *end_year)
{
    if (city_population_monthly_count() > max_months) {
        *end_month = game_time_month() - 1;
        *end_year = game_time_year();
        if (*end_month < 0) {
            *end_year -= 1;
        }
        *start_month = 11 - (max_months % 12);
        *start_year = *end_year - max_months / 12;
    } else {
        *start_month = 0;
        *start_year = scenario_property_start_year();
        *end_month = (max_months + *start_month) % 12;
        *end_year = (max_months + *start_month) / 12 + *start_year;
    }
}

static void draw_history_graph(int full_size, int x, int y)
{
    int max_months;
    int month_count = city_population_monthly_count();
    if (month_count <= 20) {
        max_months = 20;
    } else if (month_count <= 40) {
        max_months = 40;
    } else if (month_count <= 100) {
        max_months = 100;
    } else if (month_count <= 200) {
        max_months = 200;
    } else {
        max_months = 400;
    }
    if (!full_size) {
        if (max_months <= 40) {
            max_months = 20;
        } else {
            max_months = 100;
        }
    }
    // determine max value
    int max_value = 0;
    for (int m = 0; m < max_months; m++) {
        int value = city_population_at_month(max_months, m);
        if (value > max_value) {
            max_value = value;
        }
    }
    int y_max, y_shift;
    get_y_axis(max_value, &y_max, &y_shift);
    if (full_size) {
        // y axis
        text_draw_number_centered(y_max, x - 66, y - 3, 60, FONT_SMALL_PLAIN);
        text_draw_number_centered(y_max / 2, x - 66, y + 96, 60, FONT_SMALL_PLAIN);
        text_draw_number_centered(0, x - 66, y + 196, 60, FONT_SMALL_PLAIN);
        // x axis
        int start_month, start_year, end_month, end_year;
        get_min_max_month_year(max_months, &start_month, &start_year, &end_month, &end_year);

        int width = lang_text_draw(25, start_month, x - 20, y + 210, FONT_SMALL_PLAIN);
        lang_text_draw_year(start_year, x + width - 20, y + 210, FONT_SMALL_PLAIN);

        width = lang_text_draw(25, end_month, x + 380, y + 210, FONT_SMALL_PLAIN);
        lang_text_draw_year(end_year, x + width + 380, y + 210, FONT_SMALL_PLAIN);
    }

    if (full_size) {
        graphics_set_clip_rectangle(0, 0, 640, y + 200);
        for (int m = 0; m < max_months; m++) {
            int pop = city_population_at_month(max_months, m);
            int val;
            if (y_shift == -1) {
                val = 2 * pop;
            } else {
                val = pop >> y_shift;
            }
            if (val > 0) {
                switch (max_months) {
                    case 20:
                        image_draw(image_group(GROUP_POPULATION_GRAPH_BAR), x + 20 * m, y + 200 - val);
                        break;
                    case 40:
                        image_draw(image_group(GROUP_POPULATION_GRAPH_BAR) + 1, x + 10 * m, y + 200 - val);
                        break;
                    case 100:
                        image_draw(image_group(GROUP_POPULATION_GRAPH_BAR) + 2, x + 4 * m, y + 200 - val);
                        break;
                    case 200:
                        image_draw(image_group(GROUP_POPULATION_GRAPH_BAR) + 3, x + 2 * m, y + 200 - val);
                        break;
                    default:
                        graphics_draw_vertical_line(x + m, y + 200 - val, y + 199, COLOR_RED);
                        break;
                }
            }
        }
        graphics_reset_clip_rectangle();
    } else {
        y_shift += 2;
        for (int m = 0; m < max_months; m++) {
            int val = city_population_at_month(max_months, m) >> y_shift;
            if (val > 0) {
                if (max_months == 20) {
                    graphics_fill_rect(x + m, y + 50 - val, 4, val + 1, COLOR_RED);
                } else {
                    graphics_draw_vertical_line(x + m, y + 50 - val, y + 50, COLOR_RED);
                }
            }
        }
    }
}

static void draw_census_graph(int full_size, int x, int y)
{
    int max_value = 0;
    for (int i = 0; i < 100; i++) {
        int value = city_population_at_age(i);
        if (value > max_value) {
            max_value = value;
        }
    }
    int y_max, y_shift;
    get_y_axis(max_value, &y_max, &y_shift);
    if (full_size) {
        // y axis
        text_draw_number_centered(y_max, x - 66, y - 3, 60, FONT_SMALL_PLAIN);
        text_draw_number_centered(y_max / 2, x - 66, y + 96, 60, FONT_SMALL_PLAIN);
        text_draw_number_centered(0, x - 66, y + 196, 60, FONT_SMALL_PLAIN);
        // x axis
        for (int i = 0; i <= 10; i++) {
            text_draw_number_centered(i * 10, x + 40 * i - 22, y + 210, 40, FONT_SMALL_PLAIN);
        }
    }

    if (full_size) {
        graphics_set_clip_rectangle(0, 0, 640, y + 200);
        for (int i = 0; i < 100; i++) {
            int pop = city_population_at_age(i);
            int val;
            if (y_shift == -1) {
                val = 2 * pop;
            } else {
                val = pop >> y_shift;
            }
            if (val > 0) {
                image_draw(image_group(GROUP_POPULATION_GRAPH_BAR) + 2, x + 4 * i, y + 200 - val);
            }
        }
        graphics_reset_clip_rectangle();
    } else {
        y_shift += 2;
        for (int i = 0; i < 100; i++) {
            int val = city_population_at_age(i) >> y_shift;
            if (val > 0) {
                graphics_draw_vertical_line(x + i, y + 50 - val, y + 50, COLOR_RED);
            }
        }
    }
}

static void draw_society_graph(int full_size, int x, int y)
{
    int max_value = 0;
    for (int i = 0; i < 20; i++) {
        int value = city_population_at_level(i);
        if (value > max_value) {
            max_value = value;
        }
    }
    int y_max, y_shift;
    get_y_axis(max_value, &y_max, &y_shift);
    if (full_size) {
        // y axis
        text_draw_number_centered(y_max, x - 66, y - 3, 60, FONT_SMALL_PLAIN);
        text_draw_number_centered(y_max / 2, x - 66, y + 96, 60, FONT_SMALL_PLAIN);
        text_draw_number_centered(0, x - 66, y + 196, 60, FONT_SMALL_PLAIN);
        // x axis
        lang_text_draw_centered(55, 9, x - 80, y + 210, 200, FONT_SMALL_PLAIN);
        lang_text_draw_centered(55, 10, x + 280, y + 210, 200, FONT_SMALL_PLAIN);
    }

    if (full_size) {
        graphics_set_clip_rectangle(0, 0, 640, y + 200);
        for (int i = 0; i < 20; i++) {
            int pop = city_population_at_level(i);
            int val;
            if (y_shift == -1) {
                val = 2 * pop;
            } else {
                val = pop >> y_shift;
            }
            if (val > 0) {
                image_draw(image_group(GROUP_POPULATION_GRAPH_BAR), x + 20 * i, y + 200 - val);
            }
        }
        graphics_reset_clip_rectangle();
    } else {
        y_shift += 2;
        for (int i = 0; i < 20; i++) {
            int val = city_population_at_level(i) >> y_shift;
            if (val > 0) {
                graphics_fill_rect(x + 5 * i, y + 50 - val, 4, val + 1, COLOR_RED);
            }
        }
    }
}

static void print_society_info(void) {
    int width;
    int avg_tax_per_house = 0;
    if (calculate_total_housing_buildings() > 0) {
        avg_tax_per_house = city_finance_estimated_tax_income() / calculate_total_housing_buildings();
    }

    // Housing prosperity cap
    width = text_draw(translation_for(TR_ADVISOR_HOUSING_PROSPERITY_RATING), 75, 342, FONT_NORMAL_WHITE, 0);
    text_draw_number(city_ratings_prosperity_max(), '@', " ", 75 + width, 342, FONT_NORMAL_WHITE);

    // Percent patricians
    width = text_draw(translation_for(TR_ADVISOR_PERCENTAGE_IN_VILLAS_PALACES), 75, 360, FONT_NORMAL_WHITE, 0);
    text_draw_percentage(percentage_city_population_in_villas_palaces(), 75 + width, 360, FONT_NORMAL_WHITE);

    // Percent impoverished
    width = text_draw(translation_for(TR_ADVISOR_PERCENTAGE_IN_TENTS_SHACKS), 75, 378, FONT_NORMAL_WHITE, 0);
    text_draw_percentage(percentage_city_population_in_tents_shacks(), 75 + width, 378, FONT_NORMAL_WHITE);

    // Average tax
    width = text_draw(translation_for(TR_ADVISOR_AVERAGE_TAX), 75, 396, FONT_NORMAL_WHITE, 0);
    text_draw_money(avg_tax_per_house, 75 + width, 396, FONT_NORMAL_WHITE);
}

static void print_census_info(void) {

    int width;

    // Average age
    width = text_draw(translation_for(TR_ADVISOR_AVERAGE_AGE), 75, 342, FONT_NORMAL_WHITE, 0);
    text_draw_number(city_population_average_age(), '@', " ", 75 + width, 342, FONT_NORMAL_WHITE);

    // Percent working age
    width = text_draw(translation_for(TR_ADVISOR_PERCENT_IN_WORKFORCE), 75, 360, FONT_NORMAL_WHITE, 0);
    text_draw_percentage(city_population_percent_in_workforce(), 75 + width, 360, FONT_NORMAL_WHITE);

    // Yearly births
    width = text_draw(translation_for(TR_ADVISOR_BIRTHS_LAST_YEAR), 75, 378, FONT_NORMAL_WHITE, 0);
    text_draw_number(city_population_yearly_births(), '@', "", 75 + width, 378, FONT_NORMAL_WHITE);

    // Yearly deaths
    width = text_draw(translation_for(TR_ADVISOR_DEATHS_LAST_YEAR), 75, 396, FONT_NORMAL_WHITE, 0);
    text_draw_number(city_population_yearly_deaths(), '@', "", 75 + width, 396, FONT_NORMAL_WHITE);
}

static void print_history_info(void) {
    int width;

    // food stores
    if (scenario_property_rome_supplies_wheat()) {
        lang_text_draw(55, 11, 75, 342, FONT_NORMAL_WHITE);
    } else {
        width = lang_text_draw_amount(8, 6, city_resource_operating_granaries(), 70, 342, FONT_NORMAL_WHITE);
        if (city_resource_food_supply_months() > 0) {
            width += lang_text_draw(55, 12, 70 + width, 342, FONT_NORMAL_WHITE);
            lang_text_draw_amount(8, 4, city_resource_food_supply_months(), 70 + width, 342, FONT_NORMAL_WHITE);
        } else if (city_resource_food_stored() > city_resource_food_needed() / 2) {
            lang_text_draw(55, 13, 70 + width, 342, FONT_NORMAL_WHITE);
        } else if (city_resource_food_stored() > 0) {
            lang_text_draw(55, 15, 70 + width, 342, FONT_NORMAL_WHITE);
        } else {
            lang_text_draw(55, 14, 70 + width, 342, FONT_NORMAL_WHITE);
        }
    }

    // food types eaten
    width = lang_text_draw(55, 16, 75, 360, FONT_NORMAL_WHITE);
    text_draw_number(city_resource_food_types_available(), '@', " ", 75 + width, 360, FONT_NORMAL_WHITE);

    // immigration
    int newcomers = city_migration_newcomers();
    if (newcomers >= 5) {
        lang_text_draw(55, 24, 75, 378, FONT_NORMAL_WHITE);
        width = text_draw_number(newcomers, '@', " ", 70, 396, FONT_NORMAL_WHITE);
        lang_text_draw(55, 17, 70 + width, 396, FONT_NORMAL_WHITE);
    } else if (city_migration_no_room_for_immigrants()) {
        lang_text_draw(55, 24, 75, 378, FONT_NORMAL_WHITE);
        lang_text_draw(55, 19, 75, 396, FONT_NORMAL_WHITE);
    } else if (city_migration_percentage() < 80) {
        lang_text_draw(55, 25, 75, 378, FONT_NORMAL_WHITE);
        int text_id;
        switch (city_migration_no_immigration_cause()) {
            case NO_IMMIGRATION_LOW_WAGES: text_id = 20; break;
            case NO_IMMIGRATION_NO_JOBS: text_id = 21; break;
            case NO_IMMIGRATION_NO_FOOD: text_id = 22; break;
            case NO_IMMIGRATION_HIGH_TAXES: text_id = 23; break;
            case NO_IMMIGRATION_MANY_TENTS: text_id = 31; break;
            case NO_IMMIGRATION_LOW_MOOD: text_id = 32; break;
            default: text_id = 0; break;
        }
        if (text_id) {
            lang_text_draw(55, text_id, 75, 396, FONT_NORMAL_WHITE);
        }
    } else {
        lang_text_draw(55, 24, 75, 378, FONT_NORMAL_WHITE);
        width = text_draw_number(newcomers, '@', " ", 70, 396, FONT_NORMAL_WHITE);
        if (newcomers == 1) {
            lang_text_draw(55, 18, 70 + width, 396, FONT_NORMAL_WHITE);
        } else {
            lang_text_draw(55, 17, 70 + width, 396, FONT_NORMAL_WHITE);
        }
    }}

static void draw_housing_button(int full_size, int x, int y)
{
    image_draw_isometric_footprint(image_group(29) + 2, x, y, COLOR_MASK_NONE);
    image_draw_isometric_top(image_group(29) + 2, x, y, COLOR_MASK_NONE);
}

static int draw_background(void)
{
    int width;

    outer_panel_draw(0, 0, 40, ADVISOR_HEIGHT);
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 5, 10, 10);

    int graph_order = city_population_graph_order();
    // Title: depends on big graph shown
    if (graph_order < 2) {
        lang_text_draw(55, 0, 60, 12, FONT_LARGE_BLACK);
    } else if (graph_order < 4) {
        lang_text_draw(55, 1, 60, 12, FONT_LARGE_BLACK);
    } else {
        lang_text_draw(55, 2, 60, 12, FONT_LARGE_BLACK);
    }

    image_draw(image_group(GROUP_PANEL_WINDOWS) + 14, 56, 60);

    width = text_draw_number(city_population(), '@', " ", 450, 25, FONT_NORMAL_BLACK);
    text_draw(translation_for(TR_ADVISOR_TOTAL_POPULATION), 450+width, 25, FONT_NORMAL_BLACK, 0);

    int big_text, top_text, bot_text;
    void (*big_graph)(int, int, int);
    void (*top_graph)(int, int, int);
    void (*bot_graph)(int, int, int);
    void (*housing_button)(int, int, int);
    void (*info_panel)();

    switch (graph_order) {
        default:
        case 0:
            big_text = 6;
            top_text = 4;
            bot_text = 5;
            big_graph = draw_history_graph;
            top_graph = draw_census_graph;
            bot_graph = draw_society_graph;
            housing_button = draw_housing_button;
            info_panel = print_history_info;
            break;
        case 1:
            big_text = 6;
            top_text = 5;
            bot_text = 4;
            big_graph = draw_history_graph;
            top_graph = draw_society_graph;
            bot_graph = draw_census_graph;
            housing_button = draw_housing_button;
            info_panel = print_history_info;
            break;
        case 2:
            big_text = 7;
            top_text = 3;
            bot_text = 5;
            big_graph = draw_census_graph;
            top_graph = draw_history_graph;
            bot_graph = draw_society_graph;
            housing_button = draw_housing_button;
            info_panel = print_census_info;
            break;
        case 3:
            big_text = 7;
            top_text = 5;
            bot_text = 3;
            big_graph = draw_census_graph;
            top_graph = draw_society_graph;
            bot_graph = draw_history_graph;
            housing_button = draw_housing_button;
            info_panel = print_census_info;
            break;
        case 4:
            big_text = 8;
            top_text = 3;
            bot_text = 4;
            big_graph = draw_society_graph;
            top_graph = draw_history_graph;
            bot_graph = draw_census_graph;
            housing_button = draw_housing_button;
            info_panel = print_society_info;
            break;
        case 5:
            big_text = 8;
            top_text = 4;
            bot_text = 3;
            big_graph = draw_society_graph;
            top_graph = draw_census_graph;
            bot_graph = draw_history_graph;
            housing_button = draw_housing_button;
            info_panel = print_society_info;
            break;
    }

    text_draw_centered(translation_for(TR_HEADER_HOUSING), 545, 315, 61, FONT_NORMAL_BLACK, 0);
    lang_text_draw_centered(55, big_text, 60, 295, 400, FONT_NORMAL_BLACK);
    lang_text_draw_centered(55, top_text, 504, 130, 100, FONT_NORMAL_BLACK);
    lang_text_draw_centered(55, bot_text, 504, 230, 100, FONT_NORMAL_BLACK);

    big_graph(1, 64, 64);
    top_graph(0, 505, 63);
    bot_graph(0, 505, 163);
    housing_button(0, 545, 275);

    // info panel
    inner_panel_draw(48, 336, 34, 5);
    int image_id = image_group(GROUP_BULLET);
    image_draw(image_id, 56, 344);
    image_draw(image_id, 56, 362);
    image_draw(image_id, 56, 380);
    image_draw(image_id, 56, 398);

    info_panel();

    return ADVISOR_HEIGHT;
}



static void draw_foreground(void)
{
    if (focus_button_id == 0) {
        button_border_draw(501, 60, 106, 57, 0);
        button_border_draw(501, 160, 106, 57, 0);
        button_border_draw(545, 260, 60, 51, 0);
    } else if (focus_button_id == 1) {
        button_border_draw(501, 60, 106, 57, 1);
        button_border_draw(501, 160, 106, 57, 0);
        button_border_draw(545, 260, 60, 51, 0);
    } else if (focus_button_id == 2) {
        button_border_draw(501, 60, 106, 57, 0);
        button_border_draw(501, 160, 106, 57, 1);
        button_border_draw(545, 260, 60, 51, 0);
    } else if (focus_button_id == 3) {
        button_border_draw(501, 60, 106, 57, 0);
        button_border_draw(501, 160, 106, 57, 0);
        button_border_draw(545, 260, 60, 51, 1);
    }
}

static int handle_mouse(const mouse *m)
{
    return generic_buttons_handle_mouse(m, 0, 0, graph_buttons, 3, &focus_button_id);
}

static void button_graph(int param1, int param2)
{
    int new_order;

    if (param2) {
        // go to housing advisor
        window_advisors_show_advisor(HOUSING_ADVISOR_ID);
    } else {
        switch (city_population_graph_order()) {
            default:
            case 0:
                new_order = param1 ? 5 : 2;
                break;
            case 1:
                new_order = param1 ? 3 : 4;
                break;
            case 2:
                new_order = param1 ? 4 : 0;
                break;
            case 3:
                new_order = param1 ? 1 : 5;
                break;
            case 4:
                new_order = param1 ? 2 : 1;
                break;
            case 5:
                new_order = param1 ? 0 : 3;
                break;
        }
        city_population_set_graph_order(new_order);
    }

    window_invalidate();
}

static int get_tooltip_text(void)
{
    if (focus_button_id && focus_button_id < 3) {
        return 111;
    } else {
        return 0;
    }
}

const advisor_window_type *window_advisor_population(void)
{
    static const advisor_window_type window = {
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip_text
    };
    return &window;
}
