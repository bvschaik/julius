#ifndef SOUND_CHANNEL_H
#define SOUND_CHANNEL_H

enum {
    SOUND_CHANNEL_SPEECH = 0,

    // user interface effects
    SOUND_CHANNEL_EFFECTS_MIN = 1,
    SOUND_CHANNEL_EFFECTS_MAX = 44,

    // city sounds (from buildings)
    SOUND_CHANNEL_CITY_MIN = 45,
    SOUND_CHANNEL_CITY_MAX = 148,

    SOUND_CHANNEL_MAX = SOUND_CHANNEL_CITY_MAX + 1
};

#endif // SOUND_CHANNEL_H
