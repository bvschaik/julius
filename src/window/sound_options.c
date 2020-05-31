#include "sound_options.h"

#include "game/settings.h"
#include "graphics/arrow_button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "sound/city.h"
#include "sound/effect.h"
#include "sound/music.h"
#include "sound/speech.h"

static void button_toggle(int type, int param2);
static void button_ok(int param1, int param2);
static void button_cancel(int param1, int param2);

static void arrow_button_music(int is_down, int param2);
static void arrow_button_speech(int is_down, int param2);
static void arrow_button_effects(int is_down, int param2);
static void arrow_button_city(int is_down, int param2);

static generic_button buttons[] = {
    {64, 162, 224, 20, button_toggle, button_none, SOUND_MUSIC, 0},
    {64, 192, 224, 20, button_toggle, button_none, SOUND_SPEECH, 0},
    {64, 222, 224, 20, button_toggle, button_none, SOUND_EFFECTS, 0},
    {64, 252, 224, 20, button_toggle, button_none, SOUND_CITY, 0},
    {144, 296, 192, 20, button_ok, button_none, 1, 0},
    {144, 326, 192, 20, button_cancel, button_none, 1, 0},
};

static arrow_button arrow_buttons[] = {
    {112, 100, 17, 24, arrow_button_music, 1, 0},
    {136, 100, 15, 24, arrow_button_music, 0, 0},
    {112, 130, 17, 24, arrow_button_speech, 1, 0},
    {136, 130, 15, 24, arrow_button_speech, 0, 0},
    {112, 160, 17, 24, arrow_button_effects, 1, 0},
    {136, 160, 15, 24, arrow_button_effects, 0, 0},
    {112, 190, 17, 24, arrow_button_city, 1, 0},
    {136, 190, 15, 24, arrow_button_city, 0, 0},
};

static struct {
    int focus_button_id;
    void (*close_callback)(void);

    set_sound original_effects;
    set_sound original_music;
    set_sound original_speech;
    set_sound original_city;
} data;

static void init(void (*close_callback)(void))
{
    data.focus_button_id = 0;
    data.close_callback = close_callback;

    data.original_effects = *setting_sound(SOUND_EFFECTS);
    data.original_music = *setting_sound(SOUND_MUSIC);
    data.original_speech = *setting_sound(SOUND_SPEECH);
    data.original_city = *setting_sound(SOUND_CITY);
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(48, 80, 24, 18);

    // on/off labels
    label_draw(64, 162, 14, data.focus_button_id == 1 ? 1 : 2);
    label_draw(64, 192, 14, data.focus_button_id == 2 ? 1 : 2);
    label_draw(64, 222, 14, data.focus_button_id == 3 ? 1 : 2);
    label_draw(64, 252, 14, data.focus_button_id == 4 ? 1 : 2);
    // ok/cancel labels
    label_draw(144, 296, 12, data.focus_button_id == 5 ? 1 : 2);
    label_draw(144, 326, 12, data.focus_button_id == 6 ? 1 : 2);

    // title
    lang_text_draw_centered(46, 0, 96, 92, 288, FONT_LARGE_BLACK);

    lang_text_draw_centered(46, 12, 128, 300, 224, FONT_NORMAL_GREEN);
    lang_text_draw_centered(46, 9, 128, 330, 224, FONT_NORMAL_GREEN);

    lang_text_draw(46, 10, 112, 142, FONT_SMALL_PLAIN);
    lang_text_draw(46, 11, 336, 142, FONT_SMALL_PLAIN);

    const set_sound *music = setting_sound(SOUND_MUSIC);
    lang_text_draw_centered(46, music->enabled ? 2 : 1, 64, 166, 224, FONT_NORMAL_GREEN);
    text_draw_percentage(music->volume, 374, 166, FONT_NORMAL_PLAIN);

    const set_sound *speech = setting_sound(SOUND_SPEECH);
    lang_text_draw_centered(46, speech->enabled ? 4 : 3, 64, 196, 224, FONT_NORMAL_GREEN);
    text_draw_percentage(speech->volume, 374, 196, FONT_NORMAL_PLAIN);

    const set_sound *effects = setting_sound(SOUND_EFFECTS);
    lang_text_draw_centered(46, effects->enabled ? 6 : 5, 64, 226, 224, FONT_NORMAL_GREEN);
    text_draw_percentage(effects->volume, 374, 226, FONT_NORMAL_PLAIN);

    const set_sound *city = setting_sound(SOUND_CITY);
    lang_text_draw_centered(46, city->enabled ? 8 : 7, 64, 256, 224, FONT_NORMAL_GREEN);
    text_draw_percentage(city->volume, 374, 256, FONT_NORMAL_PLAIN);

    arrow_buttons_draw(208, 60, arrow_buttons, 8);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, 6, &data.focus_button_id) ||
        arrow_buttons_handle_mouse(m_dialog, 208, 60, arrow_buttons, 8, 0)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        data.close_callback();
    }
}

static void button_toggle(int type, int param2)
{
    setting_toggle_sound_enabled(type);
    if (type == SOUND_MUSIC) {
        if (setting_sound(SOUND_MUSIC)->enabled) {
            sound_music_update(1);
        } else {
            sound_music_stop();
        }
    } else if (type == SOUND_SPEECH) {
        if (!setting_sound(SOUND_SPEECH)->enabled) {
            sound_speech_stop();
        }
    }
}

static void button_ok(int param1, int param2)
{
    data.close_callback();
}

static void button_cancel(int param1, int param2)
{
    setting_reset_sound(SOUND_EFFECTS, data.original_effects.enabled, data.original_effects.volume);
    setting_reset_sound(SOUND_MUSIC, data.original_music.enabled, data.original_music.volume);
    setting_reset_sound(SOUND_SPEECH, data.original_speech.enabled, data.original_speech.volume);
    setting_reset_sound(SOUND_CITY, data.original_city.enabled, data.original_city.volume);
    if (data.original_music.enabled) {
        if (setting_sound_is_enabled(SOUND_MUSIC) != data.original_music.enabled) {
            sound_music_update(1);
        }
    } else {
        sound_music_stop();
    }
    sound_music_set_volume(data.original_music.volume);
    sound_speech_set_volume(data.original_speech.volume);
    sound_effect_set_volume(data.original_effects.volume);
    sound_city_set_volume(data.original_city.volume);

    data.close_callback();
}

static void update_volume(set_sound_type type, int is_decrease)
{
    if (is_decrease) {
        setting_decrease_sound_volume(type);
    } else {
        setting_increase_sound_volume(type);
    }
}

static void arrow_button_music(int is_down, int param2)
{
    update_volume(SOUND_MUSIC, is_down);
    sound_music_set_volume(setting_sound(SOUND_MUSIC)->volume);
}

static void arrow_button_speech(int is_down, int param2)
{
    update_volume(SOUND_SPEECH, is_down);
    sound_speech_set_volume(setting_sound(SOUND_SPEECH)->volume);
}

static void arrow_button_effects(int is_down, int param2)
{
    update_volume(SOUND_EFFECTS, is_down);
    sound_effect_set_volume(setting_sound(SOUND_EFFECTS)->volume);
}

static void arrow_button_city(int is_down, int param2)
{
    update_volume(SOUND_CITY, is_down);
    sound_city_set_volume(setting_sound(SOUND_CITY)->volume);
}

void window_sound_options_show(void (*close_callback)(void))
{
    window_type window = {
        WINDOW_SOUND_OPTIONS,
        window_draw_underlying_window,
        draw_foreground,
        handle_input,
    };
    init(close_callback);
    window_show(&window);
}
