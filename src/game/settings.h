#ifndef GAME_SETTINGS_H
#define GAME_SETTINGS_H


typedef enum {
    TOOLTIPS_NONE = 0,
    TOOLTIPS_SOME = 1,
    TOOLTIPS_FULL = 2
} set_tooltips;

typedef enum {
    DIFFICULTY_VERY_EASY = 0,
    DIFFICULTY_EASY = 1,
    DIFFICULTY_NORMAL = 2,
    DIFFICULTY_HARD = 3,
    DIFFICULTY_VERY_HARD = 4
} set_difficulty;

typedef enum {
	SOUND_MUSIC = 1,
	SOUND_SPEECH = 2,
	SOUND_EFFECTS = 3,
	SOUND_CITY = 4,
} set_sound_type;

typedef struct {
    int enabled;
    int volume;
} set_sound;

void settings_load();

void settings_save();

int setting_fullscreen();
void setting_window(int *width, int *height);
void setting_set_display(int fullscreen, int width, int height);

const set_sound *setting_sound(set_sound_type type);

void setting_toggle_sound_enabled(set_sound_type type);
void setting_increase_sound_volume(set_sound_type type);
void setting_decrease_sound_volume(set_sound_type type);
void setting_reset_sound(set_sound_type type, int enabled, int volume);

int setting_game_speed();
void setting_increase_game_speed();
void setting_decrease_game_speed();

int setting_scroll_speed();
void setting_increase_scroll_speed();
void setting_decrease_scroll_speed();
void setting_reset_speeds(int game_speed, int scroll_speed);

set_tooltips setting_tooltips();
void setting_cycle_tooltips();

int setting_warnings();
void setting_toggle_warnings();

int setting_monthly_autosave();
void setting_toggle_monthly_autosave();

int setting_gods_enabled();
void setting_toggle_gods_enabled();

set_difficulty setting_difficulty();
void setting_increase_difficulty();
void setting_decrease_difficulty();

int setting_victory_video();

int setting_last_advisor();
void setting_set_last_advisor(int advisor);

int setting_personal_savings_for_mission(int mission_id);
void setting_set_personal_savings_for_mission(int mission_id, int savings);
void setting_clear_personal_savings();

#endif // GAME_SETTINGS_H
