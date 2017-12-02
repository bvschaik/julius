#include "scenario.h"

#include "game/difficulty.h"
#include "game/settings.h"
#include "scenario/data.h"

struct scenario_t scenario;

void scenario_save_state(buffer *buf)
{
    buffer_write_i16(buf, scenario.start_year);
    buffer_write_i16(buf, 0);
    buffer_write_i16(buf, scenario.empire.id);
    buffer_write_i32(buf, 0);
    buffer_write_i32(buf, 0);

    // requests
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        buffer_write_i16(buf, scenario.requests[i].year);
    }
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        buffer_write_i16(buf, scenario.requests[i].resource);
    }
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        buffer_write_i16(buf, scenario.requests[i].amount);
    }
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        buffer_write_i16(buf, scenario.requests[i].deadline_years);
    }

    // invasions
    for (int i = 0; i < MAX_INVASIONS; i++)
    {
        buffer_write_i16(buf, scenario.invasions[i].year);
    }
    for (int i = 0; i < MAX_INVASIONS; i++)
    {
        buffer_write_i16(buf, scenario.invasions[i].type);
    }
    for (int i = 0; i < MAX_INVASIONS; i++)
    {
        buffer_write_i16(buf, scenario.invasions[i].amount);
    }
    for (int i = 0; i < MAX_INVASIONS; i++)
    {
        buffer_write_i16(buf, scenario.invasions[i].from);
    }
    for (int i = 0; i < MAX_INVASIONS; i++)
    {
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

    buffer_write_raw(buf, scenario.brief_description, 64);
    for (int i = 0; i < 522; i++)
    {
        buffer_write_u8(buf, 0);
    }

    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        buffer_write_u8(buf, scenario.requests[i].can_comply_dialog_shown);
    }

    buffer_write_i16(buf, scenario.image_id);
    buffer_write_i16(buf, scenario.is_open_play);
    buffer_write_i16(buf, scenario.player_rank);

    for (int i = 0; i < MAX_HERD_POINTS; i++)
    {
        buffer_write_i16(buf, scenario.herd_points[i].x);
    }
    for (int i = 0; i < MAX_HERD_POINTS; i++)
    {
        buffer_write_i16(buf, scenario.herd_points[i].y);
    }

    // demand changes
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++)
    {
        buffer_write_i16(buf, scenario.demand_changes[i].year);
    }
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++)
    {
        buffer_write_u8(buf, scenario.demand_changes[i].month);
    }
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++)
    {
        buffer_write_u8(buf, scenario.demand_changes[i].resource);
    }
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++)
    {
        buffer_write_u8(buf, scenario.demand_changes[i].route_id);
    }
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++)
    {
        buffer_write_u8(buf, scenario.demand_changes[i].is_rise);
    }

    // price changes
    for (int i = 0; i < MAX_PRICE_CHANGES; i++)
    {
        buffer_write_i16(buf, scenario.price_changes[i].year);
    }
    for (int i = 0; i < MAX_PRICE_CHANGES; i++)
    {
        buffer_write_u8(buf, scenario.price_changes[i].month);
    }
    for (int i = 0; i < MAX_PRICE_CHANGES; i++)
    {
        buffer_write_u8(buf, scenario.price_changes[i].resource);
    }
    for (int i = 0; i < MAX_PRICE_CHANGES; i++)
    {
        buffer_write_u8(buf, scenario.price_changes[i].amount);
    }
    for (int i = 0; i < MAX_PRICE_CHANGES; i++)
    {
        buffer_write_u8(buf, scenario.price_changes[i].is_rise);
    }

    buffer_write_i32(buf, scenario.gladiator_revolt.enabled);
    buffer_write_i32(buf, scenario.gladiator_revolt.year);
    buffer_write_i32(buf, scenario.emperor_change.enabled);
    buffer_write_i32(buf, scenario.emperor_change.year);

    // random events
    buffer_write_i32(buf, scenario.random_events.sea_trade_problem);
    buffer_write_i32(buf, scenario.random_events.land_trade_problem);
    buffer_write_i32(buf, scenario.random_events.raise_wages);
    buffer_write_i32(buf, scenario.random_events.lower_wages);
    buffer_write_i32(buf, scenario.random_events.contaminated_water);
    buffer_write_i32(buf, scenario.random_events.iron_mine_collapse);
    buffer_write_i32(buf, scenario.random_events.clay_pit_flooded);

    for (int i = 0; i < MAX_FISH_POINTS; i++)
    {
        buffer_write_i16(buf, scenario.fishing_points[i].x);
    }
    for (int i = 0; i < MAX_FISH_POINTS; i++)
    {
        buffer_write_i16(buf, scenario.fishing_points[i].y);
    }

    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        buffer_write_u8(buf, scenario.requests[i].favor);
    }
    for (int i = 0; i < MAX_INVASIONS; i++)
    {
        buffer_write_u8(buf, scenario.invasions[i].month);
    }
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        buffer_write_u8(buf, scenario.requests[i].month);
    }
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        buffer_write_u8(buf, scenario.requests[i].state);
    }
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        buffer_write_u8(buf, scenario.requests[i].visible);
    }
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        buffer_write_u8(buf, scenario.requests[i].months_to_comply);
    }

    buffer_write_i32(buf, scenario.rome_supplies_wheat);

    // allowed buildings
    buffer_write_i16(buf, 0);
    buffer_write_i16(buf, scenario.allowed_buildings.farms);
    buffer_write_i16(buf, scenario.allowed_buildings.raw_materials);
    buffer_write_i16(buf, scenario.allowed_buildings.workshops);
    buffer_write_i16(buf, scenario.allowed_buildings.road);
    buffer_write_i16(buf, scenario.allowed_buildings.wall);
    buffer_write_i16(buf, scenario.allowed_buildings.aqueduct);
    buffer_write_i16(buf, 0);
    buffer_write_i16(buf, scenario.allowed_buildings.amphitheater);
    buffer_write_i16(buf, scenario.allowed_buildings.theater);
    buffer_write_i16(buf, scenario.allowed_buildings.hippodrome);
    buffer_write_i16(buf, scenario.allowed_buildings.colosseum);
    buffer_write_i16(buf, scenario.allowed_buildings.gladiator_school);
    buffer_write_i16(buf, scenario.allowed_buildings.lion_house);
    buffer_write_i16(buf, scenario.allowed_buildings.actor_colony);
    buffer_write_i16(buf, scenario.allowed_buildings.chariot_maker);
    buffer_write_i16(buf, scenario.allowed_buildings.gardens);
    buffer_write_i16(buf, scenario.allowed_buildings.plaza);
    buffer_write_i16(buf, scenario.allowed_buildings.statues);
    buffer_write_i16(buf, scenario.allowed_buildings.doctor);
    buffer_write_i16(buf, scenario.allowed_buildings.hospital);
    buffer_write_i16(buf, scenario.allowed_buildings.bathhouse);
    buffer_write_i16(buf, scenario.allowed_buildings.barber);
    buffer_write_i16(buf, scenario.allowed_buildings.school);
    buffer_write_i16(buf, scenario.allowed_buildings.academy);
    buffer_write_i16(buf, scenario.allowed_buildings.library);
    buffer_write_i16(buf, scenario.allowed_buildings.prefecture);
    buffer_write_i16(buf, scenario.allowed_buildings.fort);
    buffer_write_i16(buf, scenario.allowed_buildings.gatehouse);
    buffer_write_i16(buf, scenario.allowed_buildings.tower);
    buffer_write_i16(buf, scenario.allowed_buildings.small_temples);
    buffer_write_i16(buf, scenario.allowed_buildings.large_temples);
    buffer_write_i16(buf, scenario.allowed_buildings.market);
    buffer_write_i16(buf, scenario.allowed_buildings.granary);
    buffer_write_i16(buf, scenario.allowed_buildings.warehouse);
    buffer_write_i16(buf, 0);
    buffer_write_i16(buf, scenario.allowed_buildings.dock);
    buffer_write_i16(buf, scenario.allowed_buildings.wharf);
    buffer_write_i16(buf, scenario.allowed_buildings.governor_home);
    buffer_write_i16(buf, scenario.allowed_buildings.engineers_post);
    buffer_write_i16(buf, scenario.allowed_buildings.senate);
    buffer_write_i16(buf, scenario.allowed_buildings.forum);
    buffer_write_i16(buf, scenario.allowed_buildings.well);
    buffer_write_i16(buf, scenario.allowed_buildings.oracle);
    buffer_write_i16(buf, scenario.allowed_buildings.mission_post);
    buffer_write_i16(buf, scenario.allowed_buildings.bridge);
    buffer_write_i16(buf, scenario.allowed_buildings.barracks);
    buffer_write_i16(buf, scenario.allowed_buildings.military_academy);
    buffer_write_i16(buf, scenario.allowed_buildings.distribution_center);
    buffer_write_i16(buf, 0);

    // win criteria
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

    // map points
    buffer_write_i16(buf, scenario.earthquake_point.x);
    buffer_write_i16(buf, scenario.earthquake_point.y);
    buffer_write_i16(buf, scenario.entry_point.x);
    buffer_write_i16(buf, scenario.entry_point.y);
    buffer_write_i16(buf, scenario.exit_point.x);
    buffer_write_i16(buf, scenario.exit_point.y);

    for (int i = 0; i < MAX_INVASION_POINTS; i++)
    {
        buffer_write_i16(buf, scenario.invasion_points[i].x);
    }
    for (int i = 0; i < MAX_INVASION_POINTS; i++)
    {
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
    buffer_write_u8(buf, 0);
}

void scenario_load_state(buffer *buf)
{
    scenario.start_year = buffer_read_i16(buf);
    buffer_skip(buf, 2);
    scenario.empire.id = buffer_read_i16(buf);
    buffer_skip(buf, 8);

    // requests
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        scenario.requests[i].year = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        scenario.requests[i].resource = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        scenario.requests[i].amount = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        scenario.requests[i].deadline_years = buffer_read_i16(buf);
    }

    // invasions
    for (int i = 0; i < MAX_INVASIONS; i++)
    {
        scenario.invasions[i].year = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_INVASIONS; i++)
    {
        scenario.invasions[i].type = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_INVASIONS; i++)
    {
        scenario.invasions[i].amount = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_INVASIONS; i++)
    {
        scenario.invasions[i].from = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_INVASIONS; i++)
    {
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

    buffer_write_raw(buf, scenario.brief_description, 64);
    buffer_skip(buf, 522);

    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        scenario.requests[i].can_comply_dialog_shown = buffer_read_u8(buf);
    }

    scenario.image_id = buffer_read_i16(buf);
    scenario.is_open_play = buffer_read_i16(buf);
    scenario.player_rank = buffer_read_i16(buf);

    for (int i = 0; i < MAX_HERD_POINTS; i++)
    {
        scenario.herd_points[i].x = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_HERD_POINTS; i++)
    {
        scenario.herd_points[i].y = buffer_read_i16(buf);
    }

    // demand changes
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++)
    {
        scenario.demand_changes[i].year = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++)
    {
        scenario.demand_changes[i].month = buffer_read_u8(buf);
    }
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++)
    {
        scenario.demand_changes[i].resource = buffer_read_u8(buf);
    }
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++)
    {
        scenario.demand_changes[i].route_id = buffer_read_u8(buf);
    }
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++)
    {
        scenario.demand_changes[i].is_rise = buffer_read_u8(buf);
    }

    // price changes
    for (int i = 0; i < MAX_PRICE_CHANGES; i++)
    {
        scenario.price_changes[i].year = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_PRICE_CHANGES; i++)
    {
        scenario.price_changes[i].month = buffer_read_u8(buf);
    }
    for (int i = 0; i < MAX_PRICE_CHANGES; i++)
    {
        scenario.price_changes[i].resource = buffer_read_u8(buf);
    }
    for (int i = 0; i < MAX_PRICE_CHANGES; i++)
    {
        scenario.price_changes[i].amount = buffer_read_u8(buf);
    }
    for (int i = 0; i < MAX_PRICE_CHANGES; i++)
    {
        scenario.price_changes[i].is_rise = buffer_read_u8(buf);
    }

    scenario.gladiator_revolt.enabled = buffer_read_i32(buf);
    scenario.gladiator_revolt.year = buffer_read_i32(buf);
    scenario.emperor_change.enabled = buffer_read_i32(buf);
    scenario.emperor_change.year = buffer_read_i32(buf);

    // random events
    scenario.random_events.sea_trade_problem = buffer_read_i32(buf);
    scenario.random_events.land_trade_problem = buffer_read_i32(buf);
    scenario.random_events.raise_wages = buffer_read_i32(buf);
    scenario.random_events.lower_wages = buffer_read_i32(buf);
    scenario.random_events.contaminated_water = buffer_read_i32(buf);
    scenario.random_events.iron_mine_collapse = buffer_read_i32(buf);
    scenario.random_events.clay_pit_flooded = buffer_read_i32(buf);

    for (int i = 0; i < MAX_FISH_POINTS; i++)
    {
        scenario.fishing_points[i].x = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_FISH_POINTS; i++)
    {
        scenario.fishing_points[i].y = buffer_read_i16(buf);
    }

    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        scenario.requests[i].favor = buffer_read_u8(buf);
    }
    for (int i = 0; i < MAX_INVASIONS; i++)
    {
        scenario.invasions[i].month = buffer_read_u8(buf);
    }
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        scenario.requests[i].month = buffer_read_u8(buf);
    }
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        scenario.requests[i].state = buffer_read_u8(buf);
    }
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        scenario.requests[i].visible = buffer_read_u8(buf);
    }
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        scenario.requests[i].months_to_comply = buffer_read_u8(buf);
    }

    scenario.rome_supplies_wheat = buffer_read_i32(buf);

    // allowed buildings
    buffer_skip(buf, 2);
    scenario.allowed_buildings.farms = buffer_read_i16(buf);
    scenario.allowed_buildings.raw_materials = buffer_read_i16(buf);
    scenario.allowed_buildings.workshops = buffer_read_i16(buf);
    scenario.allowed_buildings.road = buffer_read_i16(buf);
    scenario.allowed_buildings.wall = buffer_read_i16(buf);
    scenario.allowed_buildings.aqueduct = buffer_read_i16(buf);
    buffer_skip(buf, 2);
    scenario.allowed_buildings.amphitheater = buffer_read_i16(buf);
    scenario.allowed_buildings.theater = buffer_read_i16(buf);
    scenario.allowed_buildings.hippodrome = buffer_read_i16(buf);
    scenario.allowed_buildings.colosseum = buffer_read_i16(buf);
    scenario.allowed_buildings.gladiator_school = buffer_read_i16(buf);
    scenario.allowed_buildings.lion_house = buffer_read_i16(buf);
    scenario.allowed_buildings.actor_colony = buffer_read_i16(buf);
    scenario.allowed_buildings.chariot_maker = buffer_read_i16(buf);
    scenario.allowed_buildings.gardens = buffer_read_i16(buf);
    scenario.allowed_buildings.plaza = buffer_read_i16(buf);
    scenario.allowed_buildings.statues = buffer_read_i16(buf);
    scenario.allowed_buildings.doctor = buffer_read_i16(buf);
    scenario.allowed_buildings.hospital = buffer_read_i16(buf);
    scenario.allowed_buildings.bathhouse = buffer_read_i16(buf);
    scenario.allowed_buildings.barber = buffer_read_i16(buf);
    scenario.allowed_buildings.school = buffer_read_i16(buf);
    scenario.allowed_buildings.academy = buffer_read_i16(buf);
    scenario.allowed_buildings.library = buffer_read_i16(buf);
    scenario.allowed_buildings.prefecture = buffer_read_i16(buf);
    scenario.allowed_buildings.fort = buffer_read_i16(buf);
    scenario.allowed_buildings.gatehouse = buffer_read_i16(buf);
    scenario.allowed_buildings.tower = buffer_read_i16(buf);
    scenario.allowed_buildings.small_temples = buffer_read_i16(buf);
    scenario.allowed_buildings.large_temples = buffer_read_i16(buf);
    scenario.allowed_buildings.market = buffer_read_i16(buf);
    scenario.allowed_buildings.granary = buffer_read_i16(buf);
    scenario.allowed_buildings.warehouse = buffer_read_i16(buf);
    buffer_skip(buf, 2);
    scenario.allowed_buildings.dock = buffer_read_i16(buf);
    scenario.allowed_buildings.wharf = buffer_read_i16(buf);
    scenario.allowed_buildings.governor_home = buffer_read_i16(buf);
    scenario.allowed_buildings.engineers_post = buffer_read_i16(buf);
    scenario.allowed_buildings.senate = buffer_read_i16(buf);
    scenario.allowed_buildings.forum = buffer_read_i16(buf);
    scenario.allowed_buildings.well = buffer_read_i16(buf);
    scenario.allowed_buildings.oracle = buffer_read_i16(buf);
    scenario.allowed_buildings.mission_post = buffer_read_i16(buf);
    scenario.allowed_buildings.bridge = buffer_read_i16(buf);
    scenario.allowed_buildings.barracks = buffer_read_i16(buf);
    scenario.allowed_buildings.military_academy = buffer_read_i16(buf);
    scenario.allowed_buildings.distribution_center = buffer_read_i16(buf);
    buffer_skip(buf, 2);

    // win criteria
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

    // map points
    scenario.earthquake_point.x = buffer_read_i16(buf);
    scenario.earthquake_point.y = buffer_read_i16(buf);
    scenario.entry_point.x = buffer_read_i16(buf);
    scenario.entry_point.y = buffer_read_i16(buf);
    scenario.exit_point.x = buffer_read_i16(buf);
    scenario.exit_point.y = buffer_read_i16(buf);

    for (int i = 0; i < MAX_INVASION_POINTS; i++)
    {
        scenario.invasion_points[i].x = buffer_read_i16(buf);
    }
    for (int i = 0; i < MAX_INVASION_POINTS; i++)
    {
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
    buffer_skip(buf, 1);
}

void scenario_settings_init()
{
    scenario.settings.campaign_mission = 0;
    scenario.settings.campaign_rank = 0;
    scenario.settings.is_custom = 0;
    scenario.settings.starting_favor = difficulty_starting_favor();
    scenario.settings.starting_personal_savings = 0;
}

void scenario_settings_init_mission()
{
    scenario.settings.starting_favor = difficulty_starting_favor();
    scenario.settings.starting_personal_savings = setting_personal_savings_for_mission(scenario.settings.campaign_rank);
}

void scenario_settings_save_state(buffer *part1, buffer *part2, buffer *part3, buffer *player_name)
{
    buffer_write_i32(part1, scenario.settings.campaign_mission);

    buffer_write_i32(part2, scenario.settings.starting_favor);
    buffer_write_i32(part2, scenario.settings.starting_personal_savings);
    buffer_write_i32(part2, scenario.settings.campaign_rank);

    buffer_write_i32(part3, scenario.settings.is_custom);

    for (int i = 0; i < MAX_PLAYER_NAME; i++)
    {
        buffer_write_u8(player_name, 0);
    }
    buffer_write_raw(player_name, scenario.settings.player_name, MAX_PLAYER_NAME);
}

void scenario_settings_load_state(buffer *part1, buffer *part2, buffer *part3, buffer *player_name)
{
    scenario.settings.campaign_mission = buffer_read_i32(part1);

    scenario.settings.starting_favor = buffer_read_i32(part2);
    scenario.settings.starting_personal_savings = buffer_read_i32(part2);
    scenario.settings.campaign_rank = buffer_read_i32(part2);

    scenario.settings.is_custom = buffer_read_i32(part3);

    buffer_skip(player_name, MAX_PLAYER_NAME);
    buffer_read_raw(player_name, scenario.settings.player_name, MAX_PLAYER_NAME);
}
