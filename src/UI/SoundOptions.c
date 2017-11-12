#include "AllWindows.h"
#include "Window.h"
#include "../Sound.h"
#include "../Widget.h"

#include "../Data/Constants.h"
#include "../Data/Screen.h"
#include "../Data/Settings.h"

#include "core/calc.h"
#include "game/settings.h"
#include "sound/city.h"
#include "sound/effect.h"
#include "sound/music.h"
#include "sound/speech.h"

static void buttonToggle(int param1, int param2);
static void buttonOk(int param1, int param2);
static void buttonCancel(int param1, int param2);

static void arrowButtonMusic(int param1, int param2);
static void arrowButtonSpeech(int param1, int param2);
static void arrowButtonEffects(int param1, int param2);
static void arrowButtonCity(int param1, int param2);

static CustomButton buttons[] = {
	{64, 162, 288, 182, CustomButton_Immediate, buttonToggle, Widget_Button_doNothing, SOUND_MUSIC, 0},
	{64, 192, 288, 212, CustomButton_Immediate, buttonToggle, Widget_Button_doNothing, SOUND_SPEECH, 0},
	{64, 222, 288, 242, CustomButton_Immediate, buttonToggle, Widget_Button_doNothing, SOUND_EFFECTS, 0},
	{64, 252, 288, 272, CustomButton_Immediate, buttonToggle, Widget_Button_doNothing, SOUND_CITY, 0},
	{144, 296, 336, 316, CustomButton_Immediate, buttonOk, Widget_Button_doNothing, 1, 0},
	{144, 296, 336, 346, CustomButton_Immediate, buttonCancel, Widget_Button_doNothing, 1, 0},
};

static ArrowButton arrowButtons[] = {
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
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	
	Widget_Panel_drawOuterPanel(
		baseOffsetX + 48, baseOffsetY + 80,
		24, 18
	);
	
	Widget_Panel_drawSmallLabelButton(
		baseOffsetX + 64, baseOffsetY + 162,
		14, focusButtonId == 1 ? 1 : 2
	);
	Widget_Panel_drawSmallLabelButton(
		baseOffsetX + 64, baseOffsetY + 192,
		14, focusButtonId == 2 ? 1 : 2
	);
	Widget_Panel_drawSmallLabelButton(
		baseOffsetX + 64, baseOffsetY + 222,
		14, focusButtonId == 3 ? 1 : 2
	);
	Widget_Panel_drawSmallLabelButton(
		baseOffsetX + 64, baseOffsetY + 252,
		14, focusButtonId == 4 ? 1 : 2
	);
	Widget_Panel_drawSmallLabelButton(
		baseOffsetX + 144, baseOffsetY + 296,
		12, focusButtonId == 5 ? 1 : 2
	);
	Widget_Panel_drawSmallLabelButton(
		baseOffsetX + 144, baseOffsetY + 326,
		12, focusButtonId == 6 ? 1 : 2
	);
	
	Widget_GameText_drawCentered(46, 0,
		baseOffsetX + 96, baseOffsetY + 92,
		288, FONT_LARGE_BLACK
	);
	
	Widget_GameText_drawCentered(46, 12,
		baseOffsetX + 128, baseOffsetY + 300,
		224, FONT_NORMAL_GREEN
	);
	Widget_GameText_drawCentered(46, 9,
		baseOffsetX + 128, baseOffsetY + 330,
		224, FONT_NORMAL_GREEN
	);
	
	Widget_GameText_draw(46, 10,
		baseOffsetX + 112, baseOffsetY + 142,
		FONT_SMALL_PLAIN
	);
	Widget_GameText_draw(46, 11,
		baseOffsetX + 336, baseOffsetY + 142,
		FONT_SMALL_PLAIN
	);
	
	const set_sound *music = setting_sound(SOUND_MUSIC);
	Widget_GameText_drawCentered(46, music->enabled ? 2 : 1,
		baseOffsetX + 64, baseOffsetY + 166,
		224, FONT_NORMAL_GREEN
	);
	Widget_Text_drawPercentage(music->volume,
		baseOffsetX + 374, baseOffsetY + 166,
		FONT_NORMAL_PLAIN
	);
	
	const set_sound *speech = setting_sound(SOUND_SPEECH);
	Widget_GameText_drawCentered(46, speech->enabled ? 4 : 3,
		baseOffsetX + 64, baseOffsetY + 196,
		224, FONT_NORMAL_GREEN
	);
	Widget_Text_drawPercentage(speech->volume,
		baseOffsetX + 374, baseOffsetY + 196,
		FONT_NORMAL_PLAIN
	);
	
	const set_sound *effects = setting_sound(SOUND_EFFECTS);
	Widget_GameText_drawCentered(46, effects->enabled ? 6 : 5,
		baseOffsetX + 64, baseOffsetY + 226,
		224, FONT_NORMAL_GREEN
	);
	Widget_Text_drawPercentage(effects->volume,
		baseOffsetX + 374, baseOffsetY + 226,
		FONT_NORMAL_PLAIN
	);
	
	const set_sound *city = setting_sound(SOUND_CITY);
	Widget_GameText_drawCentered(46, city->enabled ? 8 : 7,
		baseOffsetX + 64, baseOffsetY + 256,
		224, FONT_NORMAL_GREEN
	);
	Widget_Text_drawPercentage(city->volume,
		baseOffsetX + 374, baseOffsetY + 256,
		FONT_NORMAL_PLAIN
	);

	Widget_Button_drawArrowButtons(
		baseOffsetX + 208, baseOffsetY + 60,
		arrowButtons, 8
	);
}

void UI_SoundOptions_handleMouse(const mouse *m)
{
	if (m->right.went_up) {
		// cancel dialog
		UI_Window_goTo(Window_City);
	} else {
		int baseOffsetX = Data_Screen.offset640x480.x;
		int baseOffsetY = Data_Screen.offset640x480.y;
		if (!Widget_Button_handleCustomButtons(
				baseOffsetX, baseOffsetY, buttons, 6, &focusButtonId)) {
			Widget_Button_handleArrowButtons(
				baseOffsetX + 208, baseOffsetY + 60, arrowButtons, 8);
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

