#include "music.h"

#include "core/dir.h"
#include "city/figures.h"
#include "city/population.h"
#include "game/settings.h"
#include "sound/device.h"

enum {
    TRACK_NONE = 0,
    TRACK_CITY_1 = 1,
    TRACK_CITY_2 = 2,
    TRACK_CITY_3 = 3,
    TRACK_CITY_4 = 4,
    TRACK_CITY_5 = 5,
    TRACK_COMBAT_SHORT = 6,
    TRACK_COMBAT_LONG = 7,
    TRACK_INTRO = 8,
    TRACK_MAX = 9
};

static struct {
    int current_track;
    int next_check;
} data = {TRACK_NONE, 0};

static const char tracks[][32] = {
    "",
    "wavs/ROME1.WAV",
    "wavs/ROME2.WAV",
    "wavs/ROME3.WAV",
    "wavs/ROME4.WAV",
    "wavs/ROME5.WAV",
    "wavs/Combat_Long.wav",
    "wavs/Combat_Short.wav",
    "wavs/setup.wav"
};

static const char mp3_tracks[][32] = {
    "",
    "mp3/ROME1.mp3",
    "mp3/ROME2.mp3",
    "mp3/ROME3.mp3",
    "mp3/ROME4.mp3",
    "mp3/ROME5.mp3",
    "mp3/Combat_Long.mp3",
    "mp3/Combat_Short.mp3",
    "mp3/setup.mp3"
};

void sound_music_set_volume(int percentage)
{
    sound_device_set_music_volume(percentage);
}

static void play_track(int track)
{
    sound_device_stop_music();
    if (track <= TRACK_NONE || track >= TRACK_MAX) {
        return;
    }
    const char* mp3_track = dir_get_case_corrected_file(mp3_tracks[track]);

    if (!mp3_track || !sound_device_play_music(mp3_track)) {
        sound_device_play_music(dir_get_case_corrected_file(tracks[track]));
    }
    sound_music_set_volume(setting_sound(SOUND_MUSIC)->volume);
    data.current_track = track;
}

void sound_music_play_intro(void)
{
    if (setting_sound(SOUND_MUSIC)->enabled) {
        play_track(TRACK_INTRO);
    }
}

void sound_music_reset(void)
{
    data.current_track = TRACK_NONE;
    data.next_check = 0;
}

void sound_music_update(void)
{
    if (data.next_check) {
        --data.next_check;
        return;
    }
    if (!setting_sound(SOUND_MUSIC)->enabled) {
        return;
    }
    int track;
    int population = city_population();
    int total_enemies = city_figures_total_invading_enemies();
    if (total_enemies >= 32) {
        track = TRACK_COMBAT_LONG;
    } else if (total_enemies > 0) {
        track = TRACK_COMBAT_SHORT;
    } else if (population < 1000) {
        track = TRACK_CITY_1;
    } else if (population < 2000) {
        track = TRACK_CITY_2;
    } else if (population < 5000) {
        track = TRACK_CITY_3;
    } else if (population < 7000) {
        track = TRACK_CITY_4;
    } else {
        track = TRACK_CITY_5;
    }

    if (track == data.current_track) {
        return;
    }

    play_track(track);
    data.next_check = 10;
}

void sound_music_stop(void)
{
    sound_device_stop_music();
    data.current_track = TRACK_NONE;
}
