#include "settings.h"

#include "city/constants.h"
#include "core/buffer.h"
#include "core/calc.h"
#include "core/io.h"
#include "core/string.h"

#define INF_SIZE 560
#define MAX_PERSONAL_SAVINGS 100
#define MAX_PLAYER_NAME 32

static struct {
    // display settings
    int fullscreen;
    int window_width;
    int window_height;
    // sound settings
    set_sound sound_effects;
    set_sound sound_music;
    set_sound sound_speech;
    set_sound sound_city;
    // speed settings
    int game_speed;
    int scroll_speed;
    // misc settings
    set_difficulty difficulty;
    set_tooltips tooltips;
    int monthly_autosave;
    int warnings;
    int gods_enabled;
    int victory_video;
    // persistent game state
    int last_advisor;
    uint8_t player_name[MAX_PLAYER_NAME];
    // personal savings
    int personal_savings[MAX_PERSONAL_SAVINGS];
    // file data
    uint8_t inf_file[INF_SIZE];
} data;

static void load_default_settings(void)
{
    data.fullscreen = 1;
    data.window_width = 800;
    data.window_height = 600;

    data.sound_effects.enabled = 1;
    data.sound_effects.volume = 100;
    data.sound_music.enabled = 1;
    data.sound_music.volume = 80;
    data.sound_speech.enabled = 1;
    data.sound_speech.volume = 100;
    data.sound_city.enabled = 1;
    data.sound_city.volume = 100;

    data.game_speed = 90;
    data.scroll_speed = 70;

    data.difficulty = DIFFICULTY_NORMAL;
    data.tooltips = TOOLTIPS_FULL;
    data.warnings = 1;
    data.gods_enabled = 1;
    data.victory_video = 0;
    data.last_advisor = ADVISOR_LABOR;

    setting_clear_personal_savings();
}

static void load_settings(buffer *buf)
{
    buffer_skip(buf, 4);
    data.fullscreen = buffer_read_i32(buf);
    buffer_skip(buf, 3);
    data.sound_effects.enabled = buffer_read_u8(buf);
    data.sound_music.enabled = buffer_read_u8(buf);
    data.sound_speech.enabled = buffer_read_u8(buf);
    buffer_skip(buf, 6);
    data.game_speed = buffer_read_i32(buf);
    data.scroll_speed = buffer_read_i32(buf);
    buffer_read_raw(buf, data.player_name, MAX_PLAYER_NAME);
    buffer_skip(buf, 16);
    data.last_advisor = buffer_read_i32(buf);
    buffer_skip(buf, 4); //int save_game_mission_id;
    data.tooltips = buffer_read_i32(buf);
    buffer_skip(buf, 4); //int starting_favor;
    buffer_skip(buf, 4); //int personal_savings_last_mission;
    buffer_skip(buf, 4); //int current_mission_id;
    buffer_skip(buf, 4); //int is_custom_scenario;
    data.sound_city.enabled = buffer_read_u8(buf);
    data.warnings = buffer_read_u8(buf);
    data.monthly_autosave = buffer_read_u8(buf);
    buffer_skip(buf, 1); //unsigned char autoclear_enabled;
    data.sound_effects.volume = buffer_read_i32(buf);
    data.sound_music.volume = buffer_read_i32(buf);
    data.sound_speech.volume = buffer_read_i32(buf);
    data.sound_city.volume = buffer_read_i32(buf);
    buffer_skip(buf, 8); // ram
    data.window_width = buffer_read_i32(buf);
    data.window_height = buffer_read_i32(buf);
    buffer_skip(buf, 8); //int max_confirmed_resolution;
    for (int i = 0; i < MAX_PERSONAL_SAVINGS; i++) {
        data.personal_savings[i] = buffer_read_i32(buf);
    }
    data.victory_video = buffer_read_i32(buf);

    if (buffer_at_end(buf)) {
        // Settings file is from unpatched C3, use default values
        data.difficulty = DIFFICULTY_HARD;
        data.gods_enabled = 1;
    } else {
        data.difficulty = buffer_read_i32(buf);
        data.gods_enabled = buffer_read_i32(buf);
    }
}

void settings_load(void)
{
    load_default_settings();

    int size = io_read_file_into_buffer("c3.inf", NOT_LOCALIZED, data.inf_file, INF_SIZE);
    if (!size) {
        return;
    }

    buffer buf;
    buffer_init(&buf, data.inf_file, size);
    load_settings(&buf);

    if (data.window_width + data.window_height < 500) {
        // most likely migration from Caesar 3
        data.window_width = 800;
        data.window_height = 600;
    }
    if (data.last_advisor <= ADVISOR_NONE || data.last_advisor > ADVISOR_CHIEF) {
        data.last_advisor = ADVISOR_LABOR;
    }
}

void settings_save(void)
{
    buffer b;
    buffer *buf = &b;
    buffer_init(buf, data.inf_file, INF_SIZE);

    buffer_skip(buf, 4);
    buffer_write_i32(buf, data.fullscreen);
    buffer_skip(buf, 3);
    buffer_write_u8(buf, data.sound_effects.enabled);
    buffer_write_u8(buf, data.sound_music.enabled);
    buffer_write_u8(buf, data.sound_speech.enabled);
    buffer_skip(buf, 6);
    buffer_write_i32(buf, data.game_speed);
    buffer_write_i32(buf, data.scroll_speed);
    buffer_write_raw(buf, data.player_name, MAX_PLAYER_NAME);
    buffer_skip(buf, 16);
    buffer_write_i32(buf, data.last_advisor);
    buffer_skip(buf, 4); //int save_game_mission_id;
    buffer_write_i32(buf, data.tooltips);
    buffer_skip(buf, 4); //int starting_favor;
    buffer_skip(buf, 4); //int personal_savings_last_mission;
    buffer_skip(buf, 4); //int current_mission_id;
    buffer_skip(buf, 4); //int is_custom_scenario;
    buffer_write_u8(buf, data.sound_city.enabled);
    buffer_write_u8(buf, data.warnings);
    buffer_write_u8(buf, data.monthly_autosave);
    buffer_skip(buf, 1); //unsigned char autoclear_enabled;
    buffer_write_i32(buf, data.sound_effects.volume);
    buffer_write_i32(buf, data.sound_music.volume);
    buffer_write_i32(buf, data.sound_speech.volume);
    buffer_write_i32(buf, data.sound_city.volume);
    buffer_skip(buf, 8); // ram
    buffer_write_i32(buf, data.window_width);
    buffer_write_i32(buf, data.window_height);
    buffer_skip(buf, 8); //int max_confirmed_resolution;
    for (int i = 0; i < MAX_PERSONAL_SAVINGS; i++) {
        buffer_write_i32(buf, data.personal_savings[i]);
    }
    buffer_write_i32(buf, data.victory_video);
    buffer_write_i32(buf, data.difficulty);
    buffer_write_i32(buf, data.gods_enabled);

    io_write_buffer_to_file("c3.inf", data.inf_file, INF_SIZE);
}

int setting_fullscreen(void)
{
    return data.fullscreen;
}

void setting_window(int *width, int *height)
{
    *width = data.window_width;
    *height = data.window_height;
}

void setting_set_display(int fullscreen, int width, int height)
{
    data.fullscreen = fullscreen;
    if (!fullscreen) {
        data.window_width = width;
        data.window_height = height;
    }
}

static set_sound *get_sound(set_sound_type type)
{
    switch (type) {
    case SOUND_MUSIC: return &data.sound_music;
    case SOUND_EFFECTS: return &data.sound_effects;
    case SOUND_SPEECH: return &data.sound_speech;
    case SOUND_CITY: return &data.sound_city;
    default: return 0;
    }
}

const set_sound *setting_sound(set_sound_type type)
{
    return get_sound(type);
}

int setting_sound_is_enabled(set_sound_type type)
{
    return get_sound(type)->enabled;
}

void setting_toggle_sound_enabled(set_sound_type type)
{
    set_sound *sound = get_sound(type);
    sound->enabled = sound->enabled ? 0 : 1;
}

void setting_set_sound_volume(set_sound_type type, int volume)
{
    set_sound *sound = get_sound(type);
    sound->volume = calc_bound(volume, 0, 100);
}

void setting_reset_sound(set_sound_type type, int enabled, int volume)
{
    set_sound *sound = get_sound(type);
    sound->enabled = enabled;
    sound->volume = calc_bound(volume, 0, 100);
}

int setting_game_speed(void)
{
    return data.game_speed;
}

void setting_increase_game_speed(void)
{
    if (data.game_speed >= 100) {
        if (data.game_speed < 500) {
            data.game_speed += 100;
        }
    } else {
        data.game_speed = calc_bound(data.game_speed + 10, 10, 100);
    }
}

void setting_decrease_game_speed(void)
{
    if (data.game_speed > 100) {
        data.game_speed -= 100;
    } else {
        data.game_speed = calc_bound(data.game_speed - 10, 10, 100);
    }
}

void setting_set_default_game_speed(void)
{
    data.game_speed = 70;
}

int setting_scroll_speed(void)
{
    return data.scroll_speed;
}

void setting_increase_scroll_speed(void)
{
    data.scroll_speed = calc_bound(data.scroll_speed + 10, 0, 100);
}

void setting_decrease_scroll_speed(void)
{
    data.scroll_speed = calc_bound(data.scroll_speed - 10, 0, 100);
}

void setting_reset_speeds(int game_speed, int scroll_speed)
{
    data.game_speed = game_speed;
    data.scroll_speed = scroll_speed;
}

set_tooltips setting_tooltips(void)
{
    return data.tooltips;
}

void setting_cycle_tooltips(void)
{
    switch (data.tooltips) {
    case TOOLTIPS_NONE: data.tooltips = TOOLTIPS_SOME; break;
    case TOOLTIPS_SOME: data.tooltips = TOOLTIPS_FULL; break;
    default: data.tooltips = TOOLTIPS_NONE; break;
    }
}

int setting_warnings(void)
{
    return data.warnings;
}

void setting_toggle_warnings(void)
{
    data.warnings = data.warnings ? 0 : 1;
}

int setting_monthly_autosave(void)
{
    return data.monthly_autosave;
}

void setting_toggle_monthly_autosave(void)
{
    data.monthly_autosave = data.monthly_autosave ? 0 : 1;
}

int setting_gods_enabled(void)
{
    return data.gods_enabled;
}

void setting_toggle_gods_enabled(void)
{
    data.gods_enabled = data.gods_enabled ? 0 : 1;
}

set_difficulty setting_difficulty(void)
{
    return data.difficulty;
}

void setting_increase_difficulty(void)
{
    if (data.difficulty >= DIFFICULTY_VERY_HARD) {
        data.difficulty = DIFFICULTY_VERY_HARD;
    } else {
        data.difficulty++;
    }
}

void setting_decrease_difficulty(void)
{
    if (data.difficulty <= DIFFICULTY_VERY_EASY) {
        data.difficulty = DIFFICULTY_VERY_EASY;
    } else {
        data.difficulty--;
    }
}

int setting_victory_video(void)
{
    data.victory_video = data.victory_video ? 0 : 1;
    return data.victory_video;
}

int setting_last_advisor(void)
{
    return data.last_advisor;
}

void setting_set_last_advisor(int advisor)
{
    data.last_advisor = advisor;
}

const uint8_t *setting_player_name(void)
{
    return data.player_name;
}

void setting_set_player_name(const uint8_t *player_name)
{
    string_copy(player_name, data.player_name, MAX_PLAYER_NAME);
}

int setting_personal_savings_for_mission(int mission_id)
{
    return data.personal_savings[mission_id];
}

void setting_set_personal_savings_for_mission(int mission_id, int savings)
{
    data.personal_savings[mission_id] = savings;
}

void setting_clear_personal_savings(void)
{
    for (int i = 0; i < MAX_PERSONAL_SAVINGS; i++) {
        data.personal_savings[i] = 0;
    }
}
