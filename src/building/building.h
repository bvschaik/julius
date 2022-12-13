#ifndef BUILDING_BUILDING_H
#define BUILDING_BUILDING_H

#include "building/type.h"
#include "core/buffer.h"
#include "core/time.h"

typedef struct building {
    int id;

    struct building *prev_of_type;
    struct building *next_of_type;

    time_millis last_update;

    unsigned char state;
    unsigned char faction_id;
    unsigned char unknown_value;
    unsigned char size;
    unsigned char house_is_merged;
    unsigned char house_size;
    unsigned char x;
    unsigned char y;
    short grid_offset;
    building_type type;
    union {
        short house_level;
        short warehouse_resource_id;
        short workshop_type;
        short orientation;
        short fort_figure_type;
        short native_meeting_center_id;
        short market_goods;
        short barracks_priority;
    } subtype;
    unsigned char road_network_id;
    unsigned short created_sequence;
    short houses_covered;
    short percentage_houses_covered;
    short house_population;
    short house_population_room;
    short distance_from_entry;
    short house_highest_population;
    short house_unreachable_ticks;
    unsigned char road_access_x;
    unsigned char road_access_y;
    short figure_id;
    short figure_id2; // labor seeker or market supplier
    short immigrant_figure_id;
    short figure_id4; // tower ballista, burning ruin prefect, doctor healing plague
    unsigned char figure_spawn_delay;
    unsigned char days_since_offering;
    unsigned char figure_roam_direction;
    unsigned char has_water_access;
    short prev_part_building_id;
    short next_part_building_id;
    short loads_stored;
    unsigned char house_sentiment_message;
    unsigned char has_well_access;
    short num_workers;
    unsigned char labor_category;
    unsigned char output_resource_id;
    unsigned char has_road_access;
    unsigned char house_criminal_active;
    short damage_risk;
    short fire_risk;
    short fire_duration;
    unsigned char fire_proof; // cannot catch fire or collapse
    unsigned char house_figure_generation_delay;
    unsigned char house_tax_coverage;
    unsigned char house_pantheon_access;
    short formation_id;
    signed char monthly_levy;
    union {
        struct {
            short queued_docker_id;
            unsigned char num_ships;
            signed char orientation;
            short docker_ids[3];
            short trade_ship_id;
            unsigned char has_accepted_route_ids;
            int accepted_route_ids;
        } dock;
        struct {
            short inventory[8];
            short pottery_demand;
            short furniture_demand;
            short oil_demand;
            short wine_demand;
            unsigned char fetch_inventory_id;
            unsigned char is_mess_hall;
        } market;
        struct {
            short resource_stored[16];
        } granary;
        struct {
            short progress;
            unsigned char blessing_days_left;
            unsigned char curse_days_left;
            unsigned char has_raw_materials;
            unsigned char has_fish;
            unsigned char is_stockpiling;
            unsigned char orientation;
            short fishing_boat_id;
            unsigned char age_months;
            unsigned char average_production_per_month;
            short production_current_month;
        } industry;
        struct {
            unsigned char num_shows;
            unsigned char days1;
            unsigned char days2;
            unsigned char play;
        } entertainment;
        struct {
            short inventory[8];
            unsigned char theater;
            unsigned char amphitheater_actor;
            unsigned char amphitheater_gladiator;
            unsigned char colosseum_gladiator;
            unsigned char colosseum_lion;
            unsigned char hippodrome;
            unsigned char school;
            unsigned char library;
            unsigned char academy;
            unsigned char barber;
            unsigned char clinic;
            unsigned char bathhouse;
            unsigned char hospital;
            unsigned char temple_ceres;
            unsigned char temple_neptune;
            unsigned char temple_mercury;
            unsigned char temple_mars;
            unsigned char temple_venus;
            unsigned char no_space_to_expand;
            unsigned char num_foods;
            unsigned char entertainment;
            unsigned char education;
            unsigned char health;
            unsigned char num_gods;
            unsigned char devolve_delay;
            unsigned char evolve_text_id;
        } house;
        struct {
            short resources_needed[16];
            int upgrades;
            short progress;
            short phase;
            short secondary_frame;
        } monument;
        struct {
            unsigned char was_tent;
        } rubble;
        struct {
            short exceptions;
        } roadblock;
        struct {
            short flag_frame;
        } warehouse;
    } data;
    int tax_income_or_storage;
    unsigned char house_days_without_food;
    unsigned char has_plague;
    signed char desirability;
    unsigned char is_deleted;
    unsigned char is_adjacent_to_water;
    unsigned char storage_id;
    union {
        signed char house_happiness;
        signed char native_anger;
    } sentiment;
    unsigned char show_on_problem_overlay;
    unsigned char house_tavern_wine_access;
    unsigned char house_tavern_meat_access;
    unsigned char house_arena_gladiator;
    unsigned char house_arena_lion;
    unsigned char is_tourism_venue;
    unsigned char tourism_disabled;
    unsigned char tourism_income;
    unsigned char tourism_income_this_year;
    unsigned char variant;
    unsigned char upgrade_level;
    unsigned char strike_duration_days;
    unsigned char sickness_level;
    unsigned char sickness_duration;
    unsigned char sickness_doctor_cure;
    unsigned char fumigation_frame;
    unsigned char fumigation_direction;
} building;

building *building_get(int id);

int building_dist(int x, int y, int w, int h, building *b);

void building_get_from_buffer(buffer *buf, int id, building *b, int includes_building_size, int save_version,
    int buffer_offset);

int building_count(void);

int building_find(building_type type);

building *building_first_of_type(building_type type);

void building_change_type(building *b, building_type type);

building *building_main(building *b);

building *building_next(building *b);

building *building_create(building_type type, int x, int y);

void building_clear_related_data(building *b);

building *building_restore_from_undo(building *to_restore);

void building_trim(void);

void building_update_state(void);

void building_update_desirability(void);

int building_is_house(building_type type);

int building_is_ceres_temple(building_type type);

int building_is_neptune_temple(building_type type);

int building_is_mercury_temple(building_type type);

int building_is_mars_temple(building_type type);

int building_is_venus_temple(building_type type);

int building_has_supplier_inventory(building_type type);

int building_is_statue_garden_temple(building_type type);

int building_is_fort(building_type type);

int building_is_primary_product_producer(building_type type);

int building_mothball_toggle(building *b);

int building_mothball_set(building *b, int value);

int building_get_tourism(const building *b);

int building_get_laborers(building_type type);

unsigned char building_stockpiling_toggle(building *b);

int building_get_tourism(const building* b);

int building_get_levy(const building *b);

void building_totals_add_corrupted_house(int unfixable);

void building_clear_all(void);

void building_save_state(buffer *buf, buffer *highest_id, buffer *highest_id_ever,
                         buffer *sequence, buffer *corrupt_houses);

void building_load_state(buffer *buf, buffer *sequence, buffer *corrupt_houses, int includes_building_size, int save_version);

#endif // BUILDING_BUILDING_H
