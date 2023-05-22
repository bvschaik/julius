#ifndef WINDOW_BUILDING_INFO_H
#define WINDOW_BUILDING_INFO_H

void window_building_info_show(int grid_offset);

int window_building_info_get_building_type(void);

void window_building_info_show_storage_orders(void);

void window_building_info_depot_select_source(void);

void window_building_info_depot_select_destination(void);

void window_building_info_depot_select_resource(void);

void window_building_info_depot_toggle_condition_type(void);

void window_building_info_depot_toggle_condition_threshold(void);

void window_building_info_depot_return_to_main_window(void);

#endif // WINDOW_BUILDING_INFO_H
