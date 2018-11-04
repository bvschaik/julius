#ifndef CITY_DATA_H
#define CITY_DATA_H

#include "core/buffer.h"

void city_data_init(void);

void city_data_init_scenario(void);

void city_data_init_campaign_mission(void);

void city_data_save_state(buffer *main, buffer *faction, buffer *faction_unknown, buffer *graph_order,
                          buffer *entry_exit_xy, buffer *entry_exit_grid_offset);

void city_data_load_state(buffer *main, buffer *faction, buffer *faction_unknown, buffer *graph_order,
                          buffer *entry_exit_xy, buffer *entry_exit_grid_offset);

#endif // CITY_DATA_H
