#include "building_state.h"
#include "building/industry.h"
#include "building/monument.h"
#include "building/roadblock.h"
#include "figure/figure.h"
#include "game/save_version.h"

#define TYPE_DATA_ORIGINAL_BUFFER_SIZE 42
#define TYPE_DATA_CURRENT_BUFFER_SIZE 26

static int is_industry_type(const building *b)
{
    return b->output_resource_id || b->type == BUILDING_NATIVE_CROPS
        || b->type == BUILDING_SHIPYARD || b->type == BUILDING_WHARF;
}

static void write_type_data(buffer *buf, const building *b)
{
    // This function should ALWAYS write 26 bytes.
    // If you don't write 26 bytes, the function will pad them at the end.
    // If you need more than 26 bytes, don't use the type data.
    int buffer_index = buf->index;

    if (building_is_house(b->type)) {
        buffer_write_u8(buf, b->data.house.theater);
        buffer_write_u8(buf, b->data.house.amphitheater_actor);
        buffer_write_u8(buf, b->data.house.amphitheater_gladiator);
        buffer_write_u8(buf, b->data.house.colosseum_gladiator);
        buffer_write_u8(buf, b->data.house.colosseum_lion);
        buffer_write_u8(buf, b->data.house.hippodrome);
        buffer_write_u8(buf, b->data.house.school);
        buffer_write_u8(buf, b->data.house.library);
        buffer_write_u8(buf, b->data.house.academy);
        buffer_write_u8(buf, b->data.house.barber);
        buffer_write_u8(buf, b->data.house.clinic);
        buffer_write_u8(buf, b->data.house.bathhouse);
        buffer_write_u8(buf, b->data.house.hospital);
        buffer_write_u8(buf, b->data.house.temple_ceres);
        buffer_write_u8(buf, b->data.house.temple_neptune);
        buffer_write_u8(buf, b->data.house.temple_mercury);
        buffer_write_u8(buf, b->data.house.temple_mars);
        buffer_write_u8(buf, b->data.house.temple_venus);
        buffer_write_u8(buf, b->data.house.no_space_to_expand);
        buffer_write_u8(buf, b->data.house.num_foods);
        buffer_write_u8(buf, b->data.house.entertainment);
        buffer_write_u8(buf, b->data.house.education);
        buffer_write_u8(buf, b->data.house.health);
        buffer_write_u8(buf, b->data.house.num_gods);
        buffer_write_u8(buf, b->data.house.devolve_delay);
        buffer_write_u8(buf, b->data.house.evolve_text_id);
    // Do not place this after if (building_has_supplier_inventory(b->type) or after if (building_monument_is_monument(b))
    // Because Caravanserai is monument AND supplier building and resources_needed / inventory is same memory spot
    } else if (b->type == BUILDING_CARAVANSERAI) {
        buffer_write_i32(buf, b->data.monument.upgrades);
        buffer_write_i16(buf, b->data.monument.progress);
        buffer_write_i16(buf, b->data.monument.phase);
        buffer_write_u8(buf, b->data.market.fetch_inventory_id);
        // As above, Ceres and Venus temples are both monuments and suppliers 
    } else if (b->type == BUILDING_LARGE_TEMPLE_CERES || b->type == BUILDING_LARGE_TEMPLE_VENUS) {
        buffer_write_i32(buf, b->data.monument.upgrades);
        buffer_write_i16(buf, b->data.monument.progress);
        buffer_write_i16(buf, b->data.monument.phase);
        buffer_write_u8(buf, b->data.market.fetch_inventory_id);
    } else if (building_has_supplier_inventory(b->type)) {
        buffer_write_u8(buf, b->data.market.fetch_inventory_id);
        buffer_write_u8(buf, b->data.market.is_mess_hall);
    } else if (building_monument_is_monument(b)) {
        buffer_write_i32(buf, b->data.monument.upgrades);
        buffer_write_i16(buf, b->data.monument.progress);
        buffer_write_i16(buf, b->data.monument.phase);
    } else if (b->type == BUILDING_DOCK) {
        buffer_write_i16(buf, b->data.dock.queued_docker_id);
        buffer_write_u8(buf, b->data.dock.has_accepted_route_ids);
        buffer_write_i32(buf, b->data.dock.accepted_route_ids);
        buffer_write_u8(buf, b->data.dock.num_ships);
        buffer_write_i8(buf, b->data.dock.orientation);
        for (int i = 0; i < 3; i++) {
            buffer_write_i16(buf, b->data.dock.docker_ids[i]);
        }
        buffer_write_i16(buf, b->data.dock.trade_ship_id);
    } else if (building_type_is_roadblock(b->type)) {
        buffer_write_u16(buf, b->data.roadblock.exceptions);
    } else if (is_industry_type(b)) {
        buffer_write_i16(buf, b->data.industry.progress);
        buffer_write_u8(buf, b->data.industry.is_stockpiling);
        buffer_write_u8(buf, b->data.industry.has_fish);
        buffer_write_u8(buf, b->data.industry.blessing_days_left);
        buffer_write_u8(buf, b->data.industry.orientation);
        buffer_write_u8(buf, b->data.industry.has_raw_materials);
        buffer_write_u8(buf, b->data.industry.curse_days_left);
        if ((b->type >= BUILDING_WHEAT_FARM && b->type <= BUILDING_POTTERY_WORKSHOP) || b->type == BUILDING_WHARF) {
            buffer_write_u8(buf, b->data.industry.age_months);
            buffer_write_u8(buf, b->data.industry.average_production_per_month);
            buffer_write_i16(buf, b->data.industry.production_current_month);
        }
        buffer_write_i16(buf, b->data.industry.fishing_boat_id);
    } else {
        buffer_write_u8(buf, b->data.entertainment.num_shows);
        buffer_write_u8(buf, b->data.entertainment.days1);
        buffer_write_u8(buf, b->data.entertainment.days2);
        buffer_write_u8(buf, b->data.entertainment.play);
    }
    int remaining_bytes = TYPE_DATA_CURRENT_BUFFER_SIZE - (buf->index - buffer_index);
    for (int i = 0; i < remaining_bytes; i++) {
        buffer_write_u8(buf, 0);
    }
}

void building_state_save_to_buffer(buffer *buf, const building *b)
{
    buffer_write_u8(buf, b->state);
    buffer_write_u8(buf, b->faction_id);
    buffer_write_u8(buf, b->unknown_value);
    buffer_write_u8(buf, b->size);
    buffer_write_u8(buf, b->house_is_merged);
    buffer_write_u8(buf, b->house_size);
    buffer_write_u8(buf, b->x);
    buffer_write_u8(buf, b->y);
    buffer_write_i16(buf, b->grid_offset);
    buffer_write_i16(buf, b->type);
    buffer_write_i16(buf, b->subtype.house_level); // which union field we use does not matter
    buffer_write_u8(buf, b->road_network_id);
    buffer_write_u8(buf, b->monthly_levy);
    buffer_write_u16(buf, b->created_sequence);
    buffer_write_i16(buf, b->houses_covered);
    buffer_write_i16(buf, b->percentage_houses_covered);
    buffer_write_i16(buf, b->house_population);
    buffer_write_i16(buf, b->house_population_room);
    buffer_write_i16(buf, b->distance_from_entry);
    buffer_write_i16(buf, b->house_highest_population);
    buffer_write_i16(buf, b->house_unreachable_ticks);
    buffer_write_u8(buf, b->road_access_x);
    buffer_write_u8(buf, b->road_access_y);
    buffer_write_i16(buf, b->figure_id);
    buffer_write_i16(buf, b->figure_id2);
    buffer_write_i16(buf, b->immigrant_figure_id);
    buffer_write_i16(buf, b->figure_id4);
    buffer_write_u8(buf, b->figure_spawn_delay);
    buffer_write_u8(buf, b->days_since_offering);
    buffer_write_u8(buf, b->figure_roam_direction);
    buffer_write_u8(buf, b->has_water_access);
    buffer_write_u8(buf, b->house_tavern_wine_access);
    buffer_write_u8(buf, b->house_tavern_food_access);
    buffer_write_i16(buf, b->prev_part_building_id);
    buffer_write_i16(buf, b->next_part_building_id);
    buffer_write_i16(buf, b->loads_stored);
    buffer_write_u8(buf, b->house_sentiment_message);
    buffer_write_u8(buf, b->has_well_access);
    buffer_write_i16(buf, b->num_workers);
    buffer_write_u8(buf, b->labor_category);
    buffer_write_u8(buf, b->output_resource_id);
    buffer_write_u8(buf, b->has_road_access);
    buffer_write_u8(buf, b->house_criminal_active);
    buffer_write_i16(buf, b->damage_risk);
    buffer_write_i16(buf, b->fire_risk);
    buffer_write_i16(buf, b->fire_duration);
    buffer_write_u8(buf, b->fire_proof);
    buffer_write_u8(buf, b->house_figure_generation_delay);
    buffer_write_u8(buf, b->house_tax_coverage);
    buffer_write_u8(buf, b->house_pantheon_access);
    buffer_write_i16(buf, b->formation_id);
    write_type_data(buf, b);
    buffer_write_i32(buf, b->tax_income_or_storage);
    buffer_write_u8(buf, b->house_days_without_food);
    buffer_write_u8(buf, b->has_plague);
    buffer_write_i8(buf, b->desirability);
    buffer_write_u8(buf, b->is_deleted);
    buffer_write_u8(buf, b->is_adjacent_to_water);
    buffer_write_u8(buf, b->storage_id);
    buffer_write_i8(buf, b->sentiment.house_happiness); // which union field we use does not matter
    buffer_write_u8(buf, b->show_on_problem_overlay);

    // expanded building data
    // Tourism
    buffer_write_u8(buf, b->house_arena_gladiator);
    buffer_write_u8(buf, b->house_arena_lion);
    buffer_write_u8(buf, b->is_tourism_venue);
    buffer_write_u8(buf, b->tourism_disabled);
    buffer_write_u8(buf, b->tourism_income);
    buffer_write_u8(buf, b->tourism_income_this_year);

    // Variants and upgrades
    buffer_write_u8(buf, b->variant);
    buffer_write_u8(buf, b->upgrade_level);

    //strikes
    buffer_write_u8(buf, b->strike_duration_days);

    // sickness
    buffer_write_u8(buf, b->sickness_level);
    buffer_write_u8(buf, b->sickness_duration);
    buffer_write_u8(buf, b->sickness_doctor_cure);
    buffer_write_u8(buf, b->fumigation_frame);
    buffer_write_u8(buf, b->fumigation_direction);

    // extra resources
    for (int i = 0; i < RESOURCE_MAX; i++) {
        buffer_write_i16(buf, b->resources[i]);
    }

    // accepted goods
    for (int i = 0; i < RESOURCE_MAX; i++) {
        buffer_write_u8(buf, b->accepted_goods[i]);
    }

    // New building state code should always be added at the end to preserve savegame retrocompatibility
    // Also, don't forget to update BUILDING_STATE_CURRENT_BUFFER_SIZE and if possible, add a new macro like
    // BUILDING_STATE_NEW_FEATURE_BUFFER_SIZE with the full building state buffer size including all added features
    // up until that point in Augustus' development
}

static void read_type_data(buffer *buf, building *b, int version)
{
    // This function should ALWAYS read 42 bytes for versions before or at SAVE_GAME_LAST_STATIC_RESOURCES.
    // The only exception is for Caravanserai on old savegame versions, which due to an oversight only read 41 bytes.
    // For versions after SAVE_GAME_LAST_STATIC_RESOURCES, the function should ALWAYS read 26 bytes.
    // If you don't need to read all bytes, they will be automatically skipped at the end.
    int type_data_bytes;
    if (version <= SAVE_GAME_LAST_STATIC_RESOURCES) {
        type_data_bytes = TYPE_DATA_ORIGINAL_BUFFER_SIZE;

        // Old savegame versions had a bug where the caravanserai's building type data size was off by 1
        // Old save versions don't need to skip the byte, while new save versions do
        if (b->type == BUILDING_CARAVANSERAI && version <= SAVE_GAME_LAST_CARAVANSERAI_WRONG_OFFSET) {
            type_data_bytes -= 1;
        }
    } else {
        type_data_bytes = TYPE_DATA_CURRENT_BUFFER_SIZE;
    }
    int buffer_index = buf->index;

    if (building_is_house(b->type)) {
        if (version <= SAVE_GAME_LAST_STATIC_RESOURCES) {
            for (int i = 0; i < LEGACY_INVENTORY_MAX; i++) {
                b->resources[resource_map_legacy_inventory(i)] = buffer_read_i16(buf);
            }
        }
        b->data.house.theater = buffer_read_u8(buf);
        b->data.house.amphitheater_actor = buffer_read_u8(buf);
        b->data.house.amphitheater_gladiator = buffer_read_u8(buf);
        b->data.house.colosseum_gladiator = buffer_read_u8(buf);
        b->data.house.colosseum_lion = buffer_read_u8(buf);
        b->data.house.hippodrome = buffer_read_u8(buf);
        b->data.house.school = buffer_read_u8(buf);
        b->data.house.library = buffer_read_u8(buf);
        b->data.house.academy = buffer_read_u8(buf);
        b->data.house.barber = buffer_read_u8(buf);
        b->data.house.clinic = buffer_read_u8(buf);
        b->data.house.bathhouse = buffer_read_u8(buf);
        b->data.house.hospital = buffer_read_u8(buf);
        b->data.house.temple_ceres = buffer_read_u8(buf);
        b->data.house.temple_neptune = buffer_read_u8(buf);
        b->data.house.temple_mercury = buffer_read_u8(buf);
        b->data.house.temple_mars = buffer_read_u8(buf);
        b->data.house.temple_venus = buffer_read_u8(buf);
        b->data.house.no_space_to_expand = buffer_read_u8(buf);
        b->data.house.num_foods = buffer_read_u8(buf);
        b->data.house.entertainment = buffer_read_u8(buf);
        b->data.house.education = buffer_read_u8(buf);
        b->data.house.health = buffer_read_u8(buf);
        b->data.house.num_gods = buffer_read_u8(buf);
        b->data.house.devolve_delay = buffer_read_u8(buf);
        b->data.house.evolve_text_id = buffer_read_u8(buf);
    // Do not place this after if (building_has_supplier_inventory(b->type) or after if (building_monument_is_monument(b))
    // Because Caravanserai is monument AND supplier building and resources_needed / inventory is same memory spot
    } else if (b->type == BUILDING_CARAVANSERAI) {
        if (version <= SAVE_GAME_LAST_STATIC_RESOURCES) {
            for (int i = 0; i < RESOURCE_MAX_LEGACY; i++) {
                b->resources[resource_remap(i)] = buffer_read_i16(buf);
            }
        }
        b->data.monument.upgrades = buffer_read_i32(buf);
        b->data.monument.progress = buffer_read_i16(buf);
        b->data.monument.phase = buffer_read_i16(buf);
        b->data.market.fetch_inventory_id = resource_map_legacy_inventory(buffer_read_u8(buf));
        // As above, Ceres and Venus temples are both monuments and suppliers 
    } else if (b->type == BUILDING_LARGE_TEMPLE_CERES || b->type == BUILDING_LARGE_TEMPLE_VENUS) {
        if (version <= SAVE_GAME_LAST_STATIC_RESOURCES) {
            for (int i = 0; i < RESOURCE_MAX_LEGACY; i++) {
                b->resources[resource_remap(i)] = buffer_read_i16(buf);
            }
        }
        b->data.monument.upgrades = buffer_read_i32(buf);
        b->data.monument.progress = buffer_read_i16(buf);
        b->data.monument.phase = buffer_read_i16(buf);
        if (!b->data.monument.phase) { // Compatibility fix
            b->data.monument.phase = MONUMENT_FINISHED;
        }
        b->data.market.fetch_inventory_id = resource_map_legacy_inventory(buffer_read_u8(buf));
    } else if (building_has_supplier_inventory(b->type)) {
        if (version <= SAVE_GAME_LAST_STATIC_RESOURCES) {
            buffer_skip(buf, 2);
            for (int i = 0; i < LEGACY_INVENTORY_MAX; i++) {
                b->resources[resource_map_legacy_inventory(i)] = buffer_read_i16(buf);
            }
            int pottery_demand = buffer_read_i16(buf);
            if (b->accepted_goods[RESOURCE_POTTERY]) {
                b->accepted_goods[RESOURCE_POTTERY] += pottery_demand;
            }
            int furniture_demand = buffer_read_i16(buf);
            if (b->accepted_goods[RESOURCE_FURNITURE]) {
                b->accepted_goods[RESOURCE_FURNITURE] += furniture_demand;
            }
            int oil_demand = buffer_read_i16(buf);
            if (b->accepted_goods[RESOURCE_OIL]) {
                b->accepted_goods[RESOURCE_OIL] += oil_demand;
            }
            int wine_demand = buffer_read_i16(buf);
            if (b->accepted_goods[RESOURCE_WINE]) {
                b->accepted_goods[RESOURCE_WINE] += wine_demand;
            }
        }
        b->data.market.fetch_inventory_id = resource_map_legacy_inventory(buffer_read_u8(buf));
        b->data.market.is_mess_hall = buffer_read_u8(buf);
    } else if (b->type == BUILDING_GRANARY) {
        if (version <= SAVE_GAME_LAST_STATIC_RESOURCES) {
            buffer_skip(buf, 2);
            for (int i = 0; i < RESOURCE_MAX_LEGACY; i++) {
                b->resources[resource_remap(i)] = buffer_read_i16(buf);
            }
        }
    } else if (building_monument_is_monument(b)) {
        if (version <= SAVE_GAME_LAST_STATIC_RESOURCES) {
            for (int i = 0; i < RESOURCE_MAX_LEGACY; i++) {
                b->resources[resource_remap(i)] = buffer_read_i16(buf);
            }
            if (b->resources[RESOURCE_NONE] < 0) {
                b->resources[RESOURCE_NONE] = 1;
            }
        }
        b->data.monument.upgrades = buffer_read_i32(buf);
        b->data.monument.progress = buffer_read_i16(buf);
        b->data.monument.phase = buffer_read_i16(buf);
    } else if (b->type == BUILDING_DOCK) {
        b->data.dock.queued_docker_id = buffer_read_i16(buf);
        b->data.dock.has_accepted_route_ids = buffer_read_u8(buf);
        b->data.dock.accepted_route_ids = buffer_read_i32(buf);
        if (version <= SAVE_GAME_LAST_STATIC_RESOURCES) {
            buffer_skip(buf, 20);
        }
        b->data.dock.num_ships = buffer_read_u8(buf);
        if (version <= SAVE_GAME_LAST_STATIC_RESOURCES) {
            buffer_skip(buf, 2);
        }
        b->data.dock.orientation = buffer_read_i8(buf);
        if (version <= SAVE_GAME_LAST_STATIC_RESOURCES) {
            buffer_skip(buf, 3);
        }
        for (int i = 0; i < 3; i++) {
            b->data.dock.docker_ids[i] = buffer_read_i16(buf);
        }
        b->data.dock.trade_ship_id = buffer_read_i16(buf);
    } else if (building_type_is_roadblock(b->type)) {
        b->data.roadblock.exceptions = buffer_read_i16(buf);
    } else if (is_industry_type(b)) {
        b->data.industry.progress = buffer_read_i16(buf);
        if (version <= SAVE_GAME_LAST_STATIC_RESOURCES) {
            buffer_skip(buf, 11);
        }
        b->data.industry.is_stockpiling = buffer_read_u8(buf);
        b->data.industry.has_fish = buffer_read_u8(buf);
        if (version <= SAVE_GAME_LAST_STATIC_RESOURCES) {
            buffer_skip(buf, 14);
        }
        b->data.industry.blessing_days_left = buffer_read_u8(buf);
        b->data.industry.orientation = buffer_read_u8(buf);
        b->data.industry.has_raw_materials = buffer_read_u8(buf);
        if (version <= SAVE_GAME_LAST_STATIC_RESOURCES) {
            buffer_skip(buf, 1);
        }
        b->data.industry.curse_days_left = buffer_read_u8(buf);
        if ((b->type >= BUILDING_WHEAT_FARM && b->type <= BUILDING_POTTERY_WORKSHOP) || b->type == BUILDING_WHARF) {
            b->data.industry.age_months = buffer_read_u8(buf);
            b->data.industry.average_production_per_month = buffer_read_u8(buf);
            b->data.industry.production_current_month = buffer_read_i16(buf);
            if (version <= SAVE_GAME_LAST_STATIC_RESOURCES) {
                buffer_skip(buf, 2);
            }
        } else if (version <= SAVE_GAME_LAST_STATIC_RESOURCES) {
            buffer_skip(buf, 6);
        }
        b->data.industry.fishing_boat_id = buffer_read_i16(buf);
    } else {
        if (version <= SAVE_GAME_LAST_STATIC_RESOURCES) {
            buffer_skip(buf, 26);
        }
        b->data.entertainment.num_shows = buffer_read_u8(buf);
        b->data.entertainment.days1 = buffer_read_u8(buf);
        b->data.entertainment.days2 = buffer_read_u8(buf);
        b->data.entertainment.play = buffer_read_u8(buf);
    }
    int remaining_bytes = type_data_bytes - (buf->index - buffer_index);
    if (remaining_bytes > 0) {
        buffer_skip(buf, remaining_bytes);
    }
}

static void migrate_accepted_goods(building *b, int permissions)
{
    int max = b->type == BUILDING_DOCK ? RESOURCE_MAX_LEGACY : LEGACY_INVENTORY_MAX;
    for (int i = 0; i < max; i++) {
        int goods_bit = 1 << i;
        int id = b->type == BUILDING_DOCK ? resource_remap(i) : resource_map_legacy_inventory(i);
        b->accepted_goods[id] = !(permissions & goods_bit);
    }
}

void building_state_load_from_buffer(buffer *buf, building *b, int building_buf_size, int save_version, int for_preview)
{
    b->state = buffer_read_u8(buf);
    b->faction_id = buffer_read_u8(buf);
    b->unknown_value = buffer_read_u8(buf);
    b->size = buffer_read_u8(buf);
    b->house_is_merged = buffer_read_u8(buf);
    b->house_size = buffer_read_u8(buf);
    b->x = buffer_read_u8(buf);
    b->y = buffer_read_u8(buf);
    b->grid_offset = buffer_read_i16(buf);
    b->type = buffer_read_i16(buf);
    if (b->type == BUILDING_WAREHOUSE_SPACE) {
        b->subtype.warehouse_resource_id = resource_remap(buffer_read_i16(buf));
    } else if (save_version <= SAVE_GAME_LAST_STATIC_RESOURCES &&
        (b->type == BUILDING_DOCK || building_has_supplier_inventory(b->type))) {
        migrate_accepted_goods(b, buffer_read_i16(buf));
    } else {
        b->subtype.house_level = buffer_read_i16(buf); // which union field we use does not matter        
    }
    b->road_network_id = buffer_read_u8(buf);
    b->monthly_levy = buffer_read_u8(buf);
    b->created_sequence = buffer_read_u16(buf);
    b->houses_covered = buffer_read_i16(buf);
    b->percentage_houses_covered = buffer_read_i16(buf);
    b->house_population = buffer_read_i16(buf);
    b->house_population_room = buffer_read_i16(buf);
    b->distance_from_entry = buffer_read_i16(buf);
    b->house_highest_population = buffer_read_i16(buf);
    b->house_unreachable_ticks = buffer_read_i16(buf);
    b->road_access_x = buffer_read_u8(buf);
    b->road_access_y = buffer_read_u8(buf);
    b->figure_id = buffer_read_i16(buf);
    b->figure_id2 = buffer_read_i16(buf);
    b->immigrant_figure_id = buffer_read_i16(buf);
    b->figure_id4 = buffer_read_i16(buf);
    b->figure_spawn_delay = buffer_read_u8(buf);
    b->days_since_offering = buffer_read_u8(buf);
    b->figure_roam_direction = buffer_read_u8(buf);
    b->has_water_access = buffer_read_u8(buf);
    b->house_tavern_wine_access = buffer_read_u8(buf);
    b->house_tavern_food_access = buffer_read_u8(buf);
    b->prev_part_building_id = buffer_read_i16(buf);
    b->next_part_building_id = buffer_read_i16(buf);
    b->loads_stored = buffer_read_i16(buf);
    b->house_sentiment_message = buffer_read_u8(buf);
    b->has_well_access = buffer_read_u8(buf);
    b->num_workers = buffer_read_i16(buf);
    b->labor_category = buffer_read_u8(buf);
    b->output_resource_id = resource_remap(buffer_read_u8(buf));
    b->has_road_access = buffer_read_u8(buf);
    b->house_criminal_active = buffer_read_u8(buf);
    b->damage_risk = buffer_read_i16(buf);
    b->fire_risk = buffer_read_i16(buf);
    b->fire_duration = buffer_read_i16(buf);
    b->fire_proof = buffer_read_u8(buf);
    b->house_figure_generation_delay = buffer_read_u8(buf);
    b->house_tax_coverage = buffer_read_u8(buf);
    b->house_pantheon_access = buffer_read_u8(buf);
    b->formation_id = buffer_read_i16(buf);
    read_type_data(buf, b, save_version);
    b->tax_income_or_storage = buffer_read_i32(buf);
    b->house_days_without_food = buffer_read_u8(buf);
    b->has_plague = buffer_read_u8(buf);
    b->desirability = buffer_read_i8(buf);
    b->is_deleted = buffer_read_u8(buf);
    b->is_adjacent_to_water = buffer_read_u8(buf);
    b->storage_id = buffer_read_u8(buf);
    b->sentiment.house_happiness = buffer_read_i8(buf); // which union field we use does not matter
    b->show_on_problem_overlay = buffer_read_u8(buf);

    // Wharves produce fish and don't need any progress
    if (b->type == BUILDING_WHARF) {
        b->output_resource_id = RESOURCE_FISH;
        b->data.industry.progress = 0;
    }

    if (building_buf_size < BUILDING_STATE_STRIKES) {
        // Backwards compatibility fixes for sentiment update
        if (b->house_population && b->sentiment.house_happiness < 20) {
            b->sentiment.house_happiness = 30;
        }

        // Backwards compatibility fixes for culture update
        if (building_monument_is_monument(b) && b->subtype.house_level && b->type != BUILDING_HIPPODROME && b->type <= BUILDING_LIGHTHOUSE) {
            b->data.monument.phase = b->subtype.house_level;
        }

        if ((b->type == BUILDING_HIPPODROME || b->type == BUILDING_COLOSSEUM) && !b->data.monument.phase) {
            b->data.monument.phase = -1;
        }

        if (((b->type >= BUILDING_LARGE_TEMPLE_CERES && b->type <= BUILDING_LARGE_TEMPLE_VENUS) || b->type == BUILDING_ORACLE) && !b->data.monument.phase) {
            b->data.monument.phase = -1;
        }

    }

    if (save_version < SAVE_GAME_ROADBLOCK_DATA_MOVED_FROM_SUBTYPE) {
        // Backwards compatibility - roadblock data used to be stored in b->subtype 
        if (building_type_is_roadblock(b->type)) {
            b->data.roadblock.exceptions = b->subtype.orientation;
        }
    }

    // Backwards compatibility - double the current progress of industry buildings, except for wheat farms
    if (save_version < SAVE_GAME_LAST_NO_GOLD_AND_MINTING && b->output_resource_id && b->type != BUILDING_WHEAT_FARM) {
        b->data.industry.progress *= 2;
    }

    // To keep backward savegame compatibility, only fill more recent building struct elements
    // if building_buf_size is the correct size when those elements are included
    // For example, if you add an int (4 bytes) to the building state struct, in order to check
    // if the samegame version has that new int, you should add the folloging code:
    // if (building_buf_size >= BULDING_STATE_ORIGINAL_BUFFER_SIZE + 4) {
    //    b->new_var = buffer_read_i32(buf);
    // }
    // Or even better:
    // if (building_buf_size >= BULDING_STATE_NEW_FEATURE_BUFFER_SIZE) {
    //    b->new_var = buffer_read_i32(buf);
    // }
    // Building state variables are automatically set to 0, so if the savegame version doesn't include
    // that information, you can be assured that the game will read it as 0

    if (building_buf_size >= BUILDING_STATE_TOURISM_BUFFER_SIZE) {
        b->house_arena_gladiator = buffer_read_u8(buf);
        b->house_arena_lion = buffer_read_u8(buf);
        b->is_tourism_venue = buffer_read_u8(buf);
        b->tourism_disabled = buffer_read_u8(buf);
        b->tourism_income = buffer_read_u8(buf);
        b->tourism_income_this_year = buffer_read_u8(buf);
    }

    if (building_buf_size >= BUILDING_STATE_VARIANTS_AND_UPGRADES) {
        b->variant = buffer_read_u8(buf);
        b->upgrade_level = buffer_read_u8(buf);
    }

    if (building_buf_size >= BUILDING_STATE_STRIKES) {
        b->strike_duration_days = buffer_read_u8(buf);
    }

    if (building_buf_size >= BUILDING_STATE_SICKNESS) {
        b->sickness_level = buffer_read_u8(buf);
        b->sickness_duration = buffer_read_u8(buf);
        b->sickness_doctor_cure = buffer_read_u8(buf);
        b->fumigation_frame = buffer_read_u8(buf);
        b->fumigation_direction = buffer_read_u8(buf);
    }

    if (save_version > SAVE_GAME_LAST_STATIC_RESOURCES) {
        for (int i = 0; i < resource_total_mapped(); i++) {
            b->resources[resource_remap(i)] = buffer_read_i16(buf);
        }
        for (int i = 0; i < resource_total_mapped(); i++) {
            b->accepted_goods[resource_remap(i)] = buffer_read_u8(buf);
        }
    }

    if (
        (b->type == BUILDING_LIGHTHOUSE || b->type == BUILDING_CARAVANSERAI) && 
        b->figure_id2 && 
        !for_preview && 
        figure_get(b->figure_id2)->type != FIGURE_LABOR_SEEKER
    ) {
        b->figure_id = b->figure_id2;
        b->figure_id2 = 0;
    }

    // The following code should only be executed if the savegame includes building information that is not 
    // supported on this specific version of Augustus. The extra bytes in the buffer must be skipped in order
    // to prevent reading bogus data for the next building
    if (building_buf_size > BUILDING_STATE_CURRENT_BUFFER_SIZE) {
        buffer_skip(buf, building_buf_size - BUILDING_STATE_CURRENT_BUFFER_SIZE);
    }
}
