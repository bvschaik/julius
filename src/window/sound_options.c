#include "sound_options.h"

#include "game/settings.h"
#include "graphics/arrow_button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "sound/city.h"
#include "sound/effect.h"
#include "sound/music.h"
#include "sound/speech.h"
#include "window/city.h"

static void button_toggle(int type, int param2);
static void button_ok(int param1, int param2);
static void button_cancel(int param1, int param2);

static void arrow_button_music(int is_down, int param2);
static void arrow_button_speech(int is_down, int param2);
static void arrow_button_effects(int is_down, int param2);
static void arrow_button_city(int is_down, int param2);

static generic_button buttons[] = {
    {64, 162, 288, 182, GB_IMMEDIATE, button_toggle, button_none, SOUND_MUSIC, 0},
    {64, 192, 288, 212, GB_IMMEDIATE, button_toggle, button_none, SOUND_SPEECH, 0},
    {64, 222, 288, 242, GB_IMMEDIATE, button_toggle, button_none, SOUND_EFFECTS, 0},
    {64, 252, 288, 272, GB_IMMEDIATE, button_toggle, button_none, SOUND_CITY, 0},
    {144, 296, 336, 316, GB_IMMEDIATE, button_ok, button_none, 1, 0},
    {144, 296, 336, 346, GB_IMMEDIATE, button_cancel, button_none, 1, 0},
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

static set_sound original_effects;
static set_sound original_music;
static set_sound original_speech;
static set_sound original_city;

static int focus_button_id;

static void init(void)
{
    original_effects = *setting_sound(SOUND_EFFECTS);
    original_music = *setting_sound(SOUND_MUSIC);
    original_speech = *setting_sound(SOUND_SPEECH);
    original_city = *setting_sound(SOUND_CITY);
    focus_button_id = 0;
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    
    outer_panel_draw(48, 80, 24, 18);
    
    // on/off labels
    label_draw(64, 162, 14, focus_button_id == 1 ? 1 : 2);
    label_draw(64, 192, 14, focus_button_id == 2 ? 1 : 2);
    label_draw(64, 222, 14, focus_button_id == 3 ? 1 : 2);
    label_draw(64, 252, 14, focus_button_id == 4 ? 1 : 2);
    // ok/cancel labels
    label_draw(144, 296, 12, focus_button_id == 5 ? 1 : 2);
    label_draw(144, 326, 12, focus_button_id == 6 ? 1 : 2);
    
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

static void handle_mouse(const mouse *m)
{
    if (m->right.went_up) {
        // cancel dialog
        window_city_show();
    } else {
        const mouse *m_dialog = mouse_in_dialog(m);
        if (!generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, 6, &focus_button_id)) {
            arrow_buttons_handle_mouse(m_dialog, 208, 60, arrow_buttons, 8);
        }
    }
}

static void button_toggle(int type, int param2)
{
    setting_toggle_sound_enabled(type);
    if (type == SOUND_MUSIC) {
        if (setting_sound(SOUND_MUSIC)->enabled) {
            sound_music_reset();
            sound_music_update();
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
    window_city_show();
}

static void button_cancel(int param1, int param2)
{
    setting_reset_sound(SOUND_EFFECTS, original_effects.enabled, original_effects.volume);
    setting_reset_sound(SOUND_MUSIC, original_music.enabled, original_music.volume);
    setting_reset_sound(SOUND_SPEECH, original_speech.enabled, original_speech.volume);
    setting_reset_sound(SOUND_CITY, original_city.enabled, original_city.volume);
    if (original_music.enabled) {
        sound_music_reset();
        sound_music_update();
    } else {
        sound_music_stop();
    }
    sound_music_set_volume(original_music.volume);
    sound_speech_set_volume(original_speech.volume);
    sound_effect_set_volume(original_effects.volume);
    sound_city_set_volume(original_city.volume);

    window_city_show();
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

void window_sound_options_show(void)
{
    window_type window = {
        WINDOW_SOUND_OPTIONS,
        0,
        draw_foreground,
        handle_mouse,
    };
    init();
    window_show(&window);
}
