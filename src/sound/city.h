#ifndef SOUND_CITY_H
#define SOUND_CITY_H

#include "building/building.h"
#include "core/buffer.h"

enum {
    SOUND_DIRECTION_LEFT = 0,
    SOUND_DIRECTION_CENTER = 2,
    SOUND_DIRECTION_RIGHT = 4
};

void sound_city_init(void);

void sound_city_set_volume(int percentage);

void sound_city_mark_building_view(building_type type, int num_workers, int direction);

void sound_city_decay_views(void);

void sound_city_progress_ambient(void);

void sound_city_play(void);

void sound_city_save_state(buffer *buf);

void sound_city_load_state(buffer *buf);

#endif // SOUND_CITY_H
