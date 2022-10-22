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
    {102, 370, 300, 20, button_hold_games, button_none, 0, 0},
};

struct games_text {
    translation_key preparation_text;
    translation_key ongoing_text;
} text_data[] = {
    {0,0}, // 0 element unused
    {TR_WINDOW_ADVISOR_ENTERTAINMENT_PREPARING_NG, TR_WINDOW_ADVISOR_ENTERTAINMENT_UNDERWAY_NG},
    {TR_WINDOW_ADVISOR_ENTERTAINMENT_PREPARING_AG, TR_WINDOW_ADVISOR_ENTERTAINMENT_UNDERWAY_AG},
    {TR_WINDOW_ADVISOR_ENTERTAINMENT_PREPARING_IG, TR_WINDOW_ADVISOR_ENTERTAINMENT_UNDERWAY_IG},
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

void window_entertainment_draw_games_text(int x, int y)
{
    games_type *game = city_games_get_game_type(city_festival_selected_game_id());
    int cooldown = city_festival_games_cooldown();

    if (cooldown) {
        text_draw_centered(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_GAMES_COOLDOWN_TEXT), x, y + 15, 400, FONT_NORMAL_WHITE, 0);
        int width = text_draw(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_GAMES_COOLDOWN), x + 46, y + 50, FONT_NORMAL_WHITE, 0);
        text_draw_number(cooldown, '@', "", x + 46 + width, y + 50, FONT_NORMAL_WHITE, 0);
    } else if (city_festival_games_planning_time()) {
        text_draw_centered(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_GAMES_PREPARING), x, y + 15, 400, FONT_NORMAL_WHITE, 0);
        int width = text_draw(translation_for(text_data[game->id].preparation_text), x + 56, y + 50, FONT_NORMAL_WHITE, 0);
        text_draw_number(city_festival_games_planning_time(), '@', "", x + 56 + width, y + 50, FONT_NORMAL_WHITE, 0);
    } else if (city_festival_games_active()) {
        text_draw_multiline(translation_for(text_data[game->id].ongoing_text), x + 4, y, 400, FONT_NORMAL_WHITE, 0);
    } else {
        text_draw_multiline(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_GAMES_DESC), x + 4, y, 400, FONT_NORMAL_WHITE, 0);
        text_draw_centered(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_GAMES_BUTTON), x + 56, y + 60, 300, FONT_NORMAL_WHITE, 0);
    }
}

static void draw_games_info(void)
{
    inner_panel_draw(48, 302, 34, 6);
    text_draw(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_GAMES_HEADER), 52, 274, FONT_LARGE_BLACK, 0);
    image_draw(assets_get_image_id("UI", "HoldGames Banner"), 460, 305, COLOR_MASK_NONE, SCALE_NONE);
    image_draw_border(assets_get_image_id("UI", "HoldGames Banner Border"), 460, 305, COLOR_MASK_NONE);
    window_entertainment_draw_games_text(56, 315);
}

static int draw_background(void)
{
    city_gods_calculate_moods(0);
    city_culture_calculate();

    outer_panel_draw(0, 0, 40, ADVISOR_HEIGHT);
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 8, 10, 10, COLOR_MASK_NONE, SCALE_NONE);

    lang_text_draw(58, 0, 60, 12, FONT_LARGE_BLACK);

    lang_text_draw(58, 1, 180, 46, FONT_SMALL_PLAIN);
    lang_text_draw(58, 2, 260, 46, FONT_SMALL_PLAIN);
    lang_text_draw(58, 3, PEOPLE_OFFSET + 10, 46, FONT_SMALL_PLAIN);
    lang_text_draw_centered(58, 4, COVERAGE_OFFSET, 46, COVERAGE_WIDTH, FONT_SMALL_PLAIN);

    inner_panel_draw(32, 60, 36, 8);

    // taverns
    text_draw(translation_for(TR_WINDOW_ADVISOR_ENTERTAINMENT_TAVERN_COVERAGE), 67, 64, FONT_NORMAL_WHITE, 0);
    text_draw_number(building_count_total(BUILDING_TAVERN), '@', " ", 40, 64, FONT_NORMAL_WHITE, 0);
    text_draw_number_centered(building_count_active(BUILDING_TAVERN), 150, 64, 100, FONT_NORMAL_WHITE);
    int width = text_draw_number(city_culture_get_tavern_person_coverage(), '_', " ",
        PEOPLE_OFFSET, 64, FONT_NORMAL_WHITE, 0);
    lang_text_draw(58, 5, PEOPLE_OFFSET + width, 64, FONT_NORMAL_WHITE);
    int pct_tavern = city_culture_coverage_tavern();
    if (pct_tavern == 0) {
        lang_text_draw_centered(57, 10, COVERAGE_OFFSET, 64, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    } else if (pct_tavern < 100) {
        lang_text_draw_centered(57, 11 + pct_tavern / 10, COVERAGE_OFFSET, 64, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    } else {
        lang_text_draw_centered(57, 21, COVERAGE_OFFSET, 64, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    }

    // theaters
    lang_text_draw_amount(8, 34, building_count_total(BUILDING_THEATER), 40, 84, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_active(BUILDING_THEATER), 150, 84, 100, FONT_NORMAL_WHITE);
    text_draw_number_centered(city_entertainment_theater_shows(), 230, 84, 100, FONT_NORMAL_WHITE);
    width = text_draw_number(city_culture_get_theatre_person_coverage(), '_', " ",
        PEOPLE_OFFSET, 84, FONT_NORMAL_WHITE, 0);
    lang_text_draw(58, 5, PEOPLE_OFFSET + width, 84, FONT_NORMAL_WHITE);
    int pct_theater = city_culture_coverage_theater();
    if (pct_theater == 0) {
        lang_text_draw_centered(57, 10, COVERAGE_OFFSET, 84, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    } else if (pct_theater < 100) {
        lang_text_draw_centered(57, 11 + pct_theater / 10, COVERAGE_OFFSET, 84, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    } else {
        lang_text_draw_centered(57, 21, COVERAGE_OFFSET, 84, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    }

    // amphitheaters
    lang_text_draw_amount(8, 36, building_count_total(BUILDING_AMPHITHEATER), 40, 104, FONT_NORMAL_WHITE);
    text_draw_number_centered(building_count_active(BUILDING_AMPHITHEATER), 150, 104, 100, FONT_NORMAL_WHITE);
    text_draw_number_centered(city_entertainment_amphitheater_shows(), 230, 104, 100, FONT_NORMAL_WHITE);
    width = text_draw_number(city_culture_get_ampitheatre_person_coverage(), '@', " ",
        PEOPLE_OFFSET, 104, FONT_NORMAL_WHITE, 0);
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
    text_draw_number(building_count_total(BUILDING_ARENA), '@', "", 40, 124, FONT_NORMAL_WHITE, 0);
    text_draw_number_centered(building_count_active(BUILDING_ARENA), 150, 124, 100, FONT_NORMAL_WHITE);
    width = text_draw_number(city_culture_get_arena_person_coverage(), '_', " ", PEOPLE_OFFSET, 124, FONT_NORMAL_WHITE, 0);
    lang_text_draw(58, 5, PEOPLE_OFFSET + width, 124, FONT_NORMAL_WHITE);
    text_draw_number_centered(city_entertainment_arena_shows(), 230, 124, 100, FONT_NORMAL_WHITE);
    int pct = city_culture_coverage_arena();
    if (pct == 0) {
        lang_text_draw_centered(57, 10, COVERAGE_OFFSET, 124, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    } else if (pct < 100) {
        lang_text_draw_centered(57, 11 + pct / 10, COVERAGE_OFFSET, 124, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    } else {
        lang_text_draw_centered(57, 21, COVERAGE_OFFSET, 124, COVERAGE_WIDTH, FONT_NORMAL_WHITE);
    }

    // colosseums
    int has_colosseum = building_count_active(BUILDING_COLOSSEUM) ? 1 : 0;
    lang_text_draw(CUSTOM_TRANSLATION, TR_ADVISOR_NO_ACTIVE_COLOSSEUM + has_colosseum, 40, 143, FONT_NORMAL_WHITE);
    lang_text_draw_centered(57, has_colosseum ? 21 : 10, COVERAGE_OFFSET, 143, COVERAGE_WIDTH, FONT_NORMAL_WHITE);

    // hippodromes
    int has_hippodrome = building_count_active(BUILDING_HIPPODROME) ? 1 : 0;
    lang_text_draw(CUSTOM_TRANSLATION, TR_ADVISOR_NO_ACTIVE_HIPPODROME + has_hippodrome, 40, 163, FONT_NORMAL_WHITE);
    lang_text_draw_centered(57, has_hippodrome ? 21 : 10, COVERAGE_OFFSET, 163, COVERAGE_WIDTH, FONT_NORMAL_WHITE);

    lang_text_draw_multiline(58, 7 + get_entertainment_advice(), 60, 198, 512, FONT_NORMAL_BLACK);

    draw_games_info();

    return ADVISOR_HEIGHT;
}

static void draw_foreground(void)
{
    if (!city_festival_games_cooldown() && !city_festival_games_planning_time() && !city_festival_games_active()) {
        button_border_draw(102, 370, 300, 20, focus_button_id == 1);
    }

}

static int handle_mouse(const mouse *m)
{
    return generic_buttons_handle_mouse(m, 0, 0, hold_games_button, 1, &focus_button_id);
}

static void button_hold_games(int param1, int param2)
{
    window_hold_games_show(0);
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
