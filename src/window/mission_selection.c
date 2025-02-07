#include "mission_selection.h"

#include "assets/assets.h"
#include "core/image_group.h"
#include "core/lang.h"
#include "core/log.h"
#include "game/campaign.h"
#include "game/mission.h"
#include "game/settings.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/text.h"
#include "graphics/screen.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/property.h"
#include "sound/device.h"
#include "sound/music.h"
#include "sound/speech.h"
#include "window/mission_briefing.h"
#include "window/mission_list.h"
#include "window/video.h"

#include <stdlib.h>

#define BACKGROUND_WIDTH 1024
#define BACKGROUND_HEIGHT 768

static void button_start(int param1, int param2);
static void button_back(int param1, int param2);

static image_button image_buttons_start_mission[] = {
    { 0, 0, 39, 26, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 120, button_back, button_none, 1, 0, 1 },
    { 50, 0, 39, 26, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 116, button_start, button_none, 1, 0, 1 }
};

static struct {
    int choice;
    int focus_button;
    struct {
        const char *intro_video;
        int total_scenarios;
        int background_image_id;
        const uint8_t *title;
        const campaign_scenario **scenarios;
    } mission;
} data;

static void clear_loaded_mission(void)
{
    free(data.mission.scenarios);
    data.mission.scenarios = 0;
    data.mission.title = 0;
    data.mission.background_image_id = 0;
    data.mission.total_scenarios = 0;
    data.choice = 0;
}

static void load_scenarios(void)
{
    const campaign_mission_info *mission = game_campaign_get_current_mission(scenario_campaign_mission());
    data.mission.title = mission->title;
    data.mission.total_scenarios = mission->total_scenarios;
    data.mission.intro_video = mission->intro_video;
    if (mission->background_image.path) {
        data.mission.background_image_id = assets_get_external_image(mission->background_image.path, 0);
    } else {
        data.mission.background_image_id = mission->background_image.id;
    }
    data.mission.scenarios = malloc(sizeof(campaign_scenario *) * data.mission.total_scenarios);
    if (!data.mission.scenarios) {
        log_error("Failed to allocate memory for scenarios. The game will now crash.", 0, 0);
        return;
    }
    for (int i = 0; i < mission->total_scenarios; i++) {
        data.mission.scenarios[i] = game_campaign_get_scenario(i + mission->first_scenario);
    }
    scenario_set_custom(game_campaign_is_original() ? 0 : 2);
}

static void init(void)
{
    clear_loaded_mission();
    sound_music_stop();
    sound_speech_stop();
    if (!game_campaign_is_active()) {
        return;
    }
    load_scenarios();
}

static void draw_background_images(void)
{
    int s_width = screen_width();
    int s_height = screen_height();
    int image_offset_x = (s_width - BACKGROUND_WIDTH) / 2;
    int image_offset_y = (s_height - BACKGROUND_HEIGHT) / 2;

    if (s_width > BACKGROUND_WIDTH || s_height > BACKGROUND_HEIGHT) {
        image_draw_fullscreen_background(image_group(GROUP_EMPIRE_MAP));
        image_draw(image_group(GROUP_SELECT_MISSION_BACKGROUND), image_offset_x, image_offset_y,
            COLOR_MASK_NONE, SCALE_NONE);
        int image_border = assets_get_image_id("UI", "Mission Selection Border");
        image_draw_border(image_border, image_offset_x, image_offset_y, COLOR_MASK_NONE);
    } else {
        image_draw(image_group(GROUP_SELECT_MISSION_BACKGROUND), image_offset_x, image_offset_y,
            COLOR_MASK_NONE, SCALE_NONE);
    }
}

static void draw_background(void)
{
    draw_background_images();
    graphics_in_dialog();
    graphics_set_clip_rectangle(0, 0, 640, 400);
    if (data.mission.background_image_id) {
        image_draw(data.mission.background_image_id, 0, 0, COLOR_MASK_NONE, SCALE_NONE);
    } else {
        image_draw(image_group(GROUP_EMPIRE_MAP), 0, 0, COLOR_MASK_NONE, 2.5f);
    }
    graphics_reset_clip_rectangle();
    if (data.mission.title) {
        text_draw(data.mission.title, 20, 410, FONT_LARGE_BLACK, 0);
    }
    if (data.choice) {
        const campaign_scenario *scenario = data.mission.scenarios[data.choice - 1];
        if (scenario->name) {
            text_draw_multiline(scenario->name, 20, 440, 560, 0, FONT_NORMAL_BLACK, 0);
        }
        if (scenario->description) {
            text_draw_multiline(scenario->description, 20, 456, 560, 0, FONT_NORMAL_BLACK, 0);
        }
    } else {
        lang_text_draw_multiline(144, 0, 20, 440, 560, FONT_NORMAL_BLACK);
    }
    graphics_reset_dialog();
}

static int is_mouse_hit(const mouse *m, int x, int y, int size)
{
    return x <= m->x && m->x < x + size && y <= m->y && m->y < y + size;
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    image_buttons_draw(530, 410, image_buttons_start_mission, data.choice ? 2 : 1);

    int image_id = image_group(GROUP_SELECT_MISSION_BUTTON);

    for (int i = 0; i < data.mission.total_scenarios; i++) {
        int offset = data.choice == i + 1 ? 2 : 0;
        offset = data.focus_button == i + 1 ? 1 : offset;
        const campaign_scenario *scenario = data.mission.scenarios[i];
        image_draw(image_id + offset, scenario->x, scenario->y, COLOR_MASK_NONE, SCALE_NONE);
    }

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);

    data.focus_button = 0;
    for (int i = 0; i < data.mission.total_scenarios; i++) {
        const campaign_scenario *scenario = data.mission.scenarios[i];
        if (is_mouse_hit(m_dialog, scenario->x, scenario->y, 44)) {
            data.focus_button = i + 1;
        }
    }

    if (image_buttons_handle_mouse(m_dialog, 530, 410, image_buttons_start_mission, data.choice ? 2 : 1, 0)) {
        return;
    }

    if (input_go_back_requested(m, h)) {
        if (data.choice > 0) {
            data.choice = 0;
            window_invalidate();
        } else {
            button_back(0, 0);
        }
        return;
    }

    if (m_dialog->left.went_up) {
        for (int i = 0; i < data.mission.total_scenarios; i++) {
            const campaign_scenario *scenario = data.mission.scenarios[i];
            if (is_mouse_hit(m_dialog, scenario->x, scenario->y, 44)) {
                scenario_set_campaign_mission(scenario->id);
                data.choice = i + 1;
                if (m_dialog->left.double_click) {
                    button_start(0, 0);
                    return;
                }
                window_invalidate();
                if (scenario->fanfare) {
                    sound_device_play_file_on_channel(scenario->fanfare, SOUND_TYPE_SPEECH,
                        setting_sound(SOUND_TYPE_SPEECH)->volume);
                } else {
                    sound_speech_stop();
                }
                break;
            }
        }
    }
}

static void button_start(int param1, int param2)
{
    clear_loaded_mission();
    window_mission_briefing_show();
}

static void button_back(int param1, int param2)
{
    clear_loaded_mission();
    window_mission_list_show();
    sound_music_play_intro();
}

static void show(void)
{
    if (!game_mission_has_choice()) {
        window_mission_briefing_show();
        return;
    }
    window_type window = {
        WINDOW_MISSION_SELECTION,
        draw_background,
        draw_foreground,
        handle_input
    };
    window_show(&window);
}

void window_mission_selection_show(void)
{
    init();
    data.mission.intro_video ? window_video_show(data.mission.intro_video, show) : show();
}

void window_mission_selection_show_again(void)
{
    init();
    show();
}
