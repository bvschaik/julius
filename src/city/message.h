#ifndef CITY_MESSAGE_H
#define CITY_MESSAGE_H

#include "core/buffer.h"


void city_message_init_scenario();

int city_message_mark_population_shown(int population);


void city_message_save_state(buffer *population);

void city_message_load_state(buffer *population);

#endif // CITY_MESSAGE_H
