#include "entertainment.h"

#include "building/count.h"
#include "city/culture.h"
#include "city/entertainment.h"
#include "city/gods.h"
#include "city/houses.h"

#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "translation/translation.h"
#include "window/option_popup.h"

option_menu_item games_options[4] = {
   {0, 0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0, 0},
};

#define ADVISOR_HEIGHT 27

#define PEOPLE_OFFSET 330
#define COVERAGE_OFFSET 470
#define COVERAGE_WIDTH 130

static void button_hold_games(int param1, int param2);

static generic_button hold_games_button[] = {
    {102, 360, 300, 20, button_hold_games, button_none, 0, 0},
};

static int focus_button_id;

static int get_entertainment_advice(void)
{
    const house_demands *demands = city_houses_demands();
    if (demands->missing.entertainment > demands->missing.more_entertainment) {
        return 3;
    } else if (!demands->missing.more_entertainment) {
        return city_culture_average_entertainment() ? 1 : 0;
    } else if (city_entertainment_venue_needing_shows()) {
        return 3 + city_entertainment_venue_needing_shows();
    } else {
        return 2;
    }
}

static void draw_games_info(void)
{
    inner_panel_draw(48, 292, 34, 6);
    //image_draw(image_group(GROUP_PANEL_WINDOWS) + 14, 460, 295);
    text_draw(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_GAMES_HEADER), 52, 264, FONT_LARGE_BLACK, 0);
    text_draw_multiline(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_GAMES_DESC), 56, 300, 400, FONT_NORMAL_WHITE, 0);

    if (0) {
        text_draw_centered(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_GAMES_PREPARING), 102, 365, 300, FONT_NORMAL_WHITE, 0);
    }
    else {
        text_draw_centered(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_GAMES_BUTTON), 102, 365, 300, FONT_NORMAL_WHITE, 0);
    }
}


static int draw_background(void)
{
    city_gods_calculate_moods(0);
    city_culture_calculate();

    outer_panel_draw(0, 0, 40, ADVISOR_HEIGHT);
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 8, 10, 10);

    lang_text_draw(58, 0, 60, 12, FONT_LARGE_BLACK);

    lang_text_draw(58, 1, 180, 46, FONT_SMALL_PLAIN);
    lang_text_draw(58, 2, 260, 46, FONT_SMALL_PLAIN);
    lang_text_draw(58, 3, PEOPLE_OFFSET + 10, 46, FONT_SMALL_PLAIN);
    lang_text_draw_centered(58, 4, COVERAGE_OFFSET, 46, COVERAGE_WIDTH, FONT_SMALL_PLAIN);

    inner_panel_draw(32, 60, 36, 7);


    // taverns
    //TR_WINDOW_ADVISOR_ENTERTAINMENT_TAVERN_COVERAGE
    text_draw(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_TAVERN_COVERAGE), 70, 64, FONT_NORMAL_WHITE, 0);
    text_draw_number(building_count_total(BUILDING_TAVERN), '@', "", 40, 64, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_active(BUILDING_TAVERN), 150, 64, 100, FONT_NORMAL_WHITE);
    int width = text_draw_number(TAVERN_COVERAGE * building_count_active(BUILDING_TAVERN), '_', " ",
        PEOPLE_OFFSET, 64, FONT_NORMAL_WHITE);
    lang_text_draw(58, 5, PEOPLE_OFFSET + width, 64, FONT_NORMAL_WHITE);
    int pct_tavern = city_culture_coverage_tavern();
    if (pct_tavern == 0) {
        lang_text_draw_centered(57, 10, COVERAGE_OFFSET, 64, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    }
    else if (pct_tavern < 100) {
        lang_text_draw_centered(57, 11 + pct_tavern / 10, COVERAGE_OFFSET, 64, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
        //lang_text_draw_centered(57, 17, COVERAGE_OFFSET, 64, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    }
    else {
        lang_text_draw_centered(57, 21, COVERAGE_OFFSET, 64, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    }

    // theaters
    lang_text_draw_amount(8, 34, building_count_total(BUILDING_THEATER), 40, 84, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_active(BUILDING_THEATER), 150, 84, 100, FONT_NORMAL_WHITE);
    text_draw_number_centered(city_entertainment_theater_shows(), 230, 84, 100, FONT_NORMAL_WHITE);
    width = text_draw_number(THEATER_COVERAGE * building_count_active(BUILDING_THEATER), '_', " ",
        PEOPLE_OFFSET, 84, FONT_NORMAL_WHITE);
    lang_text_draw(58, 5, PEOPLE_OFFSET + width, 84, FONT_NORMAL_WHITE);
    int pct_theater = city_culture_coverage_theater();
    if (pct_theater == 0) {
        lang_text_draw_centered(57, 10, COVERAGE_OFFSET, 84, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    } else if (pct_theater < 100) {
        lang_text_draw_centered(57, 11 + pct_theater / 10, COVERAGE_OFFSET, 84, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
        //lang_text_draw_centered(57, 17, COVERAGE_OFFSET, 64, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    } else {
        lang_text_draw_centered(57, 21, COVERAGE_OFFSET, 84, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    }

    // amphitheaters
    lang_text_draw_amount(8, 36, building_count_total(BUILDING_AMPHITHEATER), 40, 104, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_active(BUILDING_AMPHITHEATER), 150, 104, 100, FONT_NORMAL_WHITE);
    text_draw_number_centered(city_entertainment_amphitheater_shows(), 230, 104, 100, FONT_NORMAL_WHITE);
    width = text_draw_number(AMPHITHEATER_COVERAGE * building_count_active(BUILDING_AMPHITHEATER), '@', " ",
        PEOPLE_OFFSET, 104, FONT_NORMAL_WHITE);
    lang_text_draw(58, 5, PEOPLE_OFFSET + width, 104, FONT_NORMAL_WHITE);
    int pct_amphitheater = city_culture_coverage_amphitheater();
    if (pct_amphitheater == 0) {
        lang_text_draw_centered(57, 10, COVERAGE_OFFSET, 104, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    } else if (pct_amphitheater < 100) {
        lang_text_draw_centered(57, 11 + pct_amphitheater / 10,
            COVERAGE_OFFSET, 104, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    } else {
        lang_text_draw_centered(57, 21, COVERAGE_OFFSET, 104, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    }

    // colosseums
    lang_text_draw_amount(8, 38, building_count_total(BUILDING_COLOSSEUM), 40, 124, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_active(BUILDING_COLOSSEUM), 150, 124, 100, FONT_NORMAL_WHITE);
    text_draw_number_centered(city_entertainment_colosseum_shows(), 230, 124, 100, FONT_NORMAL_WHITE);
    lang_text_draw_centered(58, 6, PEOPLE_OFFSET + 10, 124, 100, FONT_NORMAL_WHITE);
    if (city_culture_coverage_colosseum() == 0) {
        lang_text_draw_centered(57, 10, COVERAGE_OFFSET, 124, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    }
    else {
        lang_text_draw_centered(57, 21, COVERAGE_OFFSET, 124, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    }

    // hippodromes
    lang_text_draw_amount(8, 40, building_count_total(BUILDING_HIPPODROME), 40, 143, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_active(BUILDING_HIPPODROME), 150, 143, 100, FONT_NORMAL_WHITE);
    text_draw_number_centered(city_entertainment_hippodrome_shows(), 230, 143, 100, FONT_NORMAL_WHITE);
    lang_text_draw_centered(58, 6, PEOPLE_OFFSET + 10, 143, 100, FONT_NORMAL_WHITE);
    if (city_culture_coverage_hippodrome() == 0) {
        lang_text_draw_centered(57, 10, COVERAGE_OFFSET, 143, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    } else {
        lang_text_draw_centered(57, 21, COVERAGE_OFFSET, 143, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    }

    lang_text_draw_multiline(58, 7 + get_entertainment_advice(), 60, 178, 512, FONT_NORMAL_BLACK);

    draw_games_info();

    return ADVISOR_HEIGHT;
}

static void draw_foreground(void)
{
    if (1) {
        button_border_draw(102, 360, 300, 20, focus_button_id == 1);
    }
}

static int handle_mouse(const mouse* m)
{
    return generic_buttons_handle_mouse(m, 0, 0, hold_games_button, 1, &focus_button_id);
}

static void hold_games()
{

}

static void button_hold_games(int param1, int param2)
{
    window_option_popup_show(TR_SELECT_GAMES_HEADER, TR_SELECT_GAMES_TEXT, games_options, hold_games, 1);
}

const advisor_window_type *window_advisor_entertainment(void)
{
    static const advisor_window_type window = {
        draw_background,
        draw_foreground,
        handle_mouse,
        0
    };
    return &window;
}
