#ifndef SOUND_CITY_H
#define SOUND_CITY_H

#include "core/buffer.h"

void sound_city_init();

void sound_city_set_volume(int percentage);

void sound_city_mark_building_view(int building_id, int direction);

void sound_city_decay_views();

void sound_city_play();

void sound_city_save_state(buffer *buf);

void sound_city_load_state(buffer *buf);

#endif // SOUND_CITY_H
