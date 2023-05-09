#include "scenario.h"

#include "city/resource.h"
#include "empire/city.h"
#include "empire/trade_route.h"
#include "game/difficulty.h"
#include "game/save_version.h"
#include "game/settings.h"
#include "scenario/data.h"
#include "scenario/request.h"

struct scenario_t scenario;

static struct {
    int size;
    int start_info;
    int old_filler_1;
    int invasions_part1;
    int start_funds_and_enemy_id;
    int map_size;
    int briefing;
    int old_filler_2;
    int image;
    int herds;
    int demand_changes_part1;
    int price_changes_part1;
    int gladiator_revolt;
    int emperor_change;
    int random_events;
    int fishing;
    int old_filler_3;
    int invasions_part2;
    int old_filler_4;
    int rome_wheat;
    int allowed_buildings;
    int win_criteria;
    int map_points;
    int invasions_part3;
    int misc;
    int requests;
    int introduction;
    int custom_name;
    int end;
} state_offsets;

int scenario_is_saved(void)
{
    return scenario.is_saved;
}

static void state_offsets_init(int scenario_version)
{
    int next_start_offset = 0;

    if (scenario_version > SCENARIO_LAST_NO_EXTENDED_REQUESTS) {
        state_offsets.size = next_start_offset;
        next_start_offset = state_offsets.size + sizeof(int32_t);
    }

    state_offsets.start_info = next_start_offset;
    next_start_offset = state_offsets.start_info + 14;

    if (scenario_version <= SCENARIO_LAST_NO_EXTENDED_REQUESTS) {
        state_offsets.old_filler_1 = next_start_offset;
        next_start_offset = state_offsets.old_filler_1 + 160;
    }

    state_offsets.invasions_part1 = next_start_offset;
    next_start_offset = state_offsets.invasions_part1 + (MAX_INVASIONS * 10);

    state_offsets.start_funds_and_enemy_id = next_start_offset;
    next_start_offset = state_offsets.start_funds_and_enemy_id + 14;

    state_offsets.map_size = next_start_offset;
    next_start_offset = state_offsets.map_size + 16;

    state_offsets.briefing = next_start_offset; 
    next_start_offset = state_offsets.briefing + MAX_BRIEF_DESCRIPTION + MAX_BRIEFING;

    if (scenario_version <= SCENARIO_LAST_NO_EXTENDED_REQUESTS) {
        state_offsets.old_filler_2 = next_start_offset;
        next_start_offset = state_offsets.old_filler_2 + 20;
    }

    state_offsets.image = next_start_offset;
    next_start_offset = state_offsets.image + 6;

    state_offsets.herds = next_start_offset;
    next_start_offset = state_offsets.herds + (MAX_HERD_POINTS * 4);
    
    state_offsets.demand_changes_part1 = next_start_offset;
    next_start_offset = state_offsets.demand_changes_part1 + (MAX_DEMAND_CHANGES * 9);

    state_offsets.price_changes_part1 = next_start_offset;
    next_start_offset = state_offsets.price_changes_part1 + (MAX_PRICE_CHANGES * 6);

    state_offsets.gladiator_revolt = next_start_offset;
    next_start_offset = state_offsets.gladiator_revolt + 8;

    state_offsets.emperor_change = next_start_offset;
    next_start_offset = state_offsets.emperor_change + 8;

    state_offsets.random_events = next_start_offset;
    next_start_offset = state_offsets.random_events + 36;

    state_offsets.fishing = next_start_offset;
    next_start_offset = state_offsets.fishing + (MAX_FISH_POINTS * 4);

    if (scenario_version <= SCENARIO_LAST_NO_EXTENDED_REQUESTS) {
        state_offsets.old_filler_3 = next_start_offset;
        next_start_offset = state_offsets.old_filler_3 + 20;
    }

    state_offsets.invasions_part2 = next_start_offset;
    next_start_offset = state_offsets.invasions_part2 + (MAX_INVASIONS * 1);

    if (scenario_version <= SCENARIO_LAST_NO_EXTENDED_REQUESTS) {
        state_offsets.old_filler_4 = next_start_offset;
        next_start_offset = state_offsets.old_filler_4 + 80;
    }

    state_offsets.rome_wheat = next_start_offset;
    next_start_offset = state_offsets.rome_wheat + 4;

    state_offsets.allowed_buildings = next_start_offset;
    next_start_offset = state_offsets.allowed_buildings + (MAX_ALLOWED_BUILDINGS * 2);

    state_offsets.win_criteria = next_start_offset;
    next_start_offset = state_offsets.win_criteria + 52;

    state_offsets.map_points = next_start_offset;
    next_start_offset = state_offsets.map_points + 12;

    state_offsets.invasions_part3 = next_start_offset;
    next_start_offset = state_offsets.invasions_part3 + (MAX_INVASION_POINTS * 4);

    state_offsets.misc = next_start_offset;
    next_start_offset = state_offsets.misc + 51;

    if (scenario_version > SCENARIO_LAST_NO_CUSTOM_MESSAGES) {
        state_offsets.introduction = next_start_offset;
        next_start_offset = state_offsets.introduction + 4;
    }

    state_offsets.custom_name = next_start_offset;
    next_start_offset = state_offsets.custom_name + 51;

    state_offsets.end = next_start_offset;
}

int scenario_get_state_buffer_size_by_savegame_version(int savegame_version)
{
    if (savegame_version <= SAVE_GAME_LAST_UNVERSIONED_SCENARIOS) {
        return 1720;
    } else if (savegame_version <= SAVE_GAME_LAST_NO_EXTENDED_REQUESTS) {
        state_offsets_init(SCENARIO_LAST_NO_EXTENDED_REQUESTS);
        return state_offsets.end;
    } else if (savegame_version <= SAVE_GAME_LAST_NO_CUSTOM_MESSAGES) {
        state_offsets_init(SCENARIO_LAST_NO_CUSTOM_MESSAGES);
        return state_offsets.end;
    } else {
        state_offsets_init(SCENARIO_CURRENT_VERSION);
        return state_offsets.end;
    }
}

int scenario_get_state_buffer_size_by_scenario_version(int scenario_version)
{
    if (scenario_version <= SCENARIO_LAST_UNVERSIONED) {
        return 1720;
    } else if(scenario_version <= SCENARIO_LAST_NO_WAGE_LIMITS) {
        return 1830;
    } else {
        state_offsets_init(scenario_version);
        return state_offsets.end;
    }
}

void scenario_save_state(buffer *buf)
{
    int buf_size = scenario_get_state_buffer_size_by_scenario_version(SCENARIO_CURRENT_VERSION);
    uint8_t *buf_data = malloc(buf_size);
    buffer_init(buf, buf_data, buf_size);
    
    // size
    buffer_write_i32(buf, buf_size);

    buffer_write_i16(buf, scenario.start_year);
    buffer_write_i16(buf, 0);
    buffer_write_i16(buf, scenario.empire.id);
    buffer_skip(buf, 8);

    for (int i = 0; i < MAX_INVASIONS; i++) {
        buffer_write_i16(buf, scenario.invasions[i].year);
    }
    for (int i = 0; i < MAX_INVASIONS; i++) {
        buffer_write_i16(buf, scenario.invasions[i].type);
    }
    for (int i = 0; i < MAX_INVASIONS; i++) {
        buffer_write_i16(buf, scenario.invasions[i].amount);
    }
    for (int i = 0; i < MAX_INVASIONS; i++) {
        buffer_write_i16(buf, scenario.invasions[i].from);
    }
    for (int i = 0; i < MAX_INVASIONS; i++) {
        buffer_write_i16(buf, scenario.invasions[i].attack_type);
    }

    buffer_write_i16(buf, 0);
    buffer_write_i32(buf, scenario.initial_funds);
    buffer_write_i16(buf, scenario.enemy_id);
    buffer_write_i16(buf, 0);
    buffer_write_i16(buf, 0);
    buffer_write_i16(buf, 0);

    buffer_write_i32(buf, scenario.map.width);
    buffer_write_i32(buf, scenario.map.height);
    buffer_write_i32(buf, scenario.map.grid_start);
    buffer_write_i32(buf, scenario.map.grid_border_size);

    buffer_write_raw(buf, scenario.brief_description, MAX_BRIEF_DESCRIPTION);
    buffer_write_raw(buf, scenario.briefing, MAX_BRIEFING);

    buffer_write_i16(buf, scenario.image_id);
    buffer_write_i16(buf, scenario.is_open_play);
    buffer_write_i16(buf, scenario.player_rank);

    for (int i = 0; i < MAX_HERD_POINTS; i++) {
        buffer_write_i16(buf, scenario.herd_points[i].x);
    }
    for (int i = 0; i < MAX_HERD_POINTS; i++) {
        buffer_write_i16(buf, scenario.herd_points[i].y);
    }

    for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
        buffer_write_i16(buf, scenario.demand_changes[i].year);
    }
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
        buffer_write_u8(buf, scenario.demand_changes[i].month);
    }
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
        buffer_write_u8(buf, scenario.demand_changes[i].resource);
    }
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
        buffer_write_u8(buf, scenario.demand_changes[i].route_id);
    }
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
        buffer_write_i32(buf, scenario.demand_changes[i].amount);
    }

    for (int i = 0; i < MAX_PRICE_CHANGES; i++) {
        buffer_write_i16(buf, scenario.price_changes[i].year);
    }
    for (int i = 0; i < MAX_PRICE_CHANGES; i++) {
        buffer_write_u8(buf, scenario.price_changes[i].month);
    }
    for (int i = 0; i < MAX_PRICE_CHANGES; i++) {
        buffer_write_u8(buf, scenario.price_changes[i].resource);
    }
    for (int i = 0; i < MAX_PRICE_CHANGES; i++) {
        buffer_write_u8(buf, scenario.price_changes[i].amount);
    }
    for (int i = 0; i < MAX_PRICE_CHANGES; i++) {
        buffer_write_u8(buf, scenario.price_changes[i].is_rise);
    }

    buffer_write_i32(buf, scenario.gladiator_revolt.enabled);
    buffer_write_i32(buf, scenario.gladiator_revolt.year);
    buffer_write_i32(buf, scenario.emperor_change.enabled);
    buffer_write_i32(buf, scenario.emperor_change.year);

    buffer_write_i32(buf, scenario.random_events.sea_trade_problem);
    buffer_write_i32(buf, scenario.random_events.land_trade_problem);
    buffer_write_i32(buf, scenario.random_events.raise_wages);
    buffer_write_i32(buf, scenario.random_events.max_wages);
    buffer_write_i32(buf, scenario.random_events.lower_wages);
    buffer_write_i32(buf, scenario.random_events.min_wages);
    buffer_write_i32(buf, scenario.random_events.contaminated_water);
    buffer_write_i32(buf, scenario.random_events.iron_mine_collapse);
    buffer_write_i32(buf, scenario.random_events.clay_pit_flooded);

    for (int i = 0; i < MAX_FISH_POINTS; i++) {
        buffer_write_i16(buf, scenario.fishing_points[i].x);
    }
    for (int i = 0; i < MAX_FISH_POINTS; i++) {
        buffer_write_i16(buf, scenario.fishing_points[i].y);
    }

    for (int i = 0; i < MAX_INVASIONS; i++) {
        buffer_write_u8(buf, scenario.invasions[i].month);
    }

    buffer_write_i32(buf, scenario.rome_supplies_wheat);

    for (int i = 0; i < MAX_ALLOWED_BUILDINGS; i++) {
        buffer_write_i16(buf, scenario.allowed_buildings[i]);
    }

    buffer_write_i32(buf, scenario.win_criteria.culture.goal);
    buffer_write_i32(buf, scenario.win_criteria.prosperity.goal);
    buffer_write_i32(buf, scenario.win_criteria.peace.goal);
    buffer_write_i32(buf, scenario.win_criteria.favor.goal);
    buffer_write_u8(buf, scenario.win_criteria.culture.enabled);
    buffer_write_u8(buf, scenario.win_criteria.prosperity.enabled);
    buffer_write_u8(buf, scenario.win_criteria.peace.enabled);
    buffer_write_u8(buf, scenario.win_criteria.favor.enabled);
    buffer_write_i32(buf, scenario.win_criteria.time_limit.enabled);
    buffer_write_i32(buf, scenario.win_criteria.time_limit.years);
    buffer_write_i32(buf, scenario.win_criteria.survival_time.enabled);
    buffer_write_i32(buf, scenario.win_criteria.survival_time.years);

    buffer_write_i32(buf, scenario.earthquake.severity);
    buffer_write_i32(buf, scenario.earthquake.year);

    buffer_write_i32(buf, scenario.win_criteria.population.enabled);
    buffer_write_i32(buf, scenario.win_criteria.population.goal);

    buffer_write_i16(buf, scenario.earthquake_point.x);
    buffer_write_i16(buf, scenario.earthquake_point.y);
    buffer_write_i16(buf, scenario.entry_point.x);
    buffer_write_i16(buf, scenario.entry_point.y);
    buffer_write_i16(buf, scenario.exit_point.x);
    buffer_write_i16(buf, scenario.exit_point.y);

    for (int i = 0; i < MAX_INVASION_POINTS; i++) {
        buffer_write_i16(buf, scenario.invasion_points[i].x);
    }
    for (int i = 0; i < MAX_INVASION_POINTS; i++) {
        buffer_write_i16(buf, scenario.invasion_points[i].y);
    }

    buffer_write_i16(buf, scenario.river_entry_point.x);
    buffer_write_i16(buf, scenario.river_entry_point.y);
    buffer_write_i16(buf, scenario.river_exit_point.x);
    buffer_write_i16(buf, scenario.river_exit_point.y);

    buffer_write_i32(buf, scenario.rescue_loan);
    buffer_write_i32(buf, scenario.win_criteria.milestone25_year);
    buffer_write_i32(buf, scenario.win_criteria.milestone50_year);
    buffer_write_i32(buf, scenario.win_criteria.milestone75_year);

    buffer_write_i32(buf, scenario.native_images.hut);
    buffer_write_i32(buf, scenario.native_images.meeting);
    buffer_write_i32(buf, scenario.native_images.crops);

    buffer_write_u8(buf, scenario.climate);
    buffer_write_u8(buf, scenario.flotsam_enabled);

    buffer_write_i16(buf, 0);

    buffer_write_i32(buf, scenario.empire.is_expanded);
    buffer_write_i32(buf, scenario.empire.expansion_year);

    buffer_write_u8(buf, scenario.empire.distant_battle_roman_travel_months);
    buffer_write_u8(buf, scenario.empire.distant_battle_enemy_travel_months);
    buffer_write_u8(buf, scenario.open_play_scenario_id);

    buffer_write_i32(buf, scenario.intro_custom_message_id);
    
    buffer_write_raw(buf, scenario.empire.custom_name, sizeof(scenario.empire.custom_name));
    buffer_write_u8(buf, 0);
}

void scenario_requests_save_state(buffer *buf)
{
    scenario_request_save_state(buf);

    scenario.is_saved = 1;
}

void scenario_load_state(buffer *buf, buffer *buf_requests, int version)
{
    state_offsets_init(version);
    if (version > SCENARIO_LAST_NO_EXTENDED_REQUESTS) {
        buffer_read_i32(buf);
    }

    scenario.start_year = buffer_read_i16(buf);
    buffer_skip(buf, 2);
    scenario.empire.id = buffer_read_i16(buf);
    buffer_skip(buf, 8);

    scenario_request_load_state_old_version(buf, version, REQUESTS_OLD_STATE_SECTIONS_TARGET);

    for (int i = 0; i < MAX_INVASIONS; i++) {
        scenario.invasions[i].year = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_INVASIONS; i++) {
        scenario.invasions[i].type = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_INVASIONS; i++) {
        scenario.invasions[i].amount = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_INVASIONS; i++) {
        scenario.invasions[i].from = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_INVASIONS; i++) {
        scenario.invasions[i].attack_type = buffer_read_i16(buf);
    }

    buffer_skip(buf, 2);
    scenario.initial_funds = buffer_read_i32(buf);
    scenario.enemy_id = buffer_read_i16(buf);
    buffer_skip(buf, 6);

    scenario.map.width = buffer_read_i32(buf);
    scenario.map.height = buffer_read_i32(buf);
    scenario.map.grid_start = buffer_read_i32(buf);
    scenario.map.grid_border_size = buffer_read_i32(buf);

    buffer_read_raw(buf, scenario.brief_description, MAX_BRIEF_DESCRIPTION);
    buffer_read_raw(buf, scenario.briefing, MAX_BRIEFING);

    scenario_request_load_state_old_version(buf, version, REQUESTS_OLD_STATE_SECTIONS_CAN_COMPLY);

    scenario.image_id = buffer_read_i16(buf);
    scenario.is_open_play = buffer_read_i16(buf);
    scenario.player_rank = buffer_read_i16(buf);

    for (int i = 0; i < MAX_HERD_POINTS; i++) {
        scenario.herd_points[i].x = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_HERD_POINTS; i++) {
        scenario.herd_points[i].y = buffer_read_i16(buf);
    }

    for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
        scenario.demand_changes[i].year = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
        scenario.demand_changes[i].month = buffer_read_u8(buf);
    }
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
        scenario.demand_changes[i].resource = buffer_read_u8(buf);
    }
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
        scenario.demand_changes[i].route_id = buffer_read_u8(buf);
    }
    if (version <= SCENARIO_LAST_UNVERSIONED) {
        for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
            int is_rise = buffer_read_u8(buf);
            int amount = is_rise ? DEMAND_CHANGE_LEGACY_IS_RISE : DEMAND_CHANGE_LEGACY_IS_FALL;
            scenario.demand_changes[i].amount = amount;
        }
    } else {
        for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
            scenario.demand_changes[i].amount = buffer_read_i32(buf);
        }
    }

    for (int i = 0; i < MAX_PRICE_CHANGES; i++) {
        scenario.price_changes[i].year = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_PRICE_CHANGES; i++) {
        scenario.price_changes[i].month = buffer_read_u8(buf);
    }
    for (int i = 0; i < MAX_PRICE_CHANGES; i++) {
        scenario.price_changes[i].resource = buffer_read_u8(buf);
    }
    for (int i = 0; i < MAX_PRICE_CHANGES; i++) {
        scenario.price_changes[i].amount = buffer_read_u8(buf);
    }
    for (int i = 0; i < MAX_PRICE_CHANGES; i++) {
        scenario.price_changes[i].is_rise = buffer_read_u8(buf);
    }

    scenario.gladiator_revolt.enabled = buffer_read_i32(buf);
    scenario.gladiator_revolt.year = buffer_read_i32(buf);
    scenario.emperor_change.enabled = buffer_read_i32(buf);
    scenario.emperor_change.year = buffer_read_i32(buf);

    scenario.random_events.sea_trade_problem = buffer_read_i32(buf);
    scenario.random_events.land_trade_problem = buffer_read_i32(buf);
    scenario.random_events.raise_wages = buffer_read_i32(buf);
    if (version > SCENARIO_LAST_NO_WAGE_LIMITS) {
        scenario.random_events.max_wages = buffer_read_i32(buf);
    }
    if (!scenario.random_events.max_wages) {
        scenario.random_events.max_wages = 45;
    }
    scenario.random_events.lower_wages = buffer_read_i32(buf);
    if (version > SCENARIO_LAST_NO_WAGE_LIMITS) {
        scenario.random_events.min_wages = buffer_read_i32(buf);
    }
    if (!scenario.random_events.min_wages) {
        scenario.random_events.min_wages = 5;
    }
    scenario.random_events.contaminated_water = buffer_read_i32(buf);
    scenario.random_events.iron_mine_collapse = buffer_read_i32(buf);
    scenario.random_events.clay_pit_flooded = buffer_read_i32(buf);

    for (int i = 0; i < MAX_FISH_POINTS; i++) {
        scenario.fishing_points[i].x = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_FISH_POINTS; i++) {
        scenario.fishing_points[i].y = buffer_read_i16(buf);
    }

    scenario_request_load_state_old_version(buf, version, REQUESTS_OLD_STATE_SECTIONS_FAVOR_REWARD);

    for (int i = 0; i < MAX_INVASIONS; i++) {
        scenario.invasions[i].month = buffer_read_u8(buf);
    }

    scenario_request_load_state_old_version(buf, version, REQUESTS_OLD_STATE_SECTIONS_ONGOING_INFO);

    scenario.rome_supplies_wheat = buffer_read_i32(buf);

    for (int i = 0; i < MAX_ALLOWED_BUILDINGS; i++) {
        scenario.allowed_buildings[i] = buffer_read_i16(buf);
    }

    scenario.win_criteria.culture.goal = buffer_read_i32(buf);
    scenario.win_criteria.prosperity.goal = buffer_read_i32(buf);
    scenario.win_criteria.peace.goal = buffer_read_i32(buf);
    scenario.win_criteria.favor.goal = buffer_read_i32(buf);
    scenario.win_criteria.culture.enabled = buffer_read_u8(buf);
    scenario.win_criteria.prosperity.enabled = buffer_read_u8(buf);
    scenario.win_criteria.peace.enabled = buffer_read_u8(buf);
    scenario.win_criteria.favor.enabled = buffer_read_u8(buf);
    scenario.win_criteria.time_limit.enabled = buffer_read_i32(buf);
    scenario.win_criteria.time_limit.years = buffer_read_i32(buf);
    scenario.win_criteria.survival_time.enabled = buffer_read_i32(buf);
    scenario.win_criteria.survival_time.years = buffer_read_i32(buf);

    scenario.earthquake.severity = buffer_read_i32(buf);
    scenario.earthquake.year = buffer_read_i32(buf);

    scenario.win_criteria.population.enabled = buffer_read_i32(buf);
    scenario.win_criteria.population.goal = buffer_read_i32(buf);

    scenario.earthquake_point.x = buffer_read_i16(buf);
    scenario.earthquake_point.y = buffer_read_i16(buf);
    scenario.entry_point.x = buffer_read_i16(buf);
    scenario.entry_point.y = buffer_read_i16(buf);
    scenario.exit_point.x = buffer_read_i16(buf);
    scenario.exit_point.y = buffer_read_i16(buf);

    for (int i = 0; i < MAX_INVASION_POINTS; i++) {
        scenario.invasion_points[i].x = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_INVASION_POINTS; i++) {
        scenario.invasion_points[i].y = buffer_read_i16(buf);
    }

    scenario.river_entry_point.x = buffer_read_i16(buf);
    scenario.river_entry_point.y = buffer_read_i16(buf);
    scenario.river_exit_point.x = buffer_read_i16(buf);
    scenario.river_exit_point.y = buffer_read_i16(buf);

    scenario.rescue_loan = buffer_read_i32(buf);
    scenario.win_criteria.milestone25_year = buffer_read_i32(buf);
    scenario.win_criteria.milestone50_year = buffer_read_i32(buf);
    scenario.win_criteria.milestone75_year = buffer_read_i32(buf);

    scenario.native_images.hut = buffer_read_i32(buf);
    scenario.native_images.meeting = buffer_read_i32(buf);
    scenario.native_images.crops = buffer_read_i32(buf);

    scenario.climate = buffer_read_u8(buf);
    scenario.flotsam_enabled = buffer_read_u8(buf);

    buffer_skip(buf, 2);

    scenario.empire.is_expanded = buffer_read_i32(buf);
    scenario.empire.expansion_year = buffer_read_i32(buf);

    scenario.empire.distant_battle_roman_travel_months = buffer_read_u8(buf);
    scenario.empire.distant_battle_enemy_travel_months = buffer_read_u8(buf);
    scenario.open_play_scenario_id = buffer_read_u8(buf);

    scenario.intro_custom_message_id = -1;
    if (version > SCENARIO_LAST_NO_CUSTOM_MESSAGES) {
        scenario.intro_custom_message_id = buffer_read_i32(buf);
    }
    
    if (version > SCENARIO_LAST_UNVERSIONED) {
        buffer_read_raw(buf, scenario.empire.custom_name, sizeof(scenario.empire.custom_name));
    }
    buffer_skip(buf, 1);

    // We can only remap resources at the end of the scenario load as the remapping relies on the allowed building list
    // being loaded, otherwise on some edge cases changes to meat may affect fish instead
    if (resource_mapping_get_version() < RESOURCE_CURRENT_VERSION) {
        for (int i = 0; i < MAX_REQUESTS; i++) {
            scenario.requests[i].resource = resource_remap(scenario.requests[i].resource);
        }
        for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
            scenario.demand_changes[i].resource = resource_remap(scenario.demand_changes[i].resource);
        }
        for (int i = 0; i < MAX_PRICE_CHANGES; i++) {
            scenario.price_changes[i].resource = resource_remap(scenario.price_changes[i].resource);
        }
    }

    if (version > SCENARIO_LAST_NO_EXTENDED_REQUESTS) {
        scenario_request_load_state(buf_requests);
    }
    scenario.is_saved = 1;
}

void scenario_description_from_buffer(buffer *buf, uint8_t *description, int version)
{
    state_offsets_init(version);
    buffer_set(buf, state_offsets.briefing);
    buffer_read_raw(buf, description, MAX_BRIEF_DESCRIPTION);
}

int scenario_climate_from_buffer(buffer *buf, int version)
{
    state_offsets_init(version);
    if (version <= SCENARIO_LAST_UNVERSIONED) {
        buffer_set(buf, 1704);
    } else if (version <= SCENARIO_LAST_NO_WAGE_LIMITS) {
        buffer_set(buf, 1764);
    } else {
        buffer_set(buf, state_offsets.misc + 36);
    }
    return buffer_read_u8(buf);
}

int scenario_invasions_from_buffer(buffer *buf, int version)
{
    state_offsets_init(version);
    buffer_set(buf, state_offsets.invasions_part1 + (MAX_INVASIONS * 2));
    int num_invasions = 0;
    for (int i = 0; i < MAX_INVASIONS; i++) {
        if (buffer_read_i16(buf)) {
            num_invasions++;
        }
    }
    return num_invasions;
}

int scenario_image_id_from_buffer(buffer *buf, int version)
{
    state_offsets_init(version);
    buffer_set(buf, state_offsets.image);
    return buffer_read_i16(buf);
}

int scenario_rank_from_buffer(buffer *buf, int version)
{
    state_offsets_init(version);
    buffer_set(buf, state_offsets.image + 4);
    return buffer_read_i16(buf);
}

void scenario_open_play_info_from_buffer(buffer *buf, int version, int *is_open_play, int *open_play_id)
{
    state_offsets_init(version);
    buffer_set(buf, state_offsets.image + 2);
    *is_open_play = buffer_read_i16(buf);

    if (version <= SCENARIO_LAST_UNVERSIONED) {
        buffer_set(buf, 1718);
    } else if (version <= SCENARIO_LAST_NO_WAGE_LIMITS) {
        buffer_set(buf, 1778);
    } else {
        buffer_set(buf, state_offsets.misc + 50);
    }
    *open_play_id = buffer_read_u8(buf);
}

int scenario_start_year_from_buffer(buffer *buf, int version)
{
    state_offsets_init(version);
    buffer_set(buf, state_offsets.start_info);
    return buffer_read_i16(buf);
}

void scenario_objectives_from_buffer(buffer *buf, int version, scenario_win_criteria *win_criteria)
{
    state_offsets_init(version);
    if (version <= SCENARIO_LAST_UNVERSIONED) {
        buffer_set(buf, 1572);
    } else if (version <= SCENARIO_LAST_NO_WAGE_LIMITS) {
        buffer_set(buf, 1632);
    } else {
        buffer_set(buf, state_offsets.win_criteria);
    }
    win_criteria->culture.goal = buffer_read_i32(buf);
    win_criteria->prosperity.goal = buffer_read_i32(buf);
    win_criteria->peace.goal = buffer_read_i32(buf);
    win_criteria->favor.goal = buffer_read_i32(buf);
    win_criteria->culture.enabled = buffer_read_u8(buf);
    win_criteria->prosperity.enabled = buffer_read_u8(buf);
    win_criteria->peace.enabled = buffer_read_u8(buf);
    win_criteria->favor.enabled = buffer_read_u8(buf);
    win_criteria->time_limit.enabled = buffer_read_i32(buf);
    win_criteria->time_limit.years = buffer_read_i32(buf);
    win_criteria->survival_time.enabled = buffer_read_i32(buf);
    win_criteria->survival_time.years = buffer_read_i32(buf);
    buffer_skip(buf, 8);
    win_criteria->population.enabled = buffer_read_i32(buf);
    win_criteria->population.goal = buffer_read_i32(buf);
}

void scenario_map_data_from_buffer(buffer *buf, int *width, int *height, int *grid_start, int *grid_border_size, int version)
{
    state_offsets_init(version);
    buffer_set(buf, state_offsets.map_size);
    *width = buffer_read_i32(buf);
    *height = buffer_read_i32(buf);
    *grid_start = buffer_read_i32(buf);
    *grid_border_size = buffer_read_i32(buf);
}

void scenario_settings_init(void)
{
    scenario.settings.campaign_mission = 0;
    scenario.settings.campaign_rank = 0;
    scenario.settings.is_custom = 0;
    scenario.settings.starting_favor = difficulty_starting_favor();
    scenario.settings.starting_personal_savings = 0;
}

void scenario_settings_init_mission(void)
{
    scenario.settings.starting_favor = difficulty_starting_favor();
    scenario.settings.starting_personal_savings =
        setting_personal_savings_for_mission(scenario.settings.campaign_rank);
}

void scenario_fix_patch_trade(int mission_id) {
    // Damascus, allow import of marble and marble buildings
    if (mission_id == 15) {
        empire_city_force_sell(1, RESOURCE_MARBLE);
        trade_route_set(1, RESOURCE_MARBLE, 25);        
        scenario.allowed_buildings[ALLOWED_BUILDING_LARGE_TEMPLES] = 1;
        scenario.allowed_buildings[ALLOWED_BUILDING_ORACLE] = 1;
    // Caesarea, allow import of clay (for monuments)
    } else if (mission_id == 14) {
        empire_city_force_sell(3, RESOURCE_CLAY);
        trade_route_set(3, RESOURCE_CLAY, 15);
    }
}

void scenario_unlock_all_buildings(void) {
    for (int i = 0; i < MAX_ALLOWED_BUILDINGS; i++) {
        scenario.allowed_buildings[i] = 1;
    }
}


void scenario_settings_save_state(buffer *part1, buffer *part2, buffer *part3, buffer *player_name, buffer *scenario_name)
{
    buffer_write_i32(part1, scenario.settings.campaign_mission);

    buffer_write_i32(part2, scenario.settings.starting_favor);
    buffer_write_i32(part2, scenario.settings.starting_personal_savings);
    buffer_write_i32(part2, scenario.settings.campaign_rank);

    buffer_write_i32(part3, scenario.settings.is_custom);

    for (int i = 0; i < MAX_PLAYER_NAME; i++) {
        buffer_write_u8(player_name, 0);
    }
    buffer_write_raw(player_name, scenario.settings.player_name, MAX_PLAYER_NAME);
    buffer_write_raw(scenario_name, scenario.scenario_name, MAX_SCENARIO_NAME);
}

void scenario_settings_load_state(
    buffer *part1, buffer *part2, buffer *part3, buffer *player_name, buffer *scenario_name)
{
    scenario.settings.campaign_mission = buffer_read_i32(part1);

    scenario.settings.starting_favor = buffer_read_i32(part2);
    scenario.settings.starting_personal_savings = buffer_read_i32(part2);
    scenario.settings.campaign_rank = buffer_read_i32(part2);

    scenario.settings.is_custom = buffer_read_i32(part3);

    buffer_skip(player_name, MAX_PLAYER_NAME);
    buffer_read_raw(player_name, scenario.settings.player_name, MAX_PLAYER_NAME);
    buffer_read_raw(scenario_name, scenario.scenario_name, MAX_SCENARIO_NAME);
}
