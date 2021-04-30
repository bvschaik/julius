#include "trade_policy.h"

#include "assets/assets.h"
#include "building/caravanserai.h"
#include "building/monument.h"
#include "city/buildings.h"
#include "city/finance.h"
#include "graphics/image.h"
#include "graphics/text.h"
#include "sound/speech.h"
#include "translation/translation.h"
#include "window/option_popup.h"

static struct {
    int land_policy;
} data;

static option_menu_item caravanserai_policy_options[3] = {
        { TR_BUILDING_CARAVANSERAI_POLICY_1_TITLE, TR_BUILDING_CARAVANSERAI_POLICY_1 },
        { TR_BUILDING_CARAVANSERAI_POLICY_2_TITLE, TR_BUILDING_CARAVANSERAI_POLICY_2 },
        { TR_BUILDING_CARAVANSERAI_POLICY_3_TITLE, TR_BUILDING_CARAVANSERAI_POLICY_3 }
};

static option_menu_item lighthouse_policy_options[3] = {
        {TR_BUILDING_LIGHTHOUSE_POLICY_1_TITLE, TR_BUILDING_LIGHTHOUSE_POLICY_1 },
        {TR_BUILDING_LIGHTHOUSE_POLICY_2_TITLE, TR_BUILDING_LIGHTHOUSE_POLICY_2 },
        {TR_BUILDING_LIGHTHOUSE_POLICY_3_TITLE, TR_BUILDING_LIGHTHOUSE_POLICY_3 }
};

static int no_land_policy_image;
static int no_sea_policy_image;

static void apply_policy(int selection)
{
    if (!selection) {
        return;
    }

    building *b = data.land_policy ?  building_get(city_buildings_get_caravanserai()) : building_get(building_find(BUILDING_LIGHTHOUSE));
    sound_speech_play_file(data.land_policy ? "wavs/market4.wav" : "wavs/dock1.wav");

    city_finance_process_construction(TRADE_POLICY_COST);
    building_monument_add_module(b, selection);
}

static int init_images(int land_policy)
{
    int no_policy_image = land_policy ? no_land_policy_image : no_sea_policy_image;

    if(!no_land_policy_image && land_policy) {
        no_land_policy_image = assets_get_image_id(assets_get_group_id("Areldir", "Econ_Logistics"),
                                                   "Trade Policy");
        caravanserai_policy_options[0].image_id = no_land_policy_image + 1;
        caravanserai_policy_options[1].image_id = no_land_policy_image + 2;
        caravanserai_policy_options[2].image_id = no_land_policy_image + 3;

        no_policy_image = no_land_policy_image;
    }
    if(!no_sea_policy_image && !land_policy) {
        no_sea_policy_image = assets_get_image_id(assets_get_group_id("Areldir", "Econ_Logistics"),
                                                  "Sea Trade Policy");

        lighthouse_policy_options[0].image_id = no_sea_policy_image + 1;
        lighthouse_policy_options[1].image_id = no_sea_policy_image + 2;
        lighthouse_policy_options[2].image_id = no_sea_policy_image + 3;

        no_policy_image = no_sea_policy_image;
    }

    return no_policy_image;
}

void window_building_draw_policy_action(building_info_context *c, int land_policy)
{
    option_menu_item *policy_options = land_policy ? caravanserai_policy_options : lighthouse_policy_options;

    int no_policy_image = init_images(land_policy);

    int policy = building_monument_module_type(land_policy ? BUILDING_CARAVANSERAI : BUILDING_LIGHTHOUSE);
    if (!policy) {
        text_draw_multiline(translation_for(land_policy ? TR_BUILDING_CARAVANSERAI_NO_POLICY : TR_BUILDING_LIGHTHOUSE_NO_POLICY),
                            c->x_offset + 160, c->y_offset + 160, 260, FONT_NORMAL_BLACK, 0);
        image_draw(no_policy_image, c->x_offset + 32, c->y_offset + 150);
    } else {
        text_draw_multiline(translation_for(policy_options[policy - 1].header),
                            c->x_offset + 160, c->y_offset + 160, 260, FONT_NORMAL_BLACK, 0);
        text_draw_multiline(translation_for(policy_options[policy - 1].header + 1),
                            c->x_offset + 160, c->y_offset + 185, 260, FONT_NORMAL_BLACK, 0);
        image_draw(policy_options[policy - 1].image_id, c->x_offset + 32, c->y_offset + 150);
    }
}

void window_policy_show(int land_policy)
{
    init_images(land_policy);

    data.land_policy = land_policy;
    building *b = land_policy ? building_get(city_buildings_get_caravanserai()) : building_get(building_find(BUILDING_LIGHTHOUSE));
    int current_policy = b->data.monument.upgrades;

    window_option_popup_show(land_policy ? TR_BUILDING_CARAVANSERAI_POLICY_TITLE : TR_BUILDING_LIGHTHOUSE_POLICY_TITLE,
                             land_policy ? TR_BUILDING_CARAVANSERAI_POLICY_TEXT : TR_BUILDING_LIGHTHOUSE_POLICY_TEXT,
                             land_policy ? caravanserai_policy_options : lighthouse_policy_options, 3,
                             apply_policy, current_policy, TRADE_POLICY_COST, OPTION_MENU_SMALL_ROW);
}
