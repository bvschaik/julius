#include "house.h"

#include "building/building.h"
#include "building/model.h"
#include "city/constants.h"
#include "city/finance.h"
#include "city/sentiment.h"
#include "core/calc.h"
#include "game/resource.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "map/road_access.h"
#include "sound/speech.h"
#include "translation/translation.h"
#include "window/building/figures.h"

static void draw_vacant_lot(building_info_context *c)
{
    window_building_prepare_figure_list(c);
    if (c->can_play_sound) {
        c->can_play_sound = 0;
        if (c->figure.count > 0) {
            window_building_play_figure_phrase(c);
        } else {
            sound_speech_play_file("wavs/empty_land.wav");
        }
    }
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(128, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    window_building_draw_figure_list(c);

    int text_id = 2;
    building *b = building_get(c->building_id);
    if (map_closest_road_within_radius(b->x, b->y, 1, 2, 0, 0)) {
        text_id = 1;
    }
    window_building_draw_description_at(c, BLOCK_SIZE * c->height_blocks - 113, 128, text_id);
}

static void draw_population_info(building_info_context *c, int y_offset)
{
    building *b = building_get(c->building_id);
    int icon = 13;
    if (b->subtype.house_level <= HOUSE_GRAND_INSULA) {
        icon++;
    }

    image_draw(image_group(GROUP_CONTEXT_ICONS) + icon, c->x_offset + 34, y_offset + 4, COLOR_MASK_NONE, SCALE_NONE);
    int width = text_draw_number(b->house_population, '@', " ", c->x_offset + 50, y_offset + 14, FONT_NORMAL_BROWN, 0);
    width += lang_text_draw(127, 20, c->x_offset + 50 + width, y_offset + 14, FONT_NORMAL_BROWN);

    if (b->house_population_room < 0) {
        width += text_draw_number(-b->house_population_room, '@', " ",
            c->x_offset + 50 + width, y_offset + 14, FONT_NORMAL_BROWN, 0);
        lang_text_draw(127, 21, c->x_offset + 50 + width, y_offset + 14, FONT_NORMAL_BROWN);
    } else if (b->house_population_room > 0 && !b->has_plague) {
        width += lang_text_draw(127, 22, c->x_offset + 50 + width, y_offset + 14, FONT_NORMAL_BROWN);
        text_draw_number(b->house_population_room, '@', " ",
            c->x_offset + 50 + width, y_offset + 14, FONT_NORMAL_BROWN, 0);
    }
}

static void draw_tax_info(building_info_context *c, int y_offset)
{
    building *b = building_get(c->building_id);
    if (b->house_tax_coverage) {
        int pct = calc_adjust_with_percentage(b->tax_income_or_storage / 2, city_finance_tax_percentage());
        int width = lang_text_draw(127, 24, c->x_offset + 36, y_offset, FONT_NORMAL_BROWN);
        width += lang_text_draw_amount(8, 0, pct, c->x_offset + 36 + width, y_offset, FONT_NORMAL_BROWN);
        lang_text_draw(127, 25, c->x_offset + 36 + width, y_offset, FONT_NORMAL_BROWN);
    } else {
        lang_text_draw(127, 23, c->x_offset + 36, y_offset, FONT_NORMAL_BROWN);
    }
}

static void draw_happiness_info(building_info_context *c, int y_offset)
{
    int happiness = building_get(c->building_id)->sentiment.house_happiness;
    int sentiment_text_id = TR_BUILDING_WINDOW_HOUSE_SENTIMENT_1;
    if (happiness > 0) {
        sentiment_text_id = happiness / 10 + TR_BUILDING_WINDOW_HOUSE_SENTIMENT_2;
    }
    text_draw(translation_for(sentiment_text_id), c->x_offset + 36, y_offset, FONT_NORMAL_BROWN, 0);

    int message = building_get(c->building_id)->house_sentiment_message;
    switch (message) {
        case LOW_MOOD_CAUSE_NO_JOBS:
            text_draw(translation_for(TR_BUILDING_WINDOW_HOUSE_UPSET_UNEMPLOYMENT),
                c->x_offset + 36, y_offset + 20, FONT_NORMAL_BROWN, 0);
            break;
        case LOW_MOOD_CAUSE_HIGH_TAXES:
            text_draw(translation_for(TR_BUILDING_WINDOW_HOUSE_UPSET_HIGH_TAXES),
                c->x_offset + 36, y_offset + 20, FONT_NORMAL_BROWN, 0);
            break;
        case LOW_MOOD_CAUSE_LOW_WAGES:
            text_draw(translation_for(TR_BUILDING_WINDOW_HOUSE_UPSET_LOW_WAGES),
                c->x_offset + 36, y_offset + 20, FONT_NORMAL_BROWN, 0);
            break;
        case LOW_MOOD_CAUSE_SQUALOR:
            text_draw(translation_for(TR_BUILDING_WINDOW_HOUSE_UPSET_SQUALOR),
                c->x_offset + 36, y_offset + 20, FONT_NORMAL_BROWN, 0);
            break;
        case SUGGEST_MORE_ENT:
            text_draw(translation_for(TR_BUILDING_WINDOW_HOUSE_SUGGEST_ENTERTAINMENT),
                c->x_offset + 36, y_offset + 20, FONT_NORMAL_BROWN, 0);
            break;
        case SUGGEST_MORE_FOOD:
            text_draw(translation_for(TR_BUILDING_WINDOW_HOUSE_SUGGEST_FOOD),
                c->x_offset + 36, y_offset + 20, FONT_NORMAL_BROWN, 0);
            break;
        case SUGGEST_MORE_DESIRABILITY:
            text_draw(translation_for(TR_BUILDING_WINDOW_HOUSE_SUGGEST_DESIRABILITY),
                c->x_offset + 36, y_offset + 20, FONT_NORMAL_BROWN, 0);
            break;
        default:
            break;
    }

    if (city_sentiment_get_blessing_festival_boost() > 3) {
        text_draw(translation_for(TR_BUILDING_WINDOW_HOUSE_RECENT_EVENT_POSITIVE),
            c->x_offset + 36, y_offset + 40, FONT_NORMAL_BROWN, 0);
    } else if (city_sentiment_get_blessing_festival_boost() < -3) {
        text_draw(translation_for(TR_BUILDING_WINDOW_HOUSE_RECENT_EVENT_NEGATIVE),
            c->x_offset + 36, y_offset + 40, FONT_NORMAL_BROWN, 0);
    }
}

void window_building_draw_house(building_info_context *c)
{
    c->help_id = 56;
    building *b = building_get(c->building_id);
    if (b->house_population <= 0) {
        draw_vacant_lot(c);
        return;
    }
    window_building_play_sound(c, "wavs/housing.wav");
    int level = b->type - 10;
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(29, level, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    inner_panel_draw(c->x_offset + 16, c->y_offset + 148, c->width_blocks - 2, 13);

    draw_population_info(c, c->y_offset + 154);
    draw_tax_info(c, c->y_offset + 194);
    draw_happiness_info(c, c->y_offset + 214);
    int y_content = 279;
    int y_amount = 283;

    int resource_image = image_group(GROUP_RESOURCE_ICONS);
    // food inventory
    if (model_get_house(b->subtype.house_level)->food_types) {
        // wheat
        image_draw(resource_image + RESOURCE_WHEAT, c->x_offset + 32, c->y_offset + y_content,
            COLOR_MASK_NONE, SCALE_NONE);
        text_draw_number(b->data.house.inventory[INVENTORY_WHEAT], '@', " ",
            c->x_offset + 64, c->y_offset + y_amount, FONT_NORMAL_BROWN, 0);
        // vegetables
        image_draw(resource_image + RESOURCE_VEGETABLES, c->x_offset + 142, c->y_offset + y_content,
            COLOR_MASK_NONE, SCALE_NONE);
        text_draw_number(b->data.house.inventory[INVENTORY_VEGETABLES], '@', " ",
            c->x_offset + 174, c->y_offset + y_amount, FONT_NORMAL_BROWN, 0);
        // fruit
        image_draw(resource_image + RESOURCE_FRUIT, c->x_offset + 252, c->y_offset + y_content,
            COLOR_MASK_NONE, SCALE_NONE);
        text_draw_number(b->data.house.inventory[INVENTORY_FRUIT], '@', " ",
            c->x_offset + 284, c->y_offset + y_amount, FONT_NORMAL_BROWN, 0);
        // meat/fish
        image_draw(resource_image + RESOURCE_MEAT + resource_image_offset(RESOURCE_MEAT, RESOURCE_IMAGE_ICON),
            c->x_offset + 362, c->y_offset + y_content, COLOR_MASK_NONE, SCALE_NONE);
        text_draw_number(b->data.house.inventory[INVENTORY_MEAT], '@', " ",
            c->x_offset + 394, c->y_offset + y_amount, FONT_NORMAL_BROWN, 0);
    } else {
        // no food necessary
        lang_text_draw_multiline(127, 33, c->x_offset + 36, c->y_offset + y_content,
            BLOCK_SIZE * (c->width_blocks - 6), FONT_NORMAL_BROWN);
    }
    // goods inventory
    y_content += 35;
    y_amount += 35;
    // pottery
    image_draw(resource_image + RESOURCE_POTTERY, c->x_offset + 32, c->y_offset + y_content,
        COLOR_MASK_NONE, SCALE_NONE);
    text_draw_number(b->data.house.inventory[INVENTORY_POTTERY], '@', " ",
        c->x_offset + 64, c->y_offset + y_amount, FONT_NORMAL_BROWN, 0);
    // furniture
    image_draw(resource_image + RESOURCE_FURNITURE, c->x_offset + 142, c->y_offset + y_content,
        COLOR_MASK_NONE, SCALE_NONE);
    text_draw_number(b->data.house.inventory[INVENTORY_FURNITURE], '@', " ",
        c->x_offset + 174, c->y_offset + y_amount, FONT_NORMAL_BROWN, 0);
    // oil
    image_draw(resource_image + RESOURCE_OIL, c->x_offset + 252, c->y_offset + y_content, COLOR_MASK_NONE, SCALE_NONE);
    text_draw_number(b->data.house.inventory[INVENTORY_OIL], '@', " ",
        c->x_offset + 284, c->y_offset + y_amount, FONT_NORMAL_BROWN, 0);
    // wine
    image_draw(resource_image + RESOURCE_WINE, c->x_offset + 362, c->y_offset + y_content, COLOR_MASK_NONE, SCALE_NONE);
    text_draw_number(b->data.house.inventory[INVENTORY_WINE], '@', " ",
        c->x_offset + 394, c->y_offset + y_amount, FONT_NORMAL_BROWN, 0);

    if (b->has_plague) {
        lang_text_draw_multiline(CUSTOM_TRANSLATION, TR_BUILDING_HOUSE_DISEASE_DESC,
            c->x_offset + 32, c->y_offset + 70, BLOCK_SIZE * (c->width_blocks - 4), FONT_NORMAL_BLACK);
    } else if (b->data.house.evolve_text_id == 62) {
        int width = lang_text_draw(127, 40 + b->data.house.evolve_text_id,
            c->x_offset + 32, c->y_offset + 60, FONT_NORMAL_BLACK);
        width += lang_text_draw_colored(41, c->worst_desirability_building_type,
            c->x_offset + 32 + width, c->y_offset + 60, FONT_NORMAL_PLAIN, COLOR_FONT_RED);
        text_draw((uint8_t *) ")", c->x_offset + 32 + width, c->y_offset + 60, FONT_NORMAL_BLACK, 0);
        lang_text_draw_multiline(127, 41 + b->data.house.evolve_text_id,
            c->x_offset + 32, c->y_offset + 76, BLOCK_SIZE * (c->width_blocks - 4), FONT_NORMAL_BLACK);
    } else {
        lang_text_draw_multiline(127, 40 + b->data.house.evolve_text_id,
            c->x_offset + 32, c->y_offset + 70, BLOCK_SIZE * (c->width_blocks - 4), FONT_NORMAL_BLACK);
    }
}
