#include "building_state.h"

#include "building/building.h"
#include "building/monument.h"
#include "game/file_io.h"
#include "game/resource.h"

static int is_industry_type(const building *b)
{
    return b->output_resource_id || b->type == BUILDING_NATIVE_CROPS
        || b->type == BUILDING_SHIPYARD || b->type == BUILDING_WHARF;
}

static void write_type_data(buffer *buf, const building *b)
{
    if (building_is_house(b->type)) {
        for (int i = 0; i < INVENTORY_MAX; i++) {
            buffer_write_i16(buf, b->data.house.inventory[i]);
        }
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
    } else if (building_has_market_inventory(b->type)) {
        buffer_write_i16(buf, 0);
        for (int i = 0; i < INVENTORY_MAX; i++) {
            buffer_write_i16(buf, b->data.market.inventory[i]);
        }
        buffer_write_i16(buf, b->data.market.pottery_demand);
        buffer_write_i16(buf, b->data.market.furniture_demand);
        buffer_write_i16(buf, b->data.market.oil_demand);
        buffer_write_i16(buf, b->data.market.wine_demand);
        for (int i = 0; i < 3; i++) {
            buffer_write_i16(buf, 0);
        }
        buffer_write_u8(buf, b->data.market.fetch_inventory_id);
        buffer_write_u8(buf, b->data.market.is_mess_hall);
        for (int i = 0; i < 8; i++) {
            buffer_write_u8(buf, 0);
        }
    } else if (b->type == BUILDING_GRANARY) {
        buffer_write_i16(buf, 0);
        for (int i = 0; i < RESOURCE_MAX; i++) {
            buffer_write_i16(buf, b->data.granary.resource_stored[i]);
        }
        buffer_write_i32(buf, 0);
        buffer_write_i32(buf, 0);
    } else if (building_monument_is_monument(b)) {
        for (int i = 0; i < RESOURCE_MAX; i++) {
            buffer_write_i16(buf, b->data.monument.resources_needed[i]);
        }
        buffer_write_i32(buf, b->data.monument.upgrades);
        buffer_write_i16(buf, b->data.monument.progress);
        buffer_write_i16(buf, b->data.monument.monument_phase);
        buffer_write_i16(buf, 0);
    } else if (b->type == BUILDING_DOCK) {
        buffer_write_i16(buf, b->data.dock.queued_docker_id);
        for (int i = 0; i < 25; i++) {
            buffer_write_u8(buf, 0);
        }
        buffer_write_u8(buf, b->data.dock.num_ships);
        buffer_write_u8(buf, 0);
        buffer_write_u8(buf, 0);
        buffer_write_i8(buf, b->data.dock.orientation);
        buffer_write_u8(buf, 0);
        buffer_write_u8(buf, 0);
        buffer_write_u8(buf, 0);
        for (int i = 0; i < 3; i++) {
            buffer_write_i16(buf, b->data.dock.docker_ids[i]);
        }
        buffer_write_i16(buf, b->data.dock.trade_ship_id);
    } else if (is_industry_type(b)) {
        buffer_write_i16(buf, b->data.industry.progress);
        for (int i = 0; i < 12; i++) {
            buffer_write_u8(buf, 0);
        }
        buffer_write_u8(buf, b->data.industry.has_fish);
        for (int i = 0; i < 14; i++) {
            buffer_write_u8(buf, 0);
        }
        buffer_write_u8(buf, b->data.industry.blessing_days_left);
        buffer_write_u8(buf, b->data.industry.orientation);
        buffer_write_u8(buf, b->data.industry.has_raw_materials);
        buffer_write_u8(buf, 0);
        buffer_write_u8(buf, b->data.industry.curse_days_left);
        for (int i = 0; i < 6; i++) {
            buffer_write_u8(buf, 0);
        }
        buffer_write_i16(buf, b->data.industry.fishing_boat_id);
    } else {
        for (int i = 0; i < 26; i++) {
            buffer_write_u8(buf, 0);
        }
        buffer_write_u8(buf, b->data.entertainment.num_shows);
        buffer_write_u8(buf, b->data.entertainment.days1);
        buffer_write_u8(buf, b->data.entertainment.days2);
        buffer_write_u8(buf, b->data.entertainment.play);
        for (int i = 0; i < 12; i++) {
            buffer_write_u8(buf, 0);
        }
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
    buffer_write_u8(buf, b->house_tavern_meat_access);
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
    buffer_write_u8(buf, b->ruin_has_plague);
    buffer_write_i8(buf, b->desirability);
    buffer_write_u8(buf, b->is_deleted);
    buffer_write_u8(buf, b->is_adjacent_to_water);
    buffer_write_u8(buf, b->storage_id);
    buffer_write_i8(buf, b->sentiment.house_happiness); // which union field we use does not matter
    buffer_write_u8(buf, b->show_on_problem_overlay);

    // expanded building data
    buffer_write_u8(buf, b->house_arena_gladiator);
    buffer_write_u8(buf, b->house_arena_lion);
    buffer_write_u8(buf, b->is_tourism_venue);
    buffer_write_u8(buf, b->tourism_disabled);
    buffer_write_u8(buf, b->tourism_income);
    buffer_write_u8(buf, b->tourism_income_this_year);
    buffer_write_u8(buf, b->strike_duration_days);

    // New building state code should always be added at the end to preserve savegame retrocompatibility
    // Also, don't forget to update BUILDING_STATE_CURRENT_BUFFER_SIZE and if possible, add a new macro like
    // BUILDING_STATE_NEW_FEATURE_BUFFER_SIZE with the full building state buffer size including all added features
    // up until that point in Augustus' development
}

static void read_type_data(buffer *buf, building *b)
{
    if (building_is_house(b->type)) {
        for (int i = 0; i < INVENTORY_MAX; i++) {
            b->data.house.inventory[i] = buffer_read_i16(buf);
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
    } else if (building_has_market_inventory(b->type)) {
        buffer_skip(buf, 2);
        for (int i = 0; i < INVENTORY_MAX; i++) {
            b->data.market.inventory[i] = buffer_read_i16(buf);
        }
        b->data.market.pottery_demand = buffer_read_i16(buf);
        b->data.market.furniture_demand = buffer_read_i16(buf);
        b->data.market.oil_demand = buffer_read_i16(buf);
        b->data.market.wine_demand = buffer_read_i16(buf);
        buffer_skip(buf, 6);
        b->data.market.fetch_inventory_id = buffer_read_u8(buf);
        b->data.market.is_mess_hall = buffer_read_u8(buf);
        buffer_skip(buf, 8);
    } else if (b->type == BUILDING_GRANARY) {
        buffer_skip(buf, 2);
        for (int i = 0; i < RESOURCE_MAX; i++) {
            b->data.granary.resource_stored[i] = buffer_read_i16(buf);
        }
        buffer_skip(buf, 8);
    } else if (building_monument_is_monument(b)) {
        for (int i = 0; i < RESOURCE_MAX; i++) {
            b->data.monument.resources_needed[i] = buffer_read_i16(buf);
        }
        b->data.monument.upgrades = buffer_read_i32(buf);
        b->data.monument.progress = buffer_read_i16(buf);
        b->data.monument.monument_phase = buffer_read_i16(buf);
        buffer_skip(buf, 2);
    } else if (b->type == BUILDING_DOCK) {
        b->data.dock.queued_docker_id = buffer_read_i16(buf);
        buffer_skip(buf, 25);
        b->data.dock.num_ships = buffer_read_u8(buf);
        buffer_skip(buf, 2);
        b->data.dock.orientation = buffer_read_i8(buf);
        buffer_skip(buf, 3);
        for (int i = 0; i < 3; i++) {
            b->data.dock.docker_ids[i] = buffer_read_i16(buf);
        }
        b->data.dock.trade_ship_id = buffer_read_i16(buf);
    } else if (is_industry_type(b)) {
        b->data.industry.progress = buffer_read_i16(buf);
        buffer_skip(buf, 12);
        b->data.industry.has_fish = buffer_read_u8(buf);
        buffer_skip(buf, 14);
        b->data.industry.blessing_days_left = buffer_read_u8(buf);
        b->data.industry.orientation = buffer_read_u8(buf);
        b->data.industry.has_raw_materials = buffer_read_u8(buf);
        buffer_skip(buf, 1);
        b->data.industry.curse_days_left = buffer_read_u8(buf);
        buffer_skip(buf, 6);
        b->data.industry.fishing_boat_id = buffer_read_i16(buf);
    } else {
        buffer_skip(buf, 26);
        b->data.entertainment.num_shows = buffer_read_u8(buf);
        b->data.entertainment.days1 = buffer_read_u8(buf);
        b->data.entertainment.days2 = buffer_read_u8(buf);
        b->data.entertainment.play = buffer_read_u8(buf);
        buffer_skip(buf, 12);
    }
}

void building_state_load_from_buffer(buffer *buf, building *b, int building_buf_size)
{
    int index = buf->index;
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
    b->subtype.house_level = buffer_read_i16(buf); // which union field we use does not matter
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
    b->house_tavern_meat_access = buffer_read_u8(buf);
    b->prev_part_building_id = buffer_read_i16(buf);
    b->next_part_building_id = buffer_read_i16(buf);
    b->loads_stored = buffer_read_i16(buf);
    b->house_sentiment_message = buffer_read_u8(buf);
    b->has_well_access = buffer_read_u8(buf);
    b->num_workers = buffer_read_i16(buf);
    b->labor_category = buffer_read_u8(buf);
    b->output_resource_id = buffer_read_u8(buf);
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
    read_type_data(buf, b);
    b->tax_income_or_storage = buffer_read_i32(buf);
    b->house_days_without_food = buffer_read_u8(buf);
    b->ruin_has_plague = buffer_read_u8(buf);
    b->desirability = buffer_read_i8(buf);
    b->is_deleted = buffer_read_u8(buf);
    b->is_adjacent_to_water = buffer_read_u8(buf);
    b->storage_id = buffer_read_u8(buf);
    b->sentiment.house_happiness = buffer_read_i8(buf); // which union field we use does not matter
    b->show_on_problem_overlay = buffer_read_u8(buf);

    // backwards compatibility fixes for culture update
    if (building_monument_is_monument(b) && b->subtype.house_level && b->type != BUILDING_HIPPODROME) {
        b->data.monument.monument_phase = b->subtype.house_level;
    }

    if ((b->type == BUILDING_HIPPODROME || b->type == BUILDING_COLOSSEUM) && !b->data.monument.monument_phase) {
        b->data.monument.monument_phase = -1;
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
        b->strike_duration_days = buffer_read_u8(buf);
    }

    // The following code should only be executed if the savegame includes building information that is not 
    // supported on this specific version of Augustus. The extra bytes in the buffer must be skipped in order
    // to prevent reading bogus data for the next building
    if (building_buf_size > BUILDING_STATE_CURRENT_BUFFER_SIZE) {
        buffer_skip(buf, building_buf_size - BUILDING_STATE_CURRENT_BUFFER_SIZE);
    }
}
