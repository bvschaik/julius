#include "data.h"

#include "city/constants.h"
#include "city/data_private.h"
#include "city/gods.h"
#include "game/difficulty.h"
#include "scenario/property.h"

#include <string.h>

void city_data_init(void)
{
    memset(&city_data, 0, sizeof(struct city_data_t));

    city_data.sentiment.value = 60;
    city_data.health.target_value = 50;
    city_data.health.value = 50;
    city_data.labor.wages_rome = 30;
    city_data.labor.wages = 30;
    city_data.finance.tax_percentage = 7;
    city_data.trade.caravan_import_resource = RESOURCE_MIN;
    city_data.trade.caravan_backup_import_resource = RESOURCE_MIN;
    city_data.population.monthly.next_index = 0;
    city_data.population.monthly.count = 0;
    city_data.festival.months_since_festival = 1;
    city_data.festival.selected.size = FESTIVAL_SMALL;
    city_data.emperor.gifts[GIFT_MODEST].cost = 0;
    city_data.emperor.gifts[GIFT_GENEROUS].cost = 0;
    city_data.emperor.gifts[GIFT_LAVISH].cost = 0;

    city_gods_reset();
}

void city_data_init_scenario(void)
{
    city_data.finance.treasury = difficulty_adjust_money(scenario_initial_funds());
    city_data.finance.last_year.balance = city_data.finance.treasury;
}

void city_data_init_campaign_mission(void)
{
    city_data.finance.treasury = difficulty_adjust_money(city_data.finance.treasury);
}

static void save_main_data(buffer *main)
{
    buffer_write_i32(main, city_data.finance.tax_percentage);
    buffer_write_i32(main, city_data.finance.treasury);
    buffer_write_i32(main, city_data.sentiment.value);
    buffer_write_i32(main, city_data.health.target_value);
    buffer_write_i32(main, city_data.health.value);
    buffer_write_i32(main, city_data.health.num_hospital_workers);
    buffer_write_i32(main, city_data.population.population);
    buffer_write_i32(main, city_data.population.population_last_year);
    buffer_write_i32(main, city_data.population.school_age);
    buffer_write_i32(main, city_data.population.academy_age);
    buffer_write_i32(main, city_data.population.total_capacity);
    buffer_write_i32(main, city_data.population.room_in_houses);
    for (int i = 0; i < 2400; i++) {
        buffer_write_i32(main, city_data.population.monthly.values[i]);
    }
    buffer_write_i32(main, city_data.population.monthly.next_index);
    buffer_write_i32(main, city_data.population.monthly.count);
    for (int i = 0; i < 100; i++) {
        buffer_write_i16(main, city_data.population.at_age[i]);
    }
    for (int i = 0; i < 20; i++) {
        buffer_write_i32(main, city_data.population.at_level[i]);
    }
    buffer_write_i32(main, city_data.population.yearly_births);
    buffer_write_i32(main, city_data.population.yearly_deaths);
    buffer_write_i32(main, city_data.population.lost_removal);
    buffer_write_i32(main, city_data.migration.immigration_amount_per_batch);
    buffer_write_i32(main, city_data.migration.emigration_amount_per_batch);
    buffer_write_i32(main, city_data.migration.emigration_queue_size);
    buffer_write_i32(main, city_data.migration.immigration_queue_size);
    buffer_write_i32(main, city_data.population.lost_homeless);
    buffer_write_i32(main, city_data.population.last_change);
    buffer_write_i32(main, city_data.population.average_per_year);
    buffer_write_i32(main, city_data.population.total_all_years);
    buffer_write_i32(main, city_data.population.people_in_tents_shacks);
    buffer_write_i32(main, city_data.population.people_in_villas_palaces);
    buffer_write_i32(main, city_data.population.total_years);
    buffer_write_i32(main, city_data.population.yearly_update_requested);
    buffer_write_i32(main, city_data.population.last_used_house_add);
    buffer_write_i32(main, city_data.population.last_used_house_remove);
    buffer_write_i32(main, city_data.migration.immigrated_today);
    buffer_write_i32(main, city_data.migration.emigrated_today);
    buffer_write_i32(main, city_data.migration.refused_immigrants_today);
    buffer_write_i32(main, city_data.migration.percentage);
    buffer_write_i32(main, city_data.culture.population_with_venus_access);
    buffer_write_i32(main, city_data.migration.immigration_duration);
    buffer_write_i32(main, city_data.migration.emigration_duration);
    buffer_write_i32(main, city_data.migration.newcomers);
    buffer_write_i32(main, city_data.culture.average_desirability);
    buffer_write_i32(main, city_data.finance.tourism_rating);
    buffer_write_i32(main, city_data.finance.tourism_last_month);
    buffer_write_i32(main, city_data.finance.tourism_last_year);
    buffer_write_i16(main, city_data.finance.tourism_this_year);
    buffer_write_i16(main, city_data.resource.last_used_warehouse);
    buffer_write_u16(main, city_data.trade.months_since_last_land_trade_problem);
    buffer_write_u16(main, city_data.trade.months_since_last_sea_trade_problem);
    for (int i = 0; i < RESOURCE_MAX; i++) {
        buffer_write_i16(main, city_data.resource.import_over[i]);
    }
    buffer_write_u8(main, city_data.map.entry_point.x);
    buffer_write_u8(main, city_data.map.entry_point.y);
    buffer_write_i16(main, city_data.map.entry_point.grid_offset);
    buffer_write_u8(main, city_data.map.exit_point.x);
    buffer_write_u8(main, city_data.map.exit_point.y);
    buffer_write_i16(main, city_data.map.exit_point.grid_offset);
    buffer_write_u8(main, city_data.building.senate_x);
    buffer_write_u8(main, city_data.building.senate_y);
    buffer_write_i16(main, city_data.building.senate_grid_offset);
    buffer_write_i32(main, city_data.building.senate_building_id);
    buffer_write_u8(main, city_data.trade.land_policy);
    buffer_write_u8(main, city_data.trade.sea_policy);
    for (int i = 0; i < RESOURCE_MAX; i++) {
        buffer_write_i16(main, city_data.resource.space_in_warehouses[i]);
    }
    for (int i = 0; i < RESOURCE_MAX; i++) {
        buffer_write_i16(main, city_data.resource.stored_in_warehouses[i]);
    }
    for (int i = 0; i < RESOURCE_MAX; i++) {
        buffer_write_i16(main, city_data.resource.trade_status[i]);
    }
    for (int i = 0; i < RESOURCE_MAX; i++) {
        buffer_write_i16(main, city_data.resource.export_over[i]);
    }
    for (int i = 0; i < RESOURCE_MAX; i++) {
        buffer_write_i16(main, city_data.resource.mothballed[i]);
    }
    for (int i = 0; i < RESOURCE_MAX_FOOD; i++) {
        buffer_write_i32(main, city_data.resource.granary_food_stored[i]);
    }
    for (int i = 0; i < 6; i++) {
        buffer_write_i32(main, city_data.resource.stored_in_workshops[i]);
    }
    for (int i = 0; i < 6; i++) {
        buffer_write_i32(main, city_data.resource.space_in_workshops[i]);
    }
    buffer_write_i32(main, city_data.resource.granary_total_stored);
    buffer_write_i32(main, city_data.resource.food_types_available);
    buffer_write_i32(main, city_data.resource.food_types_eaten);
    for (int i = 0; i < RESOURCE_MAX; i++) {
        buffer_write_i16(main, city_data.resource.export_status_before_stockpiling[i]);
    }
    buffer_write_i8(main, city_data.sentiment.crime_cooldown);
    buffer_write_i32(main, city_data.building.caravanserai_building_id);
    buffer_write_i32(main, city_data.caravanserai.total_food);
    for (int i = 0; i < RESOURCE_MAX; i++) {
        buffer_write_i32(main, city_data.resource.stockpiled[i]);
    }
    buffer_write_i32(main, city_data.resource.food_supply_months);
    buffer_write_i32(main, city_data.resource.granaries.operating);
    buffer_write_i32(main, city_data.population.percentage_plebs);
    buffer_write_i32(main, city_data.population.working_age);
    buffer_write_i32(main, city_data.labor.workers_available);
    for (int i = 0; i < 10; i++) {
        buffer_write_i32(main, city_data.labor.categories[i].workers_needed);
        buffer_write_i32(main, city_data.labor.categories[i].workers_allocated);
        buffer_write_i32(main, city_data.labor.categories[i].total_houses_covered);
        buffer_write_i32(main, city_data.labor.categories[i].buildings);
        buffer_write_i32(main, city_data.labor.categories[i].priority);
    }
    buffer_write_i32(main, city_data.labor.workers_employed);
    buffer_write_i32(main, city_data.labor.workers_unemployed);
    buffer_write_i32(main, city_data.labor.unemployment_percentage);
    buffer_write_i32(main, city_data.labor.unemployment_percentage_for_senate);
    buffer_write_i32(main, city_data.labor.workers_needed);
    buffer_write_i32(main, city_data.labor.wages);
    buffer_write_i32(main, city_data.labor.wages_rome);
    buffer_write_i32(main, city_data.labor.months_since_last_wage_change);
    buffer_write_i32(main, city_data.finance.wages_so_far);
    buffer_write_i32(main, city_data.finance.this_year.expenses.wages);
    buffer_write_i32(main, city_data.finance.last_year.expenses.wages);
    buffer_write_i32(main, city_data.taxes.taxed_plebs);
    buffer_write_i32(main, city_data.taxes.taxed_patricians);
    buffer_write_i32(main, city_data.taxes.untaxed_plebs);
    buffer_write_i32(main, city_data.taxes.untaxed_patricians);
    buffer_write_i32(main, city_data.taxes.percentage_taxed_plebs);
    buffer_write_i32(main, city_data.taxes.percentage_taxed_patricians);
    buffer_write_i32(main, city_data.taxes.percentage_taxed_people);
    buffer_write_i32(main, city_data.taxes.yearly.collected_plebs);
    buffer_write_i32(main, city_data.taxes.yearly.collected_patricians);
    buffer_write_i32(main, city_data.taxes.yearly.uncollected_plebs);
    buffer_write_i32(main, city_data.taxes.yearly.uncollected_patricians);
    buffer_write_i32(main, city_data.finance.this_year.income.taxes);
    buffer_write_i32(main, city_data.finance.last_year.income.taxes);
    buffer_write_i32(main, city_data.taxes.monthly.collected_plebs);
    buffer_write_i32(main, city_data.taxes.monthly.uncollected_plebs);
    buffer_write_i32(main, city_data.taxes.monthly.collected_patricians);
    buffer_write_i32(main, city_data.taxes.monthly.uncollected_patricians);
    buffer_write_i32(main, city_data.finance.this_year.income.exports);
    buffer_write_i32(main, city_data.finance.last_year.income.exports);
    buffer_write_i32(main, city_data.finance.this_year.expenses.imports);
    buffer_write_i32(main, city_data.finance.last_year.expenses.imports);
    buffer_write_i32(main, city_data.finance.interest_so_far);
    buffer_write_i32(main, city_data.finance.last_year.expenses.interest);
    buffer_write_i32(main, city_data.finance.this_year.expenses.interest);
    buffer_write_i32(main, city_data.finance.last_year.expenses.sundries);
    buffer_write_i32(main, city_data.finance.this_year.expenses.sundries);
    buffer_write_i32(main, city_data.finance.last_year.expenses.construction);
    buffer_write_i32(main, city_data.finance.this_year.expenses.construction);
    buffer_write_i32(main, city_data.finance.last_year.expenses.salary);
    buffer_write_i32(main, city_data.finance.this_year.expenses.salary);
    buffer_write_i32(main, city_data.emperor.salary_amount);
    buffer_write_i32(main, city_data.emperor.salary_rank);
    buffer_write_i32(main, city_data.finance.salary_so_far);
    buffer_write_i32(main, city_data.finance.last_year.income.total);
    buffer_write_i32(main, city_data.finance.this_year.income.total);
    buffer_write_i32(main, city_data.finance.last_year.expenses.total);
    buffer_write_i32(main, city_data.finance.this_year.expenses.total);
    buffer_write_i32(main, city_data.finance.last_year.net_in_out);
    buffer_write_i32(main, city_data.finance.this_year.net_in_out);
    buffer_write_i32(main, city_data.finance.last_year.balance);
    buffer_write_i32(main, city_data.finance.this_year.balance);
    buffer_write_i32(main, city_data.trade.caravan_import_resource);
    buffer_write_i32(main, city_data.trade.caravan_backup_import_resource);
    buffer_write_i32(main, city_data.ratings.culture);
    buffer_write_i32(main, city_data.ratings.prosperity);
    buffer_write_i32(main, city_data.ratings.peace);
    buffer_write_i32(main, city_data.ratings.favor);
    buffer_write_i32(main, city_data.finance.levies_so_far);
    buffer_write_i32(main, city_data.finance.this_year.expenses.levies);
    buffer_write_i32(main, city_data.finance.last_year.expenses.levies);
    buffer_write_i32(main, city_data.ratings.prosperity_treasury_last_year);
    buffer_write_i32(main, city_data.ratings.culture_points.theater);
    buffer_write_i32(main, city_data.ratings.culture_points.religion);
    buffer_write_i32(main, city_data.ratings.culture_points.school);
    buffer_write_i32(main, city_data.ratings.culture_points.library);
    buffer_write_i32(main, city_data.ratings.culture_points.academy);
    buffer_write_i32(main, city_data.ratings.peace_num_criminals);
    buffer_write_i32(main, city_data.ratings.peace_num_rioters);
    buffer_write_i32(main, city_data.houses.missing.fountain);
    buffer_write_i32(main, city_data.houses.missing.well);
    buffer_write_i32(main, city_data.houses.missing.more_entertainment);
    buffer_write_i32(main, city_data.houses.missing.more_education);
    buffer_write_i32(main, city_data.houses.missing.education);
    buffer_write_i32(main, city_data.houses.requiring.school);
    buffer_write_i32(main, city_data.houses.requiring.library);
    buffer_write_i32(main, city_data.games.bet_amount);
    buffer_write_i32(main, city_data.houses.missing.barber);
    buffer_write_i32(main, city_data.houses.missing.bathhouse);
    buffer_write_i32(main, city_data.houses.missing.food);
    buffer_write_i32(main, city_data.building.hippodrome_placed);
    buffer_write_i32(main, city_data.houses.missing.clinic);
    buffer_write_i32(main, city_data.houses.missing.hospital);
    buffer_write_i32(main, city_data.houses.requiring.barber);
    buffer_write_i32(main, city_data.houses.requiring.bathhouse);
    buffer_write_i32(main, city_data.houses.requiring.clinic);
    buffer_write_i32(main, city_data.houses.missing.religion);
    buffer_write_i32(main, city_data.houses.missing.second_religion);
    buffer_write_i32(main, city_data.houses.missing.third_religion);
    buffer_write_i32(main, city_data.houses.requiring.religion);
    buffer_write_i32(main, city_data.entertainment.theater_shows);
    buffer_write_i32(main, city_data.entertainment.theater_no_shows_weighted);
    buffer_write_i32(main, city_data.entertainment.amphitheater_shows);
    buffer_write_i32(main, city_data.entertainment.amphitheater_no_shows_weighted);
    buffer_write_i32(main, city_data.entertainment.colosseum_shows);
    buffer_write_i32(main, city_data.entertainment.colosseum_no_shows_weighted);
    buffer_write_i32(main, city_data.entertainment.hippodrome_shows);
    buffer_write_i32(main, city_data.entertainment.hippodrome_no_shows_weighted);
    buffer_write_i32(main, city_data.entertainment.venue_needing_shows);
    buffer_write_i32(main, city_data.culture.average_entertainment);
    buffer_write_i32(main, city_data.houses.missing.entertainment);
    buffer_write_i32(main, city_data.festival.months_since_festival);
    for (int i = 0; i < MAX_GODS; i++) {
        buffer_write_i8(main, city_data.religion.gods[i].target_happiness);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        buffer_write_i8(main, city_data.religion.gods[i].happiness);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        buffer_write_i8(main, city_data.religion.gods[i].wrath_bolts);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        buffer_write_i8(main, city_data.religion.gods[i].blessing_done);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        buffer_write_i8(main, city_data.religion.gods[i].small_curse_done);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        buffer_write_i8(main, city_data.religion.gods[i].happy_bolts);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        buffer_write_i32(main, city_data.religion.gods[i].months_since_festival);
    }
    buffer_write_i32(main, city_data.religion.least_happy_god);
    buffer_write_i32(main, city_data.migration.no_immigration_cause);
    buffer_write_i32(main, city_data.sentiment.protesters);
    buffer_write_i32(main, city_data.sentiment.criminals);
    buffer_write_i32(main, city_data.houses.health);
    buffer_write_i32(main, city_data.houses.religion);
    buffer_write_i32(main, city_data.houses.education);
    buffer_write_i32(main, city_data.houses.entertainment);
    buffer_write_i32(main, city_data.figure.rioters);
    buffer_write_i32(main, city_data.ratings.selected);
    buffer_write_i32(main, city_data.ratings.culture_explanation);
    buffer_write_i32(main, city_data.ratings.prosperity_explanation);
    buffer_write_i32(main, city_data.ratings.peace_explanation);
    buffer_write_i32(main, city_data.ratings.favor_explanation);
    buffer_write_i32(main, city_data.emperor.player_rank);
    buffer_write_i32(main, city_data.emperor.personal_savings);
    buffer_write_i32(main, city_data.finance.last_year.income.donated);
    buffer_write_i32(main, city_data.finance.this_year.income.donated);
    buffer_write_i32(main, city_data.emperor.donate_amount);
    for (int i = 0; i < 10; i++) {
        buffer_write_i16(main, city_data.building.working_dock_ids[i]);
    }
    buffer_write_u16(main, city_data.building.months_since_last_destroyed_iron_mine);
    buffer_write_u16(main, city_data.building.months_since_last_flooded_clay_pit);
    buffer_write_i16(main, city_data.sentiment.blessing_festival_boost);
    buffer_write_i16(main, city_data.figure.animals);
    buffer_write_i16(main, city_data.trade.num_sea_routes);
    buffer_write_i16(main, city_data.trade.num_land_routes);
    buffer_write_i16(main, city_data.trade.sea_trade_problem_duration);
    buffer_write_i16(main, city_data.trade.land_trade_problem_duration);
    buffer_write_i16(main, city_data.building.working_docks);
    buffer_write_i16(main, city_data.building.senate_placed);
    buffer_write_i16(main, city_data.building.working_wharfs);
    buffer_write_i16(main, city_data.finance.stolen_this_year);
    buffer_write_i16(main, city_data.finance.stolen_last_year);
    buffer_write_i32(main, city_data.trade.docker_import_resource);
    buffer_write_i32(main, city_data.trade.docker_export_resource);
    buffer_write_i32(main, city_data.emperor.debt_state);
    buffer_write_i32(main, city_data.emperor.months_in_debt);
    buffer_write_i32(main, city_data.finance.cheated_money);
    buffer_write_i8(main, city_data.building.barracks_x);
    buffer_write_i8(main, city_data.building.barracks_y);
    buffer_write_i16(main, city_data.building.barracks_grid_offset);
    buffer_write_i32(main, city_data.building.barracks_building_id);
    buffer_write_i32(main, city_data.building.barracks_placed);
    buffer_write_i32(main, city_data.building.mess_hall_building_id);
    buffer_write_i32(main, city_data.entertainment.arena_shows);
    buffer_write_i32(main, city_data.entertainment.arena_no_shows_weighted);
    buffer_write_i32(main, city_data.population.lost_troop_request);
    buffer_write_i32(main, city_data.mission.has_won);
    buffer_write_i32(main, city_data.mission.continue_months_left);
    buffer_write_i32(main, city_data.mission.continue_months_chosen);
    buffer_write_i32(main, city_data.finance.wage_rate_paid_this_year);
    buffer_write_i32(main, city_data.finance.this_year.expenses.tribute);
    buffer_write_i32(main, city_data.finance.last_year.expenses.tribute);
    buffer_write_i32(main, city_data.finance.tribute_not_paid_last_year);
    buffer_write_i32(main, city_data.finance.tribute_not_paid_total_years);
    buffer_write_i32(main, city_data.festival.selected.god);
    buffer_write_i32(main, city_data.festival.selected.size);
    buffer_write_i32(main, city_data.festival.planned.size);
    buffer_write_i32(main, city_data.festival.planned.months_to_go);
    buffer_write_i32(main, city_data.festival.planned.god);
    buffer_write_i32(main, city_data.festival.small_cost);
    buffer_write_i32(main, city_data.festival.large_cost);
    buffer_write_i32(main, city_data.festival.grand_cost);
    buffer_write_i32(main, city_data.festival.grand_wine);
    buffer_write_i32(main, city_data.festival.not_enough_wine);
    buffer_write_i32(main, city_data.culture.average_religion);
    buffer_write_i32(main, city_data.culture.average_education);
    buffer_write_i32(main, city_data.culture.average_health);
    buffer_write_i32(main, city_data.culture.religion_coverage);
    buffer_write_i32(main, city_data.festival.first_festival_effect_months);
    buffer_write_i32(main, city_data.festival.second_festival_effect_months);
    buffer_write_i32(main, city_data.sentiment.unemployment);
    buffer_write_i32(main, city_data.sentiment.previous_value);
    buffer_write_i32(main, city_data.sentiment.message_delay);
    buffer_write_i32(main, city_data.sentiment.low_mood_cause);
    buffer_write_i32(main, city_data.figure.security_breach_duration);
    buffer_write_i32(main, city_data.health.population_access.clinic);
    buffer_write_i32(main, city_data.health.population_access.baths);
    buffer_write_i32(main, city_data.health.population_access.barber);
    buffer_write_i32(main, city_data.health.months_since_last_contaminated_water);
    buffer_write_i32(main, city_data.emperor.selected_gift_size);
    buffer_write_i32(main, city_data.emperor.months_since_gift);
    buffer_write_i32(main, city_data.emperor.gift_overdose_penalty);
    buffer_write_i32(main, city_data.emperor.gifts[GIFT_MODEST].id);
    buffer_write_i32(main, city_data.emperor.gifts[GIFT_GENEROUS].id);
    buffer_write_i32(main, city_data.emperor.gifts[GIFT_LAVISH].id);
    buffer_write_i32(main, city_data.emperor.gifts[GIFT_MODEST].cost);
    buffer_write_i32(main, city_data.emperor.gifts[GIFT_GENEROUS].cost);
    buffer_write_i32(main, city_data.emperor.gifts[GIFT_LAVISH].cost);
    buffer_write_i32(main, city_data.ratings.favor_salary_penalty);
    buffer_write_i32(main, city_data.ratings.favor_milestone_penalty);
    buffer_write_i32(main, city_data.ratings.favor_ignored_request_penalty);
    buffer_write_i32(main, city_data.ratings.favor_last_year);
    buffer_write_i32(main, city_data.ratings.favor_change);
    buffer_write_i32(main, city_data.military.native_attack_duration);
    buffer_write_i32(main, city_data.building.mission_post_operational);
    buffer_write_i32(main, city_data.building.main_native_meeting.x);
    buffer_write_i32(main, city_data.building.main_native_meeting.y);
    buffer_write_i32(main, city_data.finance.wage_rate_paid_last_year);
    buffer_write_i32(main, city_data.resource.food_needed_per_month);
    buffer_write_i32(main, city_data.resource.granaries.understaffed);
    buffer_write_i32(main, city_data.resource.granaries.not_operating);
    buffer_write_i32(main, city_data.resource.granaries.not_operating_with_food);
    buffer_write_i32(main, city_data.religion.venus_blessing_months_left);
    buffer_write_i32(main, city_data.religion.venus_curse_active);
    buffer_write_i32(main, city_data.building.num_striking_industries);
    buffer_write_i32(main, city_data.religion.neptune_double_trade_active);
    buffer_write_i32(main, city_data.religion.mars_spirit_power);
    buffer_write_i32(main, city_data.religion.angry_message_delay);
    buffer_write_i32(main, city_data.resource.food_consumed_last_month);
    buffer_write_i32(main, city_data.resource.food_produced_last_month);
    buffer_write_i32(main, city_data.resource.food_produced_this_month);
    buffer_write_i32(main, city_data.ratings.peace_riot_cause);
    buffer_write_i32(main, city_data.finance.estimated_tax_income);
    buffer_write_i32(main, city_data.mission.tutorial_senate_built);
    buffer_write_i8(main, city_data.building.distribution_center_x);
    buffer_write_i8(main, city_data.building.distribution_center_y);
    buffer_write_i16(main, city_data.building.distribution_center_grid_offset);
    buffer_write_i32(main, city_data.building.distribution_center_building_id);
    buffer_write_i32(main, city_data.building.distribution_center_placed);
    buffer_write_i32(main, city_data.mess_hall.food_types);
    buffer_write_i32(main, city_data.mess_hall.food_stress_cumulative);
    buffer_write_i32(main, city_data.mess_hall.mess_hall_warning_shown);
    buffer_write_i32(main, city_data.mess_hall.food_percentage_missing_this_month);
    buffer_write_i32(main, city_data.mess_hall.total_food);
    buffer_write_i32(main, city_data.mess_hall.missing_mess_hall_warning_shown);
    buffer_write_i32(main, city_data.military.soldiers_in_city);
    buffer_write_i32(main, city_data.games.naval_battle_distant_battle_bonus);
    buffer_write_i32(main, city_data.figure.looters);
    buffer_write_i32(main, city_data.figure.robbers);
    buffer_write_i32(main, city_data.figure.protesters);
    buffer_write_i32(main, city_data.building.shipyard_boats_requested);
    buffer_write_i32(main, city_data.figure.enemies);
    buffer_write_i32(main, city_data.sentiment.wages);
    buffer_write_i32(main, city_data.population.people_in_tents);
    buffer_write_i32(main, city_data.population.people_in_large_insula_and_above);
    buffer_write_i32(main, city_data.figure.imperial_soldiers);
    buffer_write_i32(main, city_data.emperor.invasion.duration_day_countdown);
    buffer_write_i32(main, city_data.emperor.invasion.warnings_given);
    buffer_write_i32(main, city_data.emperor.invasion.days_until_invasion);
    buffer_write_i32(main, city_data.emperor.invasion.retreat_message_shown);
    buffer_write_i32(main, city_data.ratings.peace_destroyed_buildings);
    buffer_write_i32(main, city_data.ratings.peace_years_of_peace);
    buffer_write_u8(main, city_data.distant_battle.city);
    buffer_write_u8(main, city_data.distant_battle.enemy_strength);
    buffer_write_u8(main, city_data.distant_battle.roman_strength);
    buffer_write_i8(main, city_data.distant_battle.months_until_battle);
    buffer_write_i8(main, city_data.distant_battle.roman_months_to_travel_back);
    buffer_write_i8(main, city_data.distant_battle.roman_months_to_travel_forth);
    buffer_write_i8(main, city_data.distant_battle.city_foreign_months_left);
    buffer_write_i8(main, city_data.building.triumphal_arches_available);
    buffer_write_i8(main, city_data.distant_battle.total_count);
    buffer_write_i8(main, city_data.distant_battle.won_count);
    buffer_write_i8(main, city_data.distant_battle.enemy_months_traveled);
    buffer_write_i8(main, city_data.distant_battle.roman_months_traveled);
    buffer_write_u8(main, city_data.military.total_legions);
    buffer_write_u8(main, city_data.military.empire_service_legions);
    buffer_write_u8(main, city_data.games.chosen_horse);
    buffer_write_u8(main, city_data.military.total_soldiers);
    buffer_write_i8(main, city_data.building.triumphal_arches_placed);
    buffer_write_i8(main, city_data.sound.die_citizen);
    buffer_write_i8(main, city_data.sound.die_soldier);
    buffer_write_i8(main, city_data.sound.shoot_arrow);
    buffer_write_i32(main, city_data.building.trade_center_building_id);
    buffer_write_i32(main, city_data.figure.soldiers);
    buffer_write_i8(main, city_data.sound.hit_soldier);
    buffer_write_i8(main, city_data.sound.hit_spear);
    buffer_write_i8(main, city_data.sound.hit_club);
    buffer_write_i8(main, city_data.sound.march_enemy);
    buffer_write_i8(main, city_data.sound.march_horse);
    buffer_write_i8(main, city_data.sound.hit_elephant);
    buffer_write_i8(main, city_data.sound.hit_axe);
    buffer_write_i8(main, city_data.sound.hit_wolf);
    buffer_write_i8(main, city_data.sound.march_wolf);
    buffer_write_i8(main, city_data.sentiment.include_tents);
    buffer_write_i32(main, city_data.emperor.invasion.count);
    buffer_write_i32(main, city_data.emperor.invasion.size);
    buffer_write_i32(main, city_data.emperor.invasion.soldiers_killed);
    buffer_write_i32(main, city_data.military.legionary_legions);
    buffer_write_i32(main, city_data.population.highest_ever);
    buffer_write_i32(main, city_data.finance.estimated_wages);
    buffer_write_i32(main, city_data.resource.wine_types_available);
    buffer_write_i32(main, city_data.ratings.prosperity_max);
    for (int i = 0; i < 10; i++) {
        buffer_write_i32(main, city_data.map.largest_road_networks[i].id);
        buffer_write_i32(main, city_data.map.largest_road_networks[i].size);
    }
    buffer_write_i32(main, city_data.houses.missing.second_wine);
    buffer_write_i32(main, city_data.religion.neptune_sank_ships);
    buffer_write_i32(main, city_data.entertainment.hippodrome_has_race);
    buffer_write_i32(main, city_data.entertainment.hippodrome_message_shown);
    buffer_write_i32(main, city_data.entertainment.colosseum_message_shown);
    buffer_write_i32(main, city_data.migration.emigration_message_shown);
    buffer_write_i32(main, city_data.mission.fired_message_shown);
    buffer_write_i32(main, city_data.mission.victory_message_shown);
    buffer_write_i32(main, city_data.mission.start_saved_game_written);
    buffer_write_i32(main, city_data.mission.tutorial_fire_message_shown);
    buffer_write_i32(main, city_data.mission.tutorial_disease_message_shown);
    buffer_write_i32(main, city_data.figure.attacking_natives);
}

static void load_main_data(buffer *main, int has_separate_import_limits, int discard_unused_values)
{
    if (!discard_unused_values) {
        buffer_skip(main, 18076);
    }
    city_data.finance.tax_percentage = buffer_read_i32(main);
    city_data.finance.treasury = buffer_read_i32(main);
    city_data.sentiment.value = buffer_read_i32(main);
    city_data.health.target_value = buffer_read_i32(main);
    city_data.health.value = buffer_read_i32(main);
    city_data.health.num_hospital_workers = buffer_read_i32(main);
    if (!discard_unused_values) {
        buffer_skip(main, 4);
    }
    city_data.population.population = buffer_read_i32(main);
    city_data.population.population_last_year = buffer_read_i32(main);
    city_data.population.school_age = buffer_read_i32(main);
    city_data.population.academy_age = buffer_read_i32(main);
    city_data.population.total_capacity = buffer_read_i32(main);
    city_data.population.room_in_houses = buffer_read_i32(main);
    for (int i = 0; i < 2400; i++) {
        city_data.population.monthly.values[i] = buffer_read_i32(main);
    }
    city_data.population.monthly.next_index = buffer_read_i32(main);
    city_data.population.monthly.count = buffer_read_i32(main);
    for (int i = 0; i < 100; i++) {
        city_data.population.at_age[i] = buffer_read_i16(main);
    }
    for (int i = 0; i < 20; i++) {
        city_data.population.at_level[i] = buffer_read_i32(main);
    }
    city_data.population.yearly_births = buffer_read_i32(main);
    city_data.population.yearly_deaths = buffer_read_i32(main);
    city_data.population.lost_removal = buffer_read_i32(main);
    city_data.migration.immigration_amount_per_batch = buffer_read_i32(main);
    city_data.migration.emigration_amount_per_batch = buffer_read_i32(main);
    city_data.migration.emigration_queue_size = buffer_read_i32(main);
    city_data.migration.immigration_queue_size = buffer_read_i32(main);
    city_data.population.lost_homeless = buffer_read_i32(main);
    city_data.population.last_change = buffer_read_i32(main);
    city_data.population.average_per_year = buffer_read_i32(main);
    city_data.population.total_all_years = buffer_read_i32(main);
    city_data.population.people_in_tents_shacks = buffer_read_i32(main);
    city_data.population.people_in_villas_palaces = buffer_read_i32(main);
    city_data.population.total_years = buffer_read_i32(main);
    city_data.population.yearly_update_requested = buffer_read_i32(main);
    city_data.population.last_used_house_add = buffer_read_i32(main);
    city_data.population.last_used_house_remove = buffer_read_i32(main);
    city_data.migration.immigrated_today = buffer_read_i32(main);
    city_data.migration.emigrated_today = buffer_read_i32(main);
    city_data.migration.refused_immigrants_today = buffer_read_i32(main);
    city_data.migration.percentage = buffer_read_i32(main);
    city_data.culture.population_with_venus_access = buffer_read_i32(main);
    city_data.migration.immigration_duration = buffer_read_i32(main);
    city_data.migration.emigration_duration = buffer_read_i32(main);
    city_data.migration.newcomers = buffer_read_i32(main);
    city_data.culture.average_desirability = buffer_read_i32(main);
    city_data.finance.tourism_rating = buffer_read_i32(main);
    city_data.finance.tourism_last_month = buffer_read_i32(main);
    city_data.finance.tourism_last_year = buffer_read_i32(main);
    city_data.finance.tourism_this_year = buffer_read_i16(main);
    city_data.resource.last_used_warehouse = buffer_read_i16(main);
    city_data.trade.months_since_last_land_trade_problem = buffer_read_u16(main);
    city_data.trade.months_since_last_sea_trade_problem = buffer_read_u16(main);
    if (has_separate_import_limits) {
        for (int i = 0; i < RESOURCE_MAX; i++) {
            city_data.resource.import_over[i] = buffer_read_i16(main);
        }
    } else {
        buffer_skip(main, 32);
    }
    city_data.map.entry_point.x = buffer_read_u8(main);
    city_data.map.entry_point.y = buffer_read_u8(main);
    city_data.map.entry_point.grid_offset = buffer_read_i16(main);
    city_data.map.exit_point.x = buffer_read_u8(main);
    city_data.map.exit_point.y = buffer_read_u8(main);
    city_data.map.exit_point.grid_offset = buffer_read_i16(main);
    city_data.building.senate_x = buffer_read_u8(main);
    city_data.building.senate_y = buffer_read_u8(main);
    city_data.building.senate_grid_offset = buffer_read_i16(main);
    city_data.building.senate_building_id = buffer_read_i32(main);
    city_data.trade.land_policy = buffer_read_u8(main);
    city_data.trade.sea_policy = buffer_read_u8(main);
    for (int i = 0; i < RESOURCE_MAX; i++) {
        city_data.resource.space_in_warehouses[i] = buffer_read_i16(main);
    }
    for (int i = 0; i < RESOURCE_MAX; i++) {
        city_data.resource.stored_in_warehouses[i] = buffer_read_i16(main);
    }
    for (int i = 0; i < RESOURCE_MAX; i++) {
        city_data.resource.trade_status[i] = buffer_read_i16(main);
    }
    for (int i = 0; i < RESOURCE_MAX; i++) {
        city_data.resource.export_over[i] = buffer_read_i16(main);
    }
    for (int i = 0; i < RESOURCE_MAX; i++) {
        city_data.resource.mothballed[i] = buffer_read_i16(main);
    }
    if (!discard_unused_values) {
        buffer_skip(main, 2);
    }
    for (int i = 0; i < RESOURCE_MAX_FOOD; i++) {
        city_data.resource.granary_food_stored[i] = buffer_read_i32(main);
    }
    for (int i = 0; i < 6; i++) {
        city_data.resource.stored_in_workshops[i] = buffer_read_i32(main);
    }
    for (int i = 0; i < 6; i++) {
        city_data.resource.space_in_workshops[i] = buffer_read_i32(main);
    }
    city_data.resource.granary_total_stored = buffer_read_i32(main);
    city_data.resource.food_types_available = buffer_read_i32(main);
    city_data.resource.food_types_eaten = buffer_read_i32(main);
    for (int i = 0; i < RESOURCE_MAX; i++) {
        city_data.resource.export_status_before_stockpiling[i] = buffer_read_i16(main);
    }
    if (!discard_unused_values) {
        buffer_skip(main, 231);
    }
    city_data.sentiment.crime_cooldown = buffer_read_i8(main); 
    city_data.building.caravanserai_building_id = buffer_read_i32(main);
    city_data.caravanserai.total_food = buffer_read_i32(main);
    for (int i = 0; i < RESOURCE_MAX; i++) {
        city_data.resource.stockpiled[i] = buffer_read_i32(main);
    }
    city_data.resource.food_supply_months = buffer_read_i32(main);
    city_data.resource.granaries.operating = buffer_read_i32(main);
    city_data.population.percentage_plebs = buffer_read_i32(main);
    city_data.population.working_age = buffer_read_i32(main);
    city_data.labor.workers_available = buffer_read_i32(main);
    for (int i = 0; i < 10; i++) {
        city_data.labor.categories[i].workers_needed = buffer_read_i32(main);
        city_data.labor.categories[i].workers_allocated = buffer_read_i32(main);
        city_data.labor.categories[i].total_houses_covered = buffer_read_i32(main);
        city_data.labor.categories[i].buildings = buffer_read_i32(main);
        city_data.labor.categories[i].priority = buffer_read_i32(main);
    }
    city_data.labor.workers_employed = buffer_read_i32(main);
    city_data.labor.workers_unemployed = buffer_read_i32(main);
    city_data.labor.unemployment_percentage = buffer_read_i32(main);
    city_data.labor.unemployment_percentage_for_senate = buffer_read_i32(main);
    city_data.labor.workers_needed = buffer_read_i32(main);
    city_data.labor.wages = buffer_read_i32(main);
    city_data.labor.wages_rome = buffer_read_i32(main);
    city_data.labor.months_since_last_wage_change = buffer_read_i32(main);
    city_data.finance.wages_so_far = buffer_read_i32(main);
    city_data.finance.this_year.expenses.wages = buffer_read_i32(main);
    city_data.finance.last_year.expenses.wages = buffer_read_i32(main);
    city_data.taxes.taxed_plebs = buffer_read_i32(main);
    city_data.taxes.taxed_patricians = buffer_read_i32(main);
    city_data.taxes.untaxed_plebs = buffer_read_i32(main);
    city_data.taxes.untaxed_patricians = buffer_read_i32(main);
    city_data.taxes.percentage_taxed_plebs = buffer_read_i32(main);
    city_data.taxes.percentage_taxed_patricians = buffer_read_i32(main);
    city_data.taxes.percentage_taxed_people = buffer_read_i32(main);
    city_data.taxes.yearly.collected_plebs = buffer_read_i32(main);
    city_data.taxes.yearly.collected_patricians = buffer_read_i32(main);
    city_data.taxes.yearly.uncollected_plebs = buffer_read_i32(main);
    city_data.taxes.yearly.uncollected_patricians = buffer_read_i32(main);
    city_data.finance.this_year.income.taxes = buffer_read_i32(main);
    city_data.finance.last_year.income.taxes = buffer_read_i32(main);
    city_data.taxes.monthly.collected_plebs = buffer_read_i32(main);
    city_data.taxes.monthly.uncollected_plebs = buffer_read_i32(main);
    city_data.taxes.monthly.collected_patricians = buffer_read_i32(main);
    city_data.taxes.monthly.uncollected_patricians = buffer_read_i32(main);
    city_data.finance.this_year.income.exports = buffer_read_i32(main);
    city_data.finance.last_year.income.exports = buffer_read_i32(main);
    city_data.finance.this_year.expenses.imports = buffer_read_i32(main);
    city_data.finance.last_year.expenses.imports = buffer_read_i32(main);
    city_data.finance.interest_so_far = buffer_read_i32(main);
    city_data.finance.last_year.expenses.interest = buffer_read_i32(main);
    city_data.finance.this_year.expenses.interest = buffer_read_i32(main);
    city_data.finance.last_year.expenses.sundries = buffer_read_i32(main);
    city_data.finance.this_year.expenses.sundries = buffer_read_i32(main);
    city_data.finance.last_year.expenses.construction = buffer_read_i32(main);
    city_data.finance.this_year.expenses.construction = buffer_read_i32(main);
    city_data.finance.last_year.expenses.salary = buffer_read_i32(main);
    city_data.finance.this_year.expenses.salary = buffer_read_i32(main);
    city_data.emperor.salary_amount = buffer_read_i32(main);
    city_data.emperor.salary_rank = buffer_read_i32(main);
    city_data.finance.salary_so_far = buffer_read_i32(main);
    city_data.finance.last_year.income.total = buffer_read_i32(main);
    city_data.finance.this_year.income.total = buffer_read_i32(main);
    city_data.finance.last_year.expenses.total = buffer_read_i32(main);
    city_data.finance.this_year.expenses.total = buffer_read_i32(main);
    city_data.finance.last_year.net_in_out = buffer_read_i32(main);
    city_data.finance.this_year.net_in_out = buffer_read_i32(main);
    city_data.finance.last_year.balance = buffer_read_i32(main);
    city_data.finance.this_year.balance = buffer_read_i32(main);
    if (!discard_unused_values) {
        buffer_skip(main, 5632);
    }
    city_data.trade.caravan_import_resource = buffer_read_i32(main);
    city_data.trade.caravan_backup_import_resource = buffer_read_i32(main);
    city_data.ratings.culture = buffer_read_i32(main);
    city_data.ratings.prosperity = buffer_read_i32(main);
    city_data.ratings.peace = buffer_read_i32(main);
    city_data.ratings.favor = buffer_read_i32(main);
    city_data.finance.levies_so_far = buffer_read_i32(main);
    city_data.finance.this_year.expenses.levies = buffer_read_i32(main);
    city_data.finance.last_year.expenses.levies = buffer_read_i32(main);
    if (!discard_unused_values) {
        buffer_skip(main, 4);
    }
    city_data.ratings.prosperity_treasury_last_year = buffer_read_i32(main);
    city_data.ratings.culture_points.theater = buffer_read_i32(main);
    city_data.ratings.culture_points.religion = buffer_read_i32(main);
    city_data.ratings.culture_points.school = buffer_read_i32(main);
    city_data.ratings.culture_points.library = buffer_read_i32(main);
    city_data.ratings.culture_points.academy = buffer_read_i32(main);
    city_data.ratings.peace_num_criminals = buffer_read_i32(main);
    city_data.ratings.peace_num_rioters = buffer_read_i32(main);
    city_data.houses.missing.fountain = buffer_read_i32(main);
    city_data.houses.missing.well = buffer_read_i32(main);
    city_data.houses.missing.more_entertainment = buffer_read_i32(main);
    city_data.houses.missing.more_education = buffer_read_i32(main);
    city_data.houses.missing.education = buffer_read_i32(main);
    city_data.houses.requiring.school = buffer_read_i32(main);
    city_data.houses.requiring.library = buffer_read_i32(main);
    city_data.games.bet_amount = buffer_read_i32(main);
    city_data.houses.missing.barber = buffer_read_i32(main);
    city_data.houses.missing.bathhouse = buffer_read_i32(main);
    city_data.houses.missing.food = buffer_read_i32(main);
    if (!discard_unused_values) {
        buffer_skip(main, 8);
    }
    city_data.building.hippodrome_placed = buffer_read_i32(main);
    city_data.houses.missing.clinic = buffer_read_i32(main);
    city_data.houses.missing.hospital = buffer_read_i32(main);
    city_data.houses.requiring.barber = buffer_read_i32(main);
    city_data.houses.requiring.bathhouse = buffer_read_i32(main);
    city_data.houses.requiring.clinic = buffer_read_i32(main);
    city_data.houses.missing.religion = buffer_read_i32(main);
    city_data.houses.missing.second_religion = buffer_read_i32(main);
    city_data.houses.missing.third_religion = buffer_read_i32(main);
    city_data.houses.requiring.religion = buffer_read_i32(main);
    city_data.entertainment.theater_shows = buffer_read_i32(main);
    city_data.entertainment.theater_no_shows_weighted = buffer_read_i32(main);
    city_data.entertainment.amphitheater_shows = buffer_read_i32(main);
    city_data.entertainment.amphitheater_no_shows_weighted = buffer_read_i32(main);
    city_data.entertainment.colosseum_shows = buffer_read_i32(main);
    city_data.entertainment.colosseum_no_shows_weighted = buffer_read_i32(main);
    city_data.entertainment.hippodrome_shows = buffer_read_i32(main);
    city_data.entertainment.hippodrome_no_shows_weighted = buffer_read_i32(main);
    city_data.entertainment.venue_needing_shows = buffer_read_i32(main);
    city_data.culture.average_entertainment = buffer_read_i32(main);
    city_data.houses.missing.entertainment = buffer_read_i32(main);
    city_data.festival.months_since_festival = buffer_read_i32(main);
    for (int i = 0; i < MAX_GODS; i++) {
        city_data.religion.gods[i].target_happiness = buffer_read_i8(main);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        city_data.religion.gods[i].happiness = buffer_read_i8(main);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        city_data.religion.gods[i].wrath_bolts = buffer_read_i8(main);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        city_data.religion.gods[i].blessing_done = buffer_read_i8(main);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        city_data.religion.gods[i].small_curse_done = buffer_read_i8(main);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        city_data.religion.gods[i].happy_bolts = buffer_read_i8(main);
    }
    if (!discard_unused_values) {
        buffer_skip(main, 10);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        city_data.religion.gods[i].months_since_festival = buffer_read_i32(main);
    }
    city_data.religion.least_happy_god = buffer_read_i32(main);
    if (!discard_unused_values) {
        buffer_skip(main, 4);
    }
    city_data.migration.no_immigration_cause = buffer_read_i32(main);
    city_data.sentiment.protesters = buffer_read_i32(main);
    city_data.sentiment.criminals = buffer_read_i32(main);
    city_data.houses.health = buffer_read_i32(main);
    city_data.houses.religion = buffer_read_i32(main);
    city_data.houses.education = buffer_read_i32(main);
    city_data.houses.entertainment = buffer_read_i32(main);
    city_data.figure.rioters = buffer_read_i32(main);
    city_data.ratings.selected = buffer_read_i32(main);
    city_data.ratings.culture_explanation = buffer_read_i32(main);
    city_data.ratings.prosperity_explanation = buffer_read_i32(main);
    city_data.ratings.peace_explanation = buffer_read_i32(main);
    city_data.ratings.favor_explanation = buffer_read_i32(main);
    city_data.emperor.player_rank = buffer_read_i32(main);
    city_data.emperor.personal_savings = buffer_read_i32(main);
    if (!discard_unused_values) {
        buffer_skip(main, 8);
    }
    city_data.finance.last_year.income.donated = buffer_read_i32(main);
    city_data.finance.this_year.income.donated = buffer_read_i32(main);
    city_data.emperor.donate_amount = buffer_read_i32(main);
    for (int i = 0; i < 10; i++) {
        city_data.building.working_dock_ids[i] = buffer_read_i16(main);
    }
    city_data.building.months_since_last_destroyed_iron_mine = buffer_read_u16(main);
    city_data.building.months_since_last_flooded_clay_pit = buffer_read_u16(main);
    city_data.sentiment.blessing_festival_boost = buffer_read_i16(main);
    city_data.figure.animals = buffer_read_i16(main);
    city_data.trade.num_sea_routes = buffer_read_i16(main);
    city_data.trade.num_land_routes = buffer_read_i16(main);
    city_data.trade.sea_trade_problem_duration = buffer_read_i16(main);
    city_data.trade.land_trade_problem_duration = buffer_read_i16(main);
    city_data.building.working_docks = buffer_read_i16(main);
    city_data.building.senate_placed = buffer_read_i16(main);
    city_data.building.working_wharfs = buffer_read_i16(main);
    if (!discard_unused_values) {
        buffer_skip(main, 2);
    }
    city_data.finance.stolen_this_year = buffer_read_i16(main);
    city_data.finance.stolen_last_year = buffer_read_i16(main);
    city_data.trade.docker_import_resource = buffer_read_i32(main);
    city_data.trade.docker_export_resource = buffer_read_i32(main);
    city_data.emperor.debt_state = buffer_read_i32(main);
    city_data.emperor.months_in_debt = buffer_read_i32(main);
    city_data.finance.cheated_money = buffer_read_i32(main);
    city_data.building.barracks_x = buffer_read_i8(main);
    city_data.building.barracks_y = buffer_read_i8(main);
    city_data.building.barracks_grid_offset = buffer_read_i16(main);
    city_data.building.barracks_building_id = buffer_read_i32(main);
    city_data.building.barracks_placed = buffer_read_i32(main);
    city_data.building.mess_hall_building_id = buffer_read_i32(main);
    city_data.entertainment.arena_shows = buffer_read_i32(main); 
    city_data.entertainment.arena_no_shows_weighted = buffer_read_i32(main);
    if (!discard_unused_values) {
        buffer_skip(main, 8);
    }
    city_data.population.lost_troop_request = buffer_read_i32(main);
    if (!discard_unused_values) {
        buffer_skip(main, 4);
    }
    city_data.mission.has_won = buffer_read_i32(main);
    city_data.mission.continue_months_left = buffer_read_i32(main);
    city_data.mission.continue_months_chosen = buffer_read_i32(main);
    city_data.finance.wage_rate_paid_this_year = buffer_read_i32(main);
    city_data.finance.this_year.expenses.tribute = buffer_read_i32(main);
    city_data.finance.last_year.expenses.tribute = buffer_read_i32(main);
    city_data.finance.tribute_not_paid_last_year = buffer_read_i32(main);
    city_data.finance.tribute_not_paid_total_years = buffer_read_i32(main);
    city_data.festival.selected.god = buffer_read_i32(main);
    city_data.festival.selected.size = buffer_read_i32(main);
    city_data.festival.planned.size = buffer_read_i32(main);
    city_data.festival.planned.months_to_go = buffer_read_i32(main);
    city_data.festival.planned.god = buffer_read_i32(main);
    city_data.festival.small_cost = buffer_read_i32(main);
    city_data.festival.large_cost = buffer_read_i32(main);
    city_data.festival.grand_cost = buffer_read_i32(main);
    city_data.festival.grand_wine = buffer_read_i32(main);
    city_data.festival.not_enough_wine = buffer_read_i32(main);
    city_data.culture.average_religion = buffer_read_i32(main);
    city_data.culture.average_education = buffer_read_i32(main);
    city_data.culture.average_health = buffer_read_i32(main);
    city_data.culture.religion_coverage = buffer_read_i32(main);
    city_data.festival.first_festival_effect_months = buffer_read_i32(main);
    city_data.festival.second_festival_effect_months = buffer_read_i32(main);
    if (!discard_unused_values) {
        buffer_skip(main, 4);
    }
    city_data.sentiment.unemployment = buffer_read_i32(main);
    city_data.sentiment.previous_value = buffer_read_i32(main);
    city_data.sentiment.message_delay = buffer_read_i32(main);
    city_data.sentiment.low_mood_cause = buffer_read_i32(main);
    city_data.figure.security_breach_duration = buffer_read_i32(main);
    city_data.health.population_access.clinic = buffer_read_i32(main);
    city_data.health.population_access.baths = buffer_read_i32(main);
    city_data.health.population_access.barber = buffer_read_i32(main);
    city_data.health.months_since_last_contaminated_water = buffer_read_i32(main);
    city_data.emperor.selected_gift_size = buffer_read_i32(main);
    city_data.emperor.months_since_gift = buffer_read_i32(main);
    city_data.emperor.gift_overdose_penalty = buffer_read_i32(main);
    if (!discard_unused_values) {
        buffer_skip(main, 4);
    }
    city_data.emperor.gifts[GIFT_MODEST].id = buffer_read_i32(main);
    city_data.emperor.gifts[GIFT_GENEROUS].id = buffer_read_i32(main);
    city_data.emperor.gifts[GIFT_LAVISH].id = buffer_read_i32(main);
    city_data.emperor.gifts[GIFT_MODEST].cost = buffer_read_i32(main);
    city_data.emperor.gifts[GIFT_GENEROUS].cost = buffer_read_i32(main);
    city_data.emperor.gifts[GIFT_LAVISH].cost = buffer_read_i32(main);
    city_data.ratings.favor_salary_penalty = buffer_read_i32(main);
    city_data.ratings.favor_milestone_penalty = buffer_read_i32(main);
    city_data.ratings.favor_ignored_request_penalty = buffer_read_i32(main);
    city_data.ratings.favor_last_year = buffer_read_i32(main);
    city_data.ratings.favor_change = buffer_read_i32(main);
    city_data.military.native_attack_duration = buffer_read_i32(main);
    if (!discard_unused_values) {
        buffer_skip(main, 4);
    }
    city_data.building.mission_post_operational = buffer_read_i32(main);
    city_data.building.main_native_meeting.x = buffer_read_i32(main);
    city_data.building.main_native_meeting.y = buffer_read_i32(main);
    city_data.finance.wage_rate_paid_last_year = buffer_read_i32(main);
    city_data.resource.food_needed_per_month = buffer_read_i32(main);
    city_data.resource.granaries.understaffed = buffer_read_i32(main);
    city_data.resource.granaries.not_operating = buffer_read_i32(main);
    city_data.resource.granaries.not_operating_with_food = buffer_read_i32(main);
    if (!discard_unused_values) {
        buffer_skip(main, 4);
    }
    city_data.religion.venus_blessing_months_left = buffer_read_i32(main);
    city_data.religion.venus_curse_active = buffer_read_i32(main);
    city_data.building.num_striking_industries = buffer_read_i32(main);
    city_data.religion.neptune_double_trade_active = buffer_read_i32(main);
    city_data.religion.mars_spirit_power = buffer_read_i32(main);
    if (!discard_unused_values) {
        buffer_skip(main, 4);
    }
    city_data.religion.angry_message_delay = buffer_read_i32(main);
    city_data.resource.food_consumed_last_month = buffer_read_i32(main);
    city_data.resource.food_produced_last_month = buffer_read_i32(main);
    city_data.resource.food_produced_this_month = buffer_read_i32(main);
    city_data.ratings.peace_riot_cause = buffer_read_i32(main);
    city_data.finance.estimated_tax_income = buffer_read_i32(main);
    city_data.mission.tutorial_senate_built = buffer_read_i32(main);
    city_data.building.distribution_center_x = buffer_read_i8(main);
    city_data.building.distribution_center_y = buffer_read_i8(main);
    city_data.building.distribution_center_grid_offset = buffer_read_i16(main);
    city_data.building.distribution_center_building_id = buffer_read_i32(main);
    city_data.building.distribution_center_placed = buffer_read_i32(main);
    city_data.mess_hall.food_types = buffer_read_i32(main);
    city_data.mess_hall.food_stress_cumulative = buffer_read_i32(main);
    city_data.mess_hall.mess_hall_warning_shown = buffer_read_i32(main);
    city_data.mess_hall.food_percentage_missing_this_month = buffer_read_i32(main);
    city_data.mess_hall.total_food = buffer_read_i32(main);
    city_data.mess_hall.missing_mess_hall_warning_shown = buffer_read_i32(main);
    city_data.military.soldiers_in_city = buffer_read_i32(main);
    city_data.games.naval_battle_distant_battle_bonus = buffer_read_i32(main);
    city_data.figure.looters = buffer_read_i32(main);
    city_data.figure.robbers = buffer_read_i32(main);
    city_data.figure.protesters = buffer_read_i32(main);
    city_data.building.shipyard_boats_requested = buffer_read_i32(main);
    city_data.figure.enemies = buffer_read_i32(main);
    city_data.sentiment.wages = buffer_read_i32(main);
    city_data.population.people_in_tents = buffer_read_i32(main);
    city_data.population.people_in_large_insula_and_above = buffer_read_i32(main);
    city_data.figure.imperial_soldiers = buffer_read_i32(main);
    city_data.emperor.invasion.duration_day_countdown = buffer_read_i32(main);
    city_data.emperor.invasion.warnings_given = buffer_read_i32(main);
    city_data.emperor.invasion.days_until_invasion = buffer_read_i32(main);
    city_data.emperor.invasion.retreat_message_shown = buffer_read_i32(main);
    city_data.ratings.peace_destroyed_buildings = buffer_read_i32(main);
    city_data.ratings.peace_years_of_peace = buffer_read_i32(main);
    city_data.distant_battle.city = buffer_read_u8(main);
    city_data.distant_battle.enemy_strength = buffer_read_u8(main);
    city_data.distant_battle.roman_strength = buffer_read_u8(main);
    city_data.distant_battle.months_until_battle = buffer_read_i8(main);
    city_data.distant_battle.roman_months_to_travel_back = buffer_read_i8(main);
    city_data.distant_battle.roman_months_to_travel_forth = buffer_read_i8(main);
    city_data.distant_battle.city_foreign_months_left = buffer_read_i8(main);
    city_data.building.triumphal_arches_available = buffer_read_i8(main);
    city_data.distant_battle.total_count = buffer_read_i8(main);
    city_data.distant_battle.won_count = buffer_read_i8(main);
    city_data.distant_battle.enemy_months_traveled = buffer_read_i8(main);
    city_data.distant_battle.roman_months_traveled = buffer_read_i8(main);
    city_data.military.total_legions = buffer_read_u8(main);
    city_data.military.empire_service_legions = buffer_read_u8(main);
    city_data.games.chosen_horse = buffer_read_u8(main);
    city_data.military.total_soldiers = buffer_read_u8(main);
    city_data.building.triumphal_arches_placed = buffer_read_i8(main);
    city_data.sound.die_citizen = buffer_read_i8(main);
    city_data.sound.die_soldier = buffer_read_i8(main);
    city_data.sound.shoot_arrow = buffer_read_i8(main);
    city_data.building.trade_center_building_id = buffer_read_i32(main);
    city_data.figure.soldiers = buffer_read_i32(main);
    city_data.sound.hit_soldier = buffer_read_i8(main);
    city_data.sound.hit_spear = buffer_read_i8(main);
    city_data.sound.hit_club = buffer_read_i8(main);
    city_data.sound.march_enemy = buffer_read_i8(main);
    city_data.sound.march_horse = buffer_read_i8(main);
    city_data.sound.hit_elephant = buffer_read_i8(main);
    city_data.sound.hit_axe = buffer_read_i8(main);
    city_data.sound.hit_wolf = buffer_read_i8(main);
    city_data.sound.march_wolf = buffer_read_i8(main);
    if (!discard_unused_values) {
        buffer_skip(main, 6);
    }
    city_data.sentiment.include_tents = buffer_read_i8(main);
    city_data.emperor.invasion.count = buffer_read_i32(main);
    city_data.emperor.invasion.size = buffer_read_i32(main);
    city_data.emperor.invasion.soldiers_killed = buffer_read_i32(main);
    city_data.military.legionary_legions = buffer_read_i32(main);
    city_data.population.highest_ever = buffer_read_i32(main);
    city_data.finance.estimated_wages = buffer_read_i32(main);
    city_data.resource.wine_types_available = buffer_read_i32(main);
    city_data.ratings.prosperity_max = buffer_read_i32(main);
    for (int i = 0; i < 10; i++) {
        city_data.map.largest_road_networks[i].id = buffer_read_i32(main);
        city_data.map.largest_road_networks[i].size = buffer_read_i32(main);
    }
    city_data.houses.missing.second_wine = buffer_read_i32(main);
    city_data.religion.neptune_sank_ships = buffer_read_i32(main);
    city_data.entertainment.hippodrome_has_race = buffer_read_i32(main);
    city_data.entertainment.hippodrome_message_shown = buffer_read_i32(main);
    city_data.entertainment.colosseum_message_shown = buffer_read_i32(main);
    city_data.migration.emigration_message_shown = buffer_read_i32(main);
    city_data.mission.fired_message_shown = buffer_read_i32(main);
    city_data.mission.victory_message_shown = buffer_read_i32(main);
    city_data.mission.start_saved_game_written = buffer_read_i32(main);
    city_data.mission.tutorial_fire_message_shown = buffer_read_i32(main);
    city_data.mission.tutorial_disease_message_shown = buffer_read_i32(main);
    city_data.figure.attacking_natives = buffer_read_i32(main);
    if (!discard_unused_values) {
        buffer_skip(main, 232);
    }
    if (!has_separate_import_limits) {
        for (int i = RESOURCE_MIN; i < RESOURCE_MAX; i++) {
            if (city_data.resource.trade_status[i] == TRADE_STATUS_IMPORT) {
                city_data.resource.import_over[i] = city_data.resource.export_over[i];
                city_data.resource.export_over[i] = 0;
            } else {
                city_data.resource.import_over[i] = 0;
            }
        }
    }
}

static void save_entry_exit(buffer *entry_exit_xy, buffer *entry_exit_grid_offset)
{
    buffer_write_i32(entry_exit_xy, city_data.map.entry_flag.x);
    buffer_write_i32(entry_exit_xy, city_data.map.entry_flag.y);
    buffer_write_i32(entry_exit_xy, city_data.map.exit_flag.x);
    buffer_write_i32(entry_exit_xy, city_data.map.exit_flag.y);

    buffer_write_i32(entry_exit_grid_offset, city_data.map.entry_flag.grid_offset);
    buffer_write_i32(entry_exit_grid_offset, city_data.map.exit_flag.grid_offset);
}

static void load_entry_exit(buffer *entry_exit_xy, buffer *entry_exit_grid_offset)
{
    city_data.map.entry_flag.x = buffer_read_i32(entry_exit_xy);
    city_data.map.entry_flag.y = buffer_read_i32(entry_exit_xy);
    city_data.map.exit_flag.x = buffer_read_i32(entry_exit_xy);
    city_data.map.exit_flag.y = buffer_read_i32(entry_exit_xy);

    city_data.map.entry_flag.grid_offset = buffer_read_i32(entry_exit_grid_offset);
    city_data.map.exit_flag.grid_offset = buffer_read_i32(entry_exit_grid_offset);
}

void city_data_save_state(buffer *main, buffer *graph_order,
    buffer *entry_exit_xy, buffer *entry_exit_grid_offset)
{
    save_main_data(main);

    buffer_write_i32(graph_order, city_data.population.graph_order);

    save_entry_exit(entry_exit_xy, entry_exit_grid_offset);
}

void city_data_load_state(buffer *main, buffer *graph_order,
    buffer *entry_exit_xy, buffer *entry_exit_grid_offset, int has_separate_import_limits, int discard_unused_values)
{
    load_main_data(main, has_separate_import_limits, discard_unused_values);

    city_data.population.graph_order = buffer_read_i32(graph_order);

    load_entry_exit(entry_exit_xy, entry_exit_grid_offset);
}

void city_data_load_basic_info(buffer *main, int *population, int *treasury, int *caravanserai_id,
    int discard_unused_values)
{
    buffer_skip(main, discard_unused_values ? 4 : 18080);
    *treasury = buffer_read_i32(main);
    buffer_skip(main, discard_unused_values ? 16 : 20);
    *population = buffer_read_i32(main);
    buffer_skip(main, discard_unused_values ? 10363 : 10596);
    *caravanserai_id = buffer_read_i32(main);
}
