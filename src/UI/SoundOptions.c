#include "AllWindows.h"

#include "core/calc.h"
#include "game/settings.h"
#include "graphics/arrow_button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "sound/city.h"
#include "sound/effect.h"
#include "sound/music.h"
#include "sound/speech.h"

static void buttonToggle(int type, int param2);
static void buttonOk(int param1, int param2);
static void buttonCancel(int param1, int param2);

static void arrowButtonMusic(int param1, int param2);
static void arrowButtonSpeech(int param1, int param2);
static void arrowButtonEffects(int param1, int param2);
static void arrowButtonCity(int param1, int param2);

static generic_button buttons[] = {
	{64, 162, 288, 182, GB_IMMEDIATE, buttonToggle, button_none, SOUND_MUSIC, 0},
	{64, 192, 288, 212, GB_IMMEDIATE, buttonToggle, button_none, SOUND_SPEECH, 0},
	{64, 222, 288, 242, GB_IMMEDIATE, buttonToggle, button_none, SOUND_EFFECTS, 0},
	{64, 252, 288, 272, GB_IMMEDIATE, buttonToggle, button_none, SOUND_CITY, 0},
	{144, 296, 336, 316, GB_IMMEDIATE, buttonOk, button_none, 1, 0},
	{144, 296, 336, 346, GB_IMMEDIATE, buttonCancel, button_none, 1, 0},
};

static arrow_button arrowButtons[] = {
	{112, 100, 17, 24, arrowButtonMusic, 1, 0},
	{136, 100, 15, 24, arrowButtonMusic, 0, 0},
	{112, 130, 17, 24, arrowButtonSpeech, 1, 0},
	{136, 130, 15, 24, arrowButtonSpeech, 0, 0},
	{112, 160, 17, 24, arrowButtonEffects, 1, 0},
	{136, 160, 15, 24, arrowButtonEffects, 0, 0},
	{112, 190, 17, 24, arrowButtonCity, 1, 0},
	{136, 190, 15, 24, arrowButtonCity, 0, 0},
};

static set_sound original_effects;
static set_sound original_music;
static set_sound original_speech;
static set_sound original_city;

static int focusButtonId;

void UI_SoundOptions_init()
{
	original_effects = *setting_sound(SOUND_EFFECTS);
	original_music = *setting_sound(SOUND_MUSIC);
	original_speech = *setting_sound(SOUND_SPEECH);
	original_city = *setting_sound(SOUND_CITY);
}

void UI_SoundOptions_drawForeground()
{
    graphics_in_dialog();
	
	outer_panel_draw(48, 80, 24, 18);
	
    // on/off labels
	label_draw(64, 162, 14, focusButtonId == 1 ? 1 : 2);
	label_draw(64, 192, 14, focusButtonId == 2 ? 1 : 2);
	label_draw(64, 222, 14, focusButtonId == 3 ? 1 : 2);
	label_draw(64, 252, 14, focusButtonId == 4 ? 1 : 2);
    // ok/cancel labels
	label_draw(144, 296, 12, focusButtonId == 5 ? 1 : 2);
	label_draw(144, 326, 12, focusButtonId == 6 ? 1 : 2);
	
    // title
	Widget_GameText_drawCentered(46, 0, 96, 92, 288, FONT_LARGE_BLACK);

	Widget_GameText_drawCentered(46, 12, 128, 300, 224, FONT_NORMAL_GREEN);
	Widget_GameText_drawCentered(46, 9, 128, 330, 224, FONT_NORMAL_GREEN);
	
	Widget_GameText_draw(46, 10, 112, 142, FONT_SMALL_PLAIN);
	Widget_GameText_draw(46, 11, 336, 142, FONT_SMALL_PLAIN);
	
	const set_sound *music = setting_sound(SOUND_MUSIC);
	Widget_GameText_drawCentered(46, music->enabled ? 2 : 1, 64, 166, 224, FONT_NORMAL_GREEN);
	Widget_Text_drawPercentage(music->volume, 374, 166, FONT_NORMAL_PLAIN);
	
	const set_sound *speech = setting_sound(SOUND_SPEECH);
	Widget_GameText_drawCentered(46, speech->enabled ? 4 : 3, 64, 196, 224, FONT_NORMAL_GREEN);
	Widget_Text_drawPercentage(speech->volume, 374, 196, FONT_NORMAL_PLAIN);
	
	const set_sound *effects = setting_sound(SOUND_EFFECTS);
	Widget_GameText_drawCentered(46, effects->enabled ? 6 : 5, 64, 226, 224, FONT_NORMAL_GREEN);
	Widget_Text_drawPercentage(effects->volume, 374, 226, FONT_NORMAL_PLAIN);
	
	const set_sound *city = setting_sound(SOUND_CITY);
	Widget_GameText_drawCentered(46, city->enabled ? 8 : 7, 64, 256, 224, FONT_NORMAL_GREEN);
	Widget_Text_drawPercentage(city->volume, 374, 256, FONT_NORMAL_PLAIN);

	arrow_buttons_draw(208, 60, arrowButtons, 8);

    graphics_reset_dialog();
}

void UI_SoundOptions_handleMouse(const mouse *m)
{
	if (m->right.went_up) {
		// cancel dialog
		UI_Window_goTo(Window_City);
	} else {
		const mouse *m_dialog = mouse_in_dialog(m);
		if (!generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, 6, &focusButtonId)) {
			arrow_buttons_handle_mouse(m_dialog, 208, 60, arrowButtons, 8);
		}
	}
}

static void buttonToggle(int type, int param2)
{
    setting_toggle_sound_enabled(type);
    switch (type) {
    case SOUND_MUSIC:
        if (setting_sound(SOUND_MUSIC)->enabled) {
            sound_music_reset();
            sound_music_update();
        } else {
            sound_music_stop();
        }
        break;
    case SOUND_SPEECH:
        if (!setting_sound(SOUND_SPEECH)->enabled) {
            sound_speech_stop();
        }
        break;
    }
}

static void buttonOk(int param1, int param2)
{
	UI_Window_goTo(Window_City);
}

static void buttonCancel(int param1, int param2)
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

	UI_Window_goTo(Window_City);
}

static void update_volume(set_sound_type type, int is_decrease)
{
	if (is_decrease) {
		setting_decrease_sound_volume(type);
	} else {
		setting_increase_sound_volume(type);
	}
}
static void arrowButtonMusic(int param1, int param2)
{
    update_volume(SOUND_MUSIC, param1);
    sound_music_set_volume(setting_sound(SOUND_MUSIC)->volume);

	UI_Window_requestRefresh();
}

static void arrowButtonSpeech(int param1, int param2)
{
    update_volume(SOUND_SPEECH, param1);
	sound_speech_set_volume(setting_sound(SOUND_SPEECH)->volume);

	UI_Window_requestRefresh();
}

static void arrowButtonEffects(int param1, int param2)
{
    update_volume(SOUND_EFFECTS, param1);
	sound_effect_set_volume(setting_sound(SOUND_EFFECTS)->volume);

	UI_Window_requestRefresh();
}

static void arrowButtonCity(int param1, int param2)
{
    update_volume(SOUND_CITY, param1);
    sound_city_set_volume(setting_sound(SOUND_CITY)->volume);

	UI_Window_requestRefresh();
}

