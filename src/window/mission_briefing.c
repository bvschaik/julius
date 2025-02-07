#include "mission_briefing.h"

#include "assets/assets.h"
#include "city/mission.h"
#include "core/image_group.h"
#include "core/lang.h"
#include "game/campaign.h"
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
#include "scenario/event/controller.h"
#include "scenario/property.h"
#include "scenario/scenario.h"
#include "sound/device.h"
#include "sound/music.h"
#include "sound/speech.h"
#include "translation/translation.h"
#include "window/city.h"
#include "window/intermezzo.h"
#include "window/main_menu.h"
#include "window/mission_list.h"
#include "window/mission_selection.h"
#include "window/plain_message_dialog.h"
#include "window/video.h"

#include <string.h>

typedef enum {
    BUTTON_GO_BACK_NONE = 0,
    BUTTON_GO_BACK_MISSION_SELECTION = 1,
    BUTTON_GO_BACK_SCENARIO_SELECTION
} button_go_back_action;

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
    button_go_back_action back_action;
    int video_played;
    int audio_played;
    int focus_button;
    int background_image_id;
    struct {
        char audio[FILE_NAME_MAX];
        char speech[FILE_NAME_MAX];
        char background_music[FILE_NAME_MAX];
    } paths;
    int file_loaded;
} data;

static void init(void)
{
    data.focus_button = 0;
    data.audio_played = 0;
    data.paths.audio[0] = 0;
    data.paths.speech[0] = 0;
    data.paths.background_music[0] = 0;
    rich_text_reset(0);
}

static int load_scenario_file(void)
{
    if (!game_campaign_is_active()) {
        return game_file_start_scenario_by_name(scenario_name());
    } else {
        return game_campaign_load_scenario(scenario_campaign_mission());
    }
}

static int has_briefing_message(void)
{
    if (game_campaign_is_original()) {
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
    if (game_campaign_is_original() || data.back_action == BUTTON_GO_BACK_NONE || data.video_played) {
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

static void fadeout_music(sound_type unused)
{
    sound_device_fadeout_music(5000);
    sound_device_on_audio_finished(0);
}

static void init_speech(sound_type type)
{
    if (type != SOUND_TYPE_SPEECH) {
        return;
    }

    int has_speech = *data.paths.speech && *data.paths.background_music;
    if (*data.paths.speech) {
        has_speech &= sound_device_play_file_on_channel(data.paths.speech,
            SOUND_TYPE_SPEECH, setting_sound(SOUND_TYPE_SPEECH)->volume);
    }
    if (*data.paths.background_music) {
        int volume = 100;
        if (has_speech) {
            volume = setting_sound(SOUND_TYPE_SPEECH)->volume / 3;
        }
        if (volume > setting_sound(SOUND_TYPE_MUSIC)->volume) {
            volume = setting_sound(SOUND_TYPE_MUSIC)->volume;
        }
        has_speech &= sound_device_play_music(data.paths.background_music, volume, 0);
    }
    sound_device_on_audio_finished(has_speech ? fadeout_music : 0);
}

static void play_audio(void)
{
    if (game_campaign_is_original() || data.audio_played) {
        data.audio_played = 1;
        return;
    }
    data.audio_played = 1;

    custom_message_t *custom_message = custom_messages_get(scenario_intro_message());

    const char *audio_file = custom_messages_get_audio(custom_message);
    if (audio_file) {
        snprintf(data.paths.audio, FILE_NAME_MAX, "%s", audio_file);
    }
    const char *speech_file = custom_messages_get_speech(custom_message);
    if (speech_file) {
        snprintf(data.paths.speech, FILE_NAME_MAX, "%s", speech_file);
    }
    const char *background_music = custom_messages_get_background_music(custom_message);
    if (background_music) {
        snprintf(data.paths.background_music, FILE_NAME_MAX, "%s", background_music);
    }

    if (!audio_file && !speech_file && !background_music) {
        data.audio_played = 0;
        return;
    }

    int playing_audio = 0;

    sound_music_stop();

    if (audio_file) {
        playing_audio = sound_device_play_file_on_channel(data.paths.audio, SOUND_TYPE_SPEECH,
            setting_sound(SOUND_TYPE_SPEECH)->volume);
    }
    if (speech_file) {
        if (!playing_audio) {
            init_speech(SOUND_TYPE_SPEECH);
        } else {
            sound_device_on_audio_finished(init_speech);
        }
    } else if (background_music) {
        sound_device_play_music(data.paths.background_music, setting_sound(SOUND_TYPE_MUSIC)->volume, 0);
    }
}

static void draw_background_image(void)
{
    if (game_campaign_is_original()) {
        window_draw_underlying_window();
        return;
    }
    if (!data.background_image_id) {
        int image_id = 0;
        if (has_briefing_message()) {
            custom_message_t *intro_message = custom_messages_get(scenario_intro_message());
            const uint8_t *background_image = custom_messages_get_background_image(intro_message);
            if (background_image) {
                image_id = rich_text_parse_image_id(&background_image, GROUP_INTERMEZZO_BACKGROUND, 1);
            }
        }
        if (!image_id) {
            image_id = image_group(GROUP_INTERMEZZO_BACKGROUND) + 2 * (scenario_campaign_mission() % 11) + 2;
        }
        data.background_image_id = image_id;
    }

    image_draw_fullscreen_background(data.background_image_id);
}

static void get_briefing_texts(const uint8_t **title, const uint8_t **subtitle, const uint8_t **content)
{
    if (game_campaign_is_original()) {
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

static int can_go_back(void)
{
    return data.back_action == BUTTON_GO_BACK_SCENARIO_SELECTION ||
        (data.back_action == BUTTON_GO_BACK_MISSION_SELECTION && game_mission_has_choice());
}

static void draw_background(void)
{
    if (!data.file_loaded) {
        if (!load_scenario_file()) {
            window_main_menu_show(1);
            setting_clear_personal_savings();
            scenario_settings_init();
            scenario_set_campaign_rank(2);
            window_plain_message_dialog_show(TR_WINDOW_CAMPAIGN_MISSION_FAILED_TO_LOAD_TITLE,
                TR_WINDOW_CAMPAIGN_MISSION_FAILED_TO_LOAD_TEXT, 0);
            return;
        }
        data.file_loaded = 1;
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
    if (can_go_back()) {
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
        rich_text_set_fonts(FONT_NORMAL_WHITE, FONT_NORMAL_GREEN, FONT_NORMAL_RED, 5);
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
    if (can_go_back()) {
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
    if (can_go_back()) {
        if (image_buttons_handle_mouse(m_dialog, 26, 428, &image_button_back, 1, 0)) {
            return;
        }
    }
}

static void button_back(int param1, int param2)
{
    if (data.back_action == BUTTON_GO_BACK_NONE) {
        return;
    }
    sound_music_stop();
    sound_speech_stop();
    if (data.back_action == BUTTON_GO_BACK_MISSION_SELECTION) {
        window_mission_selection_show_again();
    } else {
        window_mission_list_show_again();
        sound_music_play_intro();
    }
}

static void button_start_mission(int param1, int param2)
{
    if (data.back_action != BUTTON_GO_BACK_NONE || data.audio_played) {
        sound_music_stop();
        sound_speech_stop();
    }
    sound_music_update(1);
    window_city_show();
    if (data.back_action != BUTTON_GO_BACK_NONE) {
        city_mission_reset_save_start();
    }
    scenario_events_process_all();
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
    data.back_action = BUTTON_GO_BACK_MISSION_SELECTION;
    data.video_played = 0;
    data.file_loaded = 0;
    data.background_image_id = 0;
    game_campaign_is_original() ? window_intermezzo_show(INTERMEZZO_MISSION_BRIEFING, show) : show();
}

void window_mission_briefing_show_review(void)
{
    data.back_action = BUTTON_GO_BACK_NONE;
    data.file_loaded = 1;
    data.background_image_id = 0;
    game_campaign_is_original() ? window_intermezzo_show(INTERMEZZO_MISSION_BRIEFING, show) : show();
}

void window_mission_briefing_show_from_scenario_selection(void)
{
    data.back_action = BUTTON_GO_BACK_SCENARIO_SELECTION;
    data.video_played = 0;
    data.file_loaded = 0;
    data.background_image_id = 0;
    game_campaign_is_original() ? window_intermezzo_show(INTERMEZZO_MISSION_BRIEFING, show) : show();
}
