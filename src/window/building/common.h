#ifndef WINDOW_BUILDING_COMMON_H
#define WINDOW_BUILDING_COMMON_H

static const int MIN_Y_POSITION = 32;
static const int MARGIN_POSITION = 16;

typedef enum {
    BUILDING_INFO_NONE = 0,
    BUILDING_INFO_TERRAIN = 1,
    BUILDING_INFO_BUILDING = 2,
    BUILDING_INFO_LEGION = 4
} building_info_type;

typedef enum {
    TERRAIN_INFO_NONE = 0,
    TERRAIN_INFO_TREE = 1,
    TERRAIN_INFO_ROCK = 2,
    TERRAIN_INFO_WATER = 3,
    TERRAIN_INFO_SHRUB = 4,
    TERRAIN_INFO_EARTHQUAKE = 5,
    TERRAIN_INFO_ROAD = 6,
    TERRAIN_INFO_AQUEDUCT = 7,
    TERRAIN_INFO_RUBBLE = 8,
    TERRAIN_INFO_WALL = 9,
    TERRAIN_INFO_EMPTY = 10,
    TERRAIN_INFO_BRIDGE = 11,
    TERRAIN_INFO_GARDEN = 12,
    TERRAIN_INFO_PLAZA = 13,
    TERRAIN_INFO_ENTRY_FLAG = 14,
    TERRAIN_INFO_EXIT_FLAG = 15,
    TERRAIN_INFO_HIGHWAY = 16,
} terrain_info_type;

typedef struct {
    int x_offset;
    int y_offset;
    int width_blocks;
    int height_blocks;
    int help_id;
    int can_play_sound;
    int building_id;
    int has_road_access;
    int worker_percentage;
    int has_reservoir_pipes;
    int aqueduct_has_water;
    int formation_id;
    int formation_types;
    int barracks_soldiers_requested;
    int worst_desirability_building_type;
    int warehouse_space_text;
    building_info_type type;
    terrain_info_type terrain_type;
    int can_go_to_advisor;
    int rubble_building_type;
    int storage_show_special_orders;
    struct {
        int sound_id;
        int phrase_id;
        int selected_index;
        int count;
        int drawn;
        int figure_ids[7];
    } figure;
} building_info_context;

void window_building_set_possible_position(int * x_offset, int * y_offset, int width_blocks, int height_blocks);

int window_building_get_vertical_offset(building_info_context *c, int new_window_height);

void window_building_draw_levy(int amount, int x_offset, int y_offset);

void window_building_draw_employment(building_info_context *c, int y_offset);

void window_building_draw_employment_without_house_cover(building_info_context *c, int y_offset);

void window_building_draw_description(building_info_context *c, int text_group, int text_id);

void window_building_draw_description_from_tr_string(building_info_context* c, int translation_key);

void window_building_draw_description_from_tr_string_at(building_info_context *c, int translation_key, int y_offset);

void window_building_draw_description_at(building_info_context *c, int y_offset, int text_group, int text_id);

void window_building_play_sound(building_info_context *c, const char *sound_file);

void window_building_draw_monument_construction_process(building_info_context *c,
                                                        int tr_phase_name, int tr_phase_name_text, int tr_construction_desc);
#endif // WINDOW_BUILDING_COMMON_H
