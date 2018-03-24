#include "music.h"

#include "Data/CityInfo.h"

#include "core/dir.h"
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
    TRACK_INTRO = 8
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

void sound_music_set_volume(int percentage)
{
    sound_device_set_music_volume(percentage);
}

static void play_track(int track)
{
    sound_device_stop_music();
    sound_device_play_music(dir_get_case_corrected_file(tracks[track]));
    sound_music_set_volume(setting_sound(SOUND_MUSIC)->volume);
    data.current_track = track;
}

void sound_music_play_intro()
{
    if (setting_sound(SOUND_MUSIC)->enabled) {
        play_track(TRACK_INTRO);
    }
}

void sound_music_reset()
{
    data.current_track = TRACK_NONE;
    data.next_check = 0;
}

void sound_music_update()
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
    if (Data_CityInfo.numEnemiesInCity + Data_CityInfo.numImperialSoldiersInCity >= 32) {
        track = TRACK_COMBAT_LONG;
    } else if (Data_CityInfo.numEnemiesInCity + Data_CityInfo.numImperialSoldiersInCity > 0) {
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

void sound_music_stop()
{
    sound_device_stop_music();
    data.current_track = TRACK_NONE;
}
