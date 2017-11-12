#ifndef CITY_MESSAGE_H
#define CITY_MESSAGE_H

#include "core/buffer.h"


typedef enum
{
    MESSAGE_CAT_RIOT = 0,
    MESSAGE_CAT_FIRE = 1,
    MESSAGE_CAT_COLLAPSE = 2,
    MESSAGE_CAT_RIOT_COLLAPSE = 3,
    MESSAGE_CAT_BLOCKED_DOCK = 4,
    MESSAGE_CAT_WORKERS_NEEDED = 8,
    MESSAGE_CAT_TUTORIAL3 = 9,
    MESSAGE_CAT_NO_WORKING_DOCK = 10,
    MESSAGE_CAT_FISHING_BLOCKED = 11,
} message_category;


void city_message_init_scenario();

void city_message_post_with_popup_delay(message_category category, int message_type, int param1, short param2);

void city_message_post_with_message_delay(message_category category, int use_popup, int message_type, int delay);

int city_message_get_category_count(message_category category);
void city_message_reset_category_count(message_category category);
void city_message_increase_category_count(message_category category);

void city_message_apply_sound_interval(message_category category);

void city_message_decrease_delays();

int city_message_mark_population_shown(int population);

void city_message_save_state(buffer *counts, buffer *delays, buffer *population);

void city_message_load_state(buffer *counts, buffer *delays, buffer *population);

#endif // CITY_MESSAGE_H
