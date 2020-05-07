#include "intermezzo.h"

#include "core/time.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/screen.h"
#include "graphics/window.h"
#include "scenario/property.h"
#include "sound/music.h"
#include "sound/speech.h"

#define DISPLAY_TIME_MILLIS 1000

static const char SOUND_FILES_BRIEFING[][32] = {
    "wavs/01b.wav",
    "wavs/02b.wav",
    "wavs/03b.wav",
    "wavs/04b.wav",
    "wavs/05b.wav",
    "wavs/06b.wav",
    "wavs/07b.wav",
    "wavs/08b.wav",
    "wavs/09b.wav",
    "wavs/10b.wav",
    "wavs/11b.wav",
    "wavs/12b.wav",
    "wavs/13b.wav",
    "wavs/14b.wav",
    "wavs/15b.wav",
    "wavs/16b.wav",
    "wavs/17b.wav",
    "wavs/18b.wav",
    "wavs/19b.wav",
    "wavs/20b.wav",
    "wavs/21b.wav",
    "wavs/22b.wav",
};

static const char SOUND_FILES_WON[][32] = {
    "wavs/01w.wav",
    "wavs/02w.wav",
    "wavs/03w.wav",
    "wavs/04w.wav",
    "wavs/05w.wav",
    "wavs/06w.wav",
    "wavs/07w.wav",
    "wavs/08w.wav",
    "wavs/09w.wav",
    "wavs/10w.wav",
    "wavs/11w.wav",
    "wavs/12w.wav",
    "wavs/13w.wav",
    "wavs/14w.wav",
    "wavs/15w.wav",
    "wavs/16w.wav",
    "wavs/17w.wav",
    "wavs/18w.wav",
    "wavs/19w.wav",
    "wavs/20w.wav",
    "wavs/21w.wav",
    "wavs/22w.wav",
};

static const char SOUND_FILE_LOSE[] = "wavs/lose_game.wav";

static struct {
    intermezzo_type type;
    void (*callback)(void);
    time_millis start_time;
} data;

static void init(intermezzo_type type, void (*callback)(void))
{
    data.type = type;
    data.callback = callback;
    data.start_time = time_get_millis();
    sound_music_stop();
    sound_speech_stop();
    if (data.type == INTERMEZZO_FIRED) {
        sound_speech_play_file(SOUND_FILE_LOSE);
    } else if (!scenario_is_custom()) {
        int mission = scenario_campaign_mission();
        if (data.type == INTERMEZZO_MISSION_BRIEFING) {
            sound_speech_play_file(SOUND_FILES_BRIEFING[mission]);
        } else if (data.type == INTERMEZZO_WON) {
            sound_speech_play_file(SOUND_FILES_WON[mission]);
        }
    }
}

static void draw_background(void)
{
    graphics_clear_screens();
    int x_offset = (screen_width() - 1024) / 2;
    int y_offset = (screen_height() - 768) / 2;

    int mission = scenario_campaign_mission();
    int image_base = image_group(GROUP_INTERMEZZO_BACKGROUND);
    if (data.type == INTERMEZZO_MISSION_BRIEFING) {
        if (scenario_is_custom()) {
            image_draw(image_base + 1, x_offset, y_offset);
        } else {
            image_draw(image_base + 1 + 2 * mission, x_offset, y_offset);
        }
    } else if (data.type == INTERMEZZO_FIRED) {
        image_draw(image_base, x_offset, y_offset);
    } else if (data.type == INTERMEZZO_WON) {
        if (scenario_is_custom()) {
            image_draw(image_base + 2, x_offset, y_offset);
        } else {
            image_draw(image_base + 2 + 2 * mission, x_offset, y_offset);
        }
    }
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    time_millis current_time = time_get_millis();
    if (m->right.went_up || (m->is_touch && m->left.double_click) || current_time - data.start_time > DISPLAY_TIME_MILLIS) {
        data.callback();
    }
}

void window_intermezzo_show(intermezzo_type type, void (*callback)(void))
{
    window_type window = {
        WINDOW_INTERMEZZO,
        draw_background,
        0,
        handle_input
    };
    init(type, callback);
    window_show(&window);
}
