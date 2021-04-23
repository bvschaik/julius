#include "entertainment.h"

#include "assets/assets.h"
#include "building/count.h"
#include "city/culture.h"
#include "city/entertainment.h"
#include "city/festival.h"
#include "city/finance.h"
#include "city/games.h"
#include "city/gods.h"
#include "city/houses.h"
#include "core/calc.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "translation/translation.h"
#include "window/hold_games.h"

#define ADVISOR_HEIGHT 27

#define PEOPLE_OFFSET 330
#define COVERAGE_OFFSET 470
#define COVERAGE_WIDTH 130

static int focus_button_id;
static void button_hold_games(int param1, int param2);

static generic_button hold_games_button[] = {
    {102, 380, 300, 20, button_hold_games, button_none, 0, 0},
};

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
    games_type *game = city_games_get_game_type(city_festival_selected_game_id());
    int cooldown = city_festival_games_cooldown();

    inner_panel_draw(48, 312, 34, 6);
    text_draw(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_GAMES_HEADER), 52, 284, FONT_LARGE_BLACK, 0);

    image_draw(assets_get_image_id(assets_get_group_id("Areldir", "UI_Elements"), "HoldGames Banner"), 460, 315);

    if (cooldown) {
        text_draw_centered(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_GAMES_COOLDOWN_TEXT), 56, 340, 400, FONT_NORMAL_WHITE, 0);
        int width = text_draw(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_GAMES_COOLDOWN), 102, 375, FONT_NORMAL_WHITE, 0);
        text_draw_number(cooldown, '@', "", 102 + width, 375, FONT_NORMAL_WHITE);

    }     else if (city_festival_games_planning_time()) {
        text_draw_centered(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_GAMES_PREPARING), 56, 340, 400, FONT_NORMAL_WHITE, 0);
        int width = text_draw(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_PREPARING_NG + ((game->id - 1) * 2)), 102, 375, FONT_NORMAL_WHITE, 0);
        text_draw_number(city_festival_games_planning_time(), '@', "", 102 + width, 375, FONT_NORMAL_WHITE);
    }     else if (city_festival_games_active()) {
        text_draw_multiline(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_UNDERWAY_NG + ((game->id - 1) * 2)), 60, 325, 400, FONT_NORMAL_WHITE, 0);
    }     else {
        text_draw_multiline(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_GAMES_DESC), 60, 325, 400, FONT_NORMAL_WHITE, 0);
        text_draw_centered(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_GAMES_BUTTON), 102, 385, 300, FONT_NORMAL_WHITE, 0);
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

    inner_panel_draw(32, 60, 36, 8);


    // taverns
    text_draw(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_TAVERN_COVERAGE), 67, 64, FONT_NORMAL_WHITE, 0);
    text_draw_number(building_count_total(BUILDING_TAVERN), '@', "", 40, 64, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_active(BUILDING_TAVERN), 150, 64, 100, FONT_NORMAL_WHITE);
    int width = text_draw_number(city_culture_get_tavern_person_coverage(), '_', " ",
        PEOPLE_OFFSET, 64, FONT_NORMAL_WHITE);
    lang_text_draw(58, 5, PEOPLE_OFFSET + width, 64, FONT_NORMAL_WHITE);
    int pct_tavern = city_culture_coverage_tavern();
    if (pct_tavern == 0) {
        lang_text_draw_centered(57, 10, COVERAGE_OFFSET, 64, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    }     else if (pct_tavern < 100) {
        lang_text_draw_centered(57, 11 + pct_tavern / 10, COVERAGE_OFFSET, 64, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
        //lang_text_draw_centered(57, 17, COVERAGE_OFFSET, 64, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    }     else {
        lang_text_draw_centered(57, 21, COVERAGE_OFFSET, 64, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    }

    // theaters
    lang_text_draw_amount(8, 34, building_count_total(BUILDING_THEATER), 40, 84, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_active(BUILDING_THEATER), 150, 84, 100, FONT_NORMAL_WHITE);
    text_draw_number_centered(city_entertainment_theater_shows(), 230, 84, 100, FONT_NORMAL_WHITE);
    width = text_draw_number(city_culture_get_theatre_person_coverage(), '_', " ",
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
    width = text_draw_number(city_culture_get_ampitheatre_person_coverage(), '@', " ",
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

    //arenas 
    text_draw(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_ARENA_COVERAGE), 67, 124, FONT_NORMAL_WHITE, 0);
    text_draw_number(building_count_total(BUILDING_ARENA), '@', "", 40, 124, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_active(BUILDING_ARENA), 150, 124, 100, FONT_NORMAL_WHITE);
    width = text_draw_number(city_culture_get_arena_person_coverage(), '_', " ", PEOPLE_OFFSET, 124, FONT_NORMAL_WHITE);
    lang_text_draw(58, 5, PEOPLE_OFFSET + width, 124, FONT_NORMAL_WHITE);
    text_draw_number_centered(city_entertainment_colosseum_shows(), 230, 124, 100, FONT_NORMAL_WHITE);
    int pct = city_culture_coverage_arena();
    if (pct == 0) {
        lang_text_draw_centered(57, 10, COVERAGE_OFFSET, 124, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    }     else if (pct < 100) {
        lang_text_draw_centered(57, 11 + pct / 10, COVERAGE_OFFSET, 124, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
        //lang_text_draw_centered(57, 17, COVERAGE_OFFSET, 124, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    }     else {
        lang_text_draw_centered(57, 21, COVERAGE_OFFSET, 124, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    }

    // colosseums
    lang_text_draw_amount(8, 38, building_count_total(BUILDING_COLOSSEUM), 40, 143, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_active(BUILDING_COLOSSEUM), 150, 143, 100, FONT_NORMAL_WHITE);
    text_draw_number_centered(city_entertainment_colosseum_shows(), 230, 143, 100, FONT_NORMAL_WHITE);
    lang_text_draw_centered(58, 6, PEOPLE_OFFSET + 10, 143, 100, FONT_NORMAL_WHITE);
    if (city_culture_coverage_colosseum() == 0) {
        lang_text_draw_centered(57, 10, COVERAGE_OFFSET, 143, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    }     else {
        lang_text_draw_centered(57, 21, COVERAGE_OFFSET, 143, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    }

    // hippodromes
    lang_text_draw_amount(8, 40, building_count_total(BUILDING_HIPPODROME), 40, 163, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_active(BUILDING_HIPPODROME), 150, 163, 100, FONT_NORMAL_WHITE);
    text_draw_number_centered(city_entertainment_hippodrome_shows(), 230, 163, 100, FONT_NORMAL_WHITE);
    lang_text_draw_centered(58, 6, PEOPLE_OFFSET + 10, 163, 100, FONT_NORMAL_WHITE);
    if (city_culture_coverage_hippodrome() == 0) {
        lang_text_draw_centered(57, 10, COVERAGE_OFFSET, 163, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    } else {
        lang_text_draw_centered(57, 21, COVERAGE_OFFSET, 163, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    }

    lang_text_draw_multiline(58, 7 + get_entertainment_advice(), 60, 198, 512, FONT_NORMAL_BLACK);

    draw_games_info();

    return ADVISOR_HEIGHT;
}

static void draw_foreground(void)
{
    if (!city_festival_games_cooldown() && !city_festival_games_planning_time() && !city_festival_games_active()) {
        button_border_draw(102, 380, 300, 20, focus_button_id == 1);
    }

}

static int handle_mouse(const mouse *m)
{
    return generic_buttons_handle_mouse(m, 0, 0, hold_games_button, 2, &focus_button_id);
}

static void button_hold_games(int param1, int param2)
{
    window_hold_games_show();
}

static void get_tooltip_text(advisor_tooltip_result *r)
{
    if (focus_button_id) {
        r->translation_key = TR_TOOLTIP_ADVISOR_ENTERTAINMENT_GAMES_BUTTON;
    }
}

const advisor_window_type *window_advisor_entertainment(void)
{
    static const advisor_window_type window = {
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip_text
    };
    return &window;
}
