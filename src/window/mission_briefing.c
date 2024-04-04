#include "mission_briefing.h"

#include "assets/assets.h"
#include "campaign/campaign.h"
#include "city/mission.h"
#include "core/image_group.h"
#include "core/lang.h"
#include "game/file.h"
#include "game/mission.h"
#include "game/settings.h"
#include "game/tutorial.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/rich_text.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/custom_messages.h"
#include "scenario/criteria.h"
#include "scenario/property.h"
#include "scenario/scenario.h"
#include "sound/device.h"
#include "sound/music.h"
#include "sound/speech.h"
#include "translation/translation.h"
#include "window/city.h"
#include "window/intermezzo.h"
#include "window/main_menu.h"
#include "window/mission_selection.h"
#include "window/plain_message_dialog.h"
#include "window/video.h"

static void show(void);
static void button_back(int param1, int param2);
static void button_start_mission(int param1, int param2);

static const int GOAL_OFFSETS_X[] = {32, 288, 32, 288, 288, 288};
static const int GOAL_OFFSETS_Y[] = {95, 95, 117, 117, 73, 135};

static image_button image_button_back = {
    0, 0, 31, 20, IB_NORMAL, GROUP_MESSAGE_ICON, 8, button_back, button_none, 0, 0, 1
};
static image_button image_button_start_mission = {
    0, 0, 27, 27, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 56, button_start_mission, button_none, 1, 0, 1
};

static struct {
    int is_review;
    int video_played;
    int has_audio;
    int focus_button;
    int campaign_mission_loaded;
} data;

static void init(void)
{
    data.focus_button = 0;
    data.has_audio = 0;
    rich_text_reset(0);
}

static int load_scenario_file(void)
{
    if (!campaign_is_active()) {
        return game_file_start_scenario_by_name(scenario_name());
    } else {
        return campaign_load_scenario(scenario_campaign_mission());
    }
}

static int has_briefing_message(void)
{
    if (!scenario_is_custom()) {
        return 1;
    }
    if (!scenario_intro_message()) {
        return 0;
    }
    custom_message_t *message = custom_messages_get(scenario_intro_message());
    if (!message) {
        return 0;
    }
    return custom_messages_get_video(message) || custom_messages_get_text(message) ||
        custom_messages_get_title(message) || custom_messages_get_subtitle(message);
}

static int play_video(void)
{
    if (!scenario_is_custom() || data.is_review || data.video_played) {
        return 0;
    }
    data.video_played = 1;
    uint8_t *video_file = custom_messages_get_video(custom_messages_get(scenario_intro_message()));
    if (video_file) {
        char utf8_filename[FILE_NAME_MAX];
        encoding_to_utf8(video_file, utf8_filename, FILE_NAME_MAX, encoding_system_uses_decomposed());
        window_video_show(utf8_filename, show);
        return 1;
    }
    return 0;
}

static void play_audio(void)
{
    if (!scenario_is_custom()) {
        return;
    }
    custom_message_t *custom_message = custom_messages_get(scenario_intro_message());

    const char *background_music = custom_messages_get_background_music(custom_message);
    if (background_music) {
        sound_device_play_music(background_music, setting_sound(SOUND_MUSIC)->volume, 0);
        data.has_audio = 1;
    }

    const char *audio_file = custom_messages_get_audio(custom_message);
    if (audio_file) {
        sound_speech_play_file(audio_file);
        data.has_audio = 1;
    }
}

static void draw_background_image(void)
{
    if (!scenario_is_custom()) {
        window_draw_underlying_window();
        return;
    }
    const campaign_scenario *scenario = campaign_get_scenario(scenario_campaign_mission());
    int image_id = 0;
    if (scenario->briefing_image_path) {
        image_id = assets_get_external_image(scenario->briefing_image_path, 0);
    } else {
        image_id = image_group(GROUP_INTERMEZZO_BACKGROUND) + 2 * (scenario_campaign_mission() % 11) + 1;
    }
    image_draw_fullscreen_background(image_id);
}

static void get_briefing_texts(const uint8_t **title, const uint8_t **subtitle, const uint8_t **content)
{
    if (!scenario_is_custom()) {
        int text_id = 200 + scenario_campaign_mission();
        const lang_message *msg = lang_get_message(text_id);
        *title = msg->title.text;
        *subtitle = msg->subtitle.text;
        *content = msg->content.text;
    } else {
        custom_message_t *custom_message = custom_messages_get(scenario_intro_message());
        *title = custom_messages_get_title(custom_message);
        *subtitle = custom_messages_get_subtitle(custom_message);
        *content = custom_messages_get_text(custom_message);
    }
}

static void draw_background(void)
{
    if (!data.campaign_mission_loaded) {
        if (!load_scenario_file()) {
            window_main_menu_show(1);
            setting_clear_personal_savings();
            scenario_settings_init();
            scenario_set_campaign_rank(2);
            window_plain_message_dialog_show(TR_WINDOW_CAMPAIGN_MISSION_FAILED_TO_LOAD_TITLE,
                TR_WINDOW_CAMPAIGN_MISSION_FAILED_TO_LOAD_TEXT, 0);
            return;
        }
        data.campaign_mission_loaded = 1;
    }

    if (!has_briefing_message()) {
        button_start_mission(0, 0);
        return;
    }

    if (play_video()) {
        return;
    }

    const uint8_t *title;
    const uint8_t *subtitle;
    const uint8_t *content;

    get_briefing_texts(&title, &subtitle, &content);

    if (!title && !subtitle && !content) {
        button_start_mission(0, 0);
        return;
    }

    play_audio();
    draw_background_image();

    graphics_in_dialog();

    outer_panel_draw(16, 32, 38, 27);

    if (title) {
        text_draw(title, 32, 48, FONT_LARGE_BLACK, 0);
    }
    if (subtitle) {
        text_draw(subtitle, 32, 78, FONT_NORMAL_BLACK, 0);
    }

    lang_text_draw(62, 7, 376, 433, FONT_NORMAL_BLACK);
    if (!data.is_review && game_mission_has_choice()) {
        lang_text_draw(13, 4, 66, 435, FONT_NORMAL_BLACK);
    }

    inner_panel_draw(32, 96, 33, 5);
    lang_text_draw(62, 10, 48, 104, FONT_NORMAL_WHITE);
    int goal_index = 0;
    if (scenario_criteria_population_enabled()) {
        int x = GOAL_OFFSETS_X[goal_index];
        int y = GOAL_OFFSETS_Y[goal_index];
        goal_index++;
        label_draw(16 + x, 32 + y, 15, 1);
        int width = lang_text_draw(62, 11, 16 + x + 8, 32 + y + 3, FONT_NORMAL_RED);
        text_draw_number(scenario_criteria_population(), '@', " ", 16 + x + 8 + width, 32 + y + 3, FONT_NORMAL_RED, 0);
    }
    if (scenario_criteria_culture_enabled()) {
        int x = GOAL_OFFSETS_X[goal_index];
        int y = GOAL_OFFSETS_Y[goal_index];
        goal_index++;
        label_draw(16 + x, 32 + y, 15, 1);
        int width = lang_text_draw(62, 12, 16 + x + 8, 32 + y + 3, FONT_NORMAL_RED);
        text_draw_number(scenario_criteria_culture(), '@', " ", 16 + x + 8 + width, 32 + y + 3, FONT_NORMAL_RED, 0);
    }
    if (scenario_criteria_prosperity_enabled()) {
        int x = GOAL_OFFSETS_X[goal_index];
        int y = GOAL_OFFSETS_Y[goal_index];
        goal_index++;
        label_draw(16 + x, 32 + y, 15, 1);
        int width = lang_text_draw(62, 13, 16 + x + 8, 32 + y + 3, FONT_NORMAL_RED);
        text_draw_number(scenario_criteria_prosperity(), '@', " ", 16 + x + 8 + width, 32 + y + 3, FONT_NORMAL_RED, 0);
    }
    if (scenario_criteria_peace_enabled()) {
        int x = GOAL_OFFSETS_X[goal_index];
        int y = GOAL_OFFSETS_Y[goal_index];
        goal_index++;
        label_draw(16 + x, 32 + y, 15, 1);
        int width = lang_text_draw(62, 14, 16 + x + 8, 32 + y + 3, FONT_NORMAL_RED);
        text_draw_number(scenario_criteria_peace(), '@', " ", 16 + x + 8 + width, 32 + y + 3, FONT_NORMAL_RED, 0);
    }
    if (scenario_criteria_favor_enabled()) {
        int x = GOAL_OFFSETS_X[goal_index];
        int y = GOAL_OFFSETS_Y[goal_index];
        goal_index++;
        label_draw(16 + x, 32 + y, 15, 1);
        int width = lang_text_draw(62, 15, 16 + x + 8, 32 + y + 3, FONT_NORMAL_RED);
        text_draw_number(scenario_criteria_favor(), '@', " ", 16 + x + 8 + width, 32 + y + 3, FONT_NORMAL_RED, 0);
    }
    int immediate_goal_text = tutorial_get_immediate_goal_text();
    if (immediate_goal_text) {
        int x = GOAL_OFFSETS_X[2];
        int y = GOAL_OFFSETS_Y[2];
        label_draw(16 + x, 32 + y, 31, 1);
        lang_text_draw(62, immediate_goal_text, 16 + x + 8, 32 + y + 3, FONT_NORMAL_RED);
    }

    inner_panel_draw(32, 184, 33, 15);

    if (content) {
        rich_text_set_fonts(FONT_NORMAL_WHITE, FONT_NORMAL_RED, 5);
        rich_text_init(content, 64, 184, 31, 15, 0);
        rich_text_draw(content, 48, 196, 496, 14, 0);
    }

    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    rich_text_draw_scrollbar();
    image_buttons_draw(516, 426, &image_button_start_mission, 1);
    if (!data.is_review && game_mission_has_choice()) {
        image_buttons_draw(26, 428, &image_button_back, 1);
    }

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);

    if (rich_text_handle_mouse(m_dialog)) {
        return;
    }
    if (image_buttons_handle_mouse(m_dialog, 516, 426, &image_button_start_mission, 1, 0)) {
        return;
    }
    if (!data.is_review && game_mission_has_choice()) {
        if (image_buttons_handle_mouse(m_dialog, 26, 428, &image_button_back, 1, 0)) {
            return;
        }
    }
}

static void button_back(int param1, int param2)
{
    if (!data.is_review) {
        sound_music_stop();
        sound_speech_stop();
        window_mission_selection_show_again();
    }
}

static void button_start_mission(int param1, int param2)
{
    if (!data.is_review || data.has_audio) {
        sound_music_stop();
        sound_speech_stop();
    }
    sound_music_update(1);
    window_city_show();
    if (!data.is_review) {
        city_mission_reset_save_start();
    }
}

static void show(void)
{
    window_type window = {
        WINDOW_MISSION_BRIEFING,
        draw_background,
        draw_foreground,
        handle_input
    };
    init();
    window_show(&window);
}

void window_mission_briefing_show(void)
{
    data.is_review = 0;
    data.video_played = 0;
    data.campaign_mission_loaded = 0;
    campaign_is_active() ? show() : window_intermezzo_show(INTERMEZZO_MISSION_BRIEFING, show);
}

void window_mission_briefing_show_review(void)
{
    data.is_review = 1;
    data.campaign_mission_loaded = 1;
    campaign_is_active() ? show() : window_intermezzo_show(INTERMEZZO_MISSION_BRIEFING, show);
}
