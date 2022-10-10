#include "finance.h"

#include "building/building.h"
#include "building/count.h"
#include "building/model.h"
#include "building/monument.h"
#include "city/data_private.h"
#include "city/culture.h"
#include "city/festival.h"
#include "core/calc.h"
#include "core/random.h"
#include "game/difficulty.h"
#include "game/time.h"
#include "figuretype/entertainer.h"
#include "map/data.h"
#include "map/terrain.h"

#define MAX_HOUSE_LEVELS 20


static building_levy_for_type building_levies[] = {
    {BUILDING_FORT, FORT_LEVY_MONTHLY},
    {BUILDING_SMALL_TEMPLE_CERES, SMALL_TEMPLE_LEVY_MONTHLY },
    {BUILDING_SMALL_TEMPLE_NEPTUNE, SMALL_TEMPLE_LEVY_MONTHLY },
    {BUILDING_SMALL_TEMPLE_MERCURY, SMALL_TEMPLE_LEVY_MONTHLY },
    {BUILDING_SMALL_TEMPLE_MARS, SMALL_TEMPLE_LEVY_MONTHLY },
    {BUILDING_SMALL_TEMPLE_VENUS, SMALL_TEMPLE_LEVY_MONTHLY },
    {BUILDING_LARGE_TEMPLE_CERES, LARGE_TEMPLE_LEVY_MONTHLY },
    {BUILDING_LARGE_TEMPLE_NEPTUNE, LARGE_TEMPLE_LEVY_MONTHLY },
    {BUILDING_LARGE_TEMPLE_MERCURY, LARGE_TEMPLE_LEVY_MONTHLY },
    {BUILDING_LARGE_TEMPLE_MARS, LARGE_TEMPLE_LEVY_MONTHLY }, // 10
    {BUILDING_LARGE_TEMPLE_VENUS, LARGE_TEMPLE_LEVY_MONTHLY },
    {BUILDING_ORACLE, SMALL_TEMPLE_LEVY_MONTHLY },
    {BUILDING_TOWER, TOWER_LEVY_MONTHLY },
    {BUILDING_LIGHTHOUSE, LIGHTHOUSE_LEVY_MONTHLY },
    {BUILDING_GRAND_TEMPLE_CERES, GRAND_TEMPLE_LEVY_MONTHLY},
    {BUILDING_GRAND_TEMPLE_NEPTUNE, GRAND_TEMPLE_LEVY_MONTHLY},
    {BUILDING_GRAND_TEMPLE_MERCURY, GRAND_TEMPLE_LEVY_MONTHLY},
    {BUILDING_GRAND_TEMPLE_MARS, GRAND_TEMPLE_LEVY_MONTHLY},
    {BUILDING_GRAND_TEMPLE_VENUS, GRAND_TEMPLE_LEVY_MONTHLY},
    {BUILDING_PANTHEON, PANTHEON_LEVY_MONTHLY}, // 20
    {BUILDING_COLOSSEUM, COLOSSEUM_LEVY_MONTHLY},
    {BUILDING_HIPPODROME, HIPPODROME_LEVY_MONTHLY},
    {BUILDING_SMALL_MAUSOLEUM, SMALL_MAUSOLEUM_LEVY_MONTHLY},
    {BUILDING_LARGE_MAUSOLEUM, SMALL_MAUSOLEUM_LEVY_MONTHLY},
    {BUILDING_NYMPHAEUM, SMALL_TEMPLE_LEVY_MONTHLY},
    {BUILDING_CARAVANSERAI, CARAVANSERAI_LEVY_MONTHLY },
};

static tourism_for_type tourism_modifiers[] = {
    {BUILDING_TAVERN, 2, TAVERN_COVERAGE, 0},
    {BUILDING_THEATER, 1, THEATER_COVERAGE, 0},
    {BUILDING_AMPHITHEATER, 1, AMPHITHEATER_COVERAGE, 0},
    {BUILDING_ARENA, 2, ARENA_COVERAGE, 0},
    {BUILDING_COLOSSEUM, 4, 0, 0},
    {BUILDING_HIPPODROME, 5, 0, 0},
    {BUILDING_GRAND_TEMPLE_CERES, 3, 0, 0},
    {BUILDING_GRAND_TEMPLE_NEPTUNE, 3, 0, 0},
    {BUILDING_GRAND_TEMPLE_MERCURY, 3, 0, 0},
    {BUILDING_GRAND_TEMPLE_MARS, 3, 0, 0},
    {BUILDING_GRAND_TEMPLE_VENUS, 3, 0, 0},
    {BUILDING_PANTHEON, 3, 0, 0}
};

int city_finance_treasury(void)
{
    return city_data.finance.treasury;
}

void city_finance_treasury_add(int amount)
{
    city_data.finance.treasury += amount;
}

void city_finance_treasury_add_tourism(int amount)
{
    city_finance_treasury_add(amount);
    city_data.finance.tourism_this_year += amount;
}


int city_finance_out_of_money(void)
{
    return city_data.finance.treasury <= -5000;
}

int city_finance_tax_percentage(void)
{
    return city_data.finance.tax_percentage;
}

void city_finance_change_tax_percentage(int change)
{
    city_data.finance.tax_percentage = calc_bound(city_data.finance.tax_percentage + change, 0, 25);
}

int city_finance_percentage_taxed_people(void)
{
    return city_data.taxes.percentage_taxed_people;
}

int city_finance_estimated_tax_income(void)
{
    return city_data.finance.estimated_tax_income;
}

int city_finance_estimated_wages(void)
{
    return city_data.finance.estimated_wages;
}

void city_finance_process_import(int price)
{
    city_data.finance.treasury -= price;
    city_data.finance.this_year.expenses.imports += price;
}

void city_finance_process_export(int price)
{
    city_data.finance.treasury += price;
    city_data.finance.this_year.income.exports += price;
    if (city_data.religion.neptune_double_trade_active) {
        city_data.finance.treasury += price / 2;
        city_data.finance.this_year.income.exports += price / 2;
    }
}

void city_finance_process_cheat(void)
{
    if (city_data.finance.treasury < 5000) {
        city_data.finance.treasury += 1000;
        city_data.finance.cheated_money += 1000;
    }
}

void city_finance_process_console(int amount)
{
    city_data.finance.treasury += amount;
    city_data.finance.cheated_money += amount;
}

void city_finance_process_stolen(int stolen)
{
    city_data.finance.stolen_this_year += stolen;
    city_finance_process_sundry(stolen);
}

void city_finance_process_donation(int amount)
{
    city_data.finance.treasury += amount;
    city_data.finance.this_year.income.donated += amount;
}

void city_finance_process_sundry(int cost)
{
    city_data.finance.treasury -= cost;
    city_data.finance.this_year.expenses.sundries += cost;
}

void city_finance_process_construction(int cost)
{
    city_data.finance.treasury -= cost;
    city_data.finance.this_year.expenses.construction += cost;
}

void city_finance_update_interest(void)
{
    city_data.finance.this_year.expenses.interest = city_data.finance.interest_so_far;
}

void city_finance_update_salary(void)
{
    city_data.finance.this_year.expenses.salary = city_data.finance.salary_so_far;
}

void city_finance_calculate_totals(void)
{
    finance_overview *this_year = &city_data.finance.this_year;
    this_year->income.total =
        this_year->income.donated +
        this_year->income.taxes +
        this_year->income.exports;

    this_year->expenses.total =
        this_year->expenses.sundries +
        this_year->expenses.salary +
        this_year->expenses.interest +
        this_year->expenses.construction +
        this_year->expenses.wages +
        this_year->expenses.levies +
        this_year->expenses.imports;

    finance_overview *last_year = &city_data.finance.last_year;
    last_year->net_in_out = last_year->income.total - last_year->expenses.total;
    this_year->net_in_out = this_year->income.total - this_year->expenses.total;
    this_year->balance = last_year->balance + this_year->net_in_out;

    this_year->expenses.tribute = 0;
}

void city_finance_estimate_wages(void)
{
    int monthly_wages = city_data.labor.wages * city_data.labor.workers_employed / 10 / 12;
    city_data.finance.this_year.expenses.wages = city_data.finance.wages_so_far;
    city_data.finance.estimated_wages = (12 - game_time_month()) * monthly_wages + city_data.finance.wages_so_far;
}

void city_finance_estimate_taxes(void)
{
    city_data.taxes.monthly.collected_plebs = 0;
    city_data.taxes.monthly.collected_patricians = 0;
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state == BUILDING_STATE_IN_USE && b->house_size && b->house_tax_coverage) {
                int is_patrician = b->subtype.house_level >= HOUSE_SMALL_VILLA;
                int trm = difficulty_adjust_money(model_get_house(b->subtype.house_level)->tax_multiplier);
                if (is_patrician) {
                    city_data.taxes.monthly.collected_patricians += b->house_population * trm;
                } else {
                    city_data.taxes.monthly.collected_plebs += b->house_population * trm;
                }
            }
        }
    }
    int monthly_patricians = calc_adjust_with_percentage(
        city_data.taxes.monthly.collected_patricians / 2,
        city_data.finance.tax_percentage);
    int monthly_plebs = calc_adjust_with_percentage(
        city_data.taxes.monthly.collected_plebs / 2,
        city_data.finance.tax_percentage);
    int estimated_rest_of_year = (12 - game_time_month()) * (monthly_patricians + monthly_plebs);

    city_data.finance.this_year.income.taxes =
        city_data.taxes.yearly.collected_plebs + city_data.taxes.yearly.collected_patricians;
    city_data.finance.estimated_tax_income = city_data.finance.this_year.income.taxes + estimated_rest_of_year;
}

static void collect_monthly_taxes(void)
{
    city_data.taxes.taxed_plebs = 0;
    city_data.taxes.taxed_patricians = 0;
    city_data.taxes.untaxed_plebs = 0;
    city_data.taxes.untaxed_patricians = 0;
    city_data.taxes.monthly.uncollected_plebs = 0;
    city_data.taxes.monthly.collected_plebs = 0;
    city_data.taxes.monthly.uncollected_patricians = 0;
    city_data.taxes.monthly.collected_patricians = 0;

    for (int i = 0; i < MAX_HOUSE_LEVELS; i++) {
        city_data.population.at_level[i] = 0;
    }
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state != BUILDING_STATE_IN_USE || !b->house_size) {
                continue;
            }

            int is_patrician = b->subtype.house_level >= HOUSE_SMALL_VILLA;
            int population = b->house_population;
            int trm = difficulty_adjust_money(model_get_house(b->subtype.house_level)->tax_multiplier);
            city_data.population.at_level[b->subtype.house_level] += population;

            int tax = population * trm;
            if (b->house_tax_coverage) {
                if (is_patrician) {
                    city_data.taxes.taxed_patricians += population;
                    city_data.taxes.monthly.collected_patricians += tax;
                } else {
                    city_data.taxes.taxed_plebs += population;
                    city_data.taxes.monthly.collected_plebs += tax;
                }
                b->tax_income_or_storage += tax;
            } else {
                if (is_patrician) {
                    city_data.taxes.untaxed_patricians += population;
                    city_data.taxes.monthly.uncollected_patricians += tax;
                } else {
                    city_data.taxes.untaxed_plebs += population;
                    city_data.taxes.monthly.uncollected_plebs += tax;
                }
            }
        }
    }

    int collected_patricians = calc_adjust_with_percentage(
        city_data.taxes.monthly.collected_patricians / 2,
        city_data.finance.tax_percentage);
    int collected_plebs = calc_adjust_with_percentage(
        city_data.taxes.monthly.collected_plebs / 2,
        city_data.finance.tax_percentage);
    int collected_total = collected_patricians + collected_plebs;

    city_data.taxes.yearly.collected_patricians += collected_patricians;
    city_data.taxes.yearly.collected_plebs += collected_plebs;
    city_data.taxes.yearly.uncollected_patricians += calc_adjust_with_percentage(
        city_data.taxes.monthly.uncollected_patricians / 2,
        city_data.finance.tax_percentage);
    city_data.taxes.yearly.uncollected_plebs += calc_adjust_with_percentage(
        city_data.taxes.monthly.uncollected_plebs / 2,
        city_data.finance.tax_percentage);

    city_data.finance.treasury += collected_total;

    int total_patricians = city_data.taxes.taxed_patricians + city_data.taxes.untaxed_patricians;
    int total_plebs = city_data.taxes.taxed_plebs + city_data.taxes.untaxed_plebs;
    city_data.taxes.percentage_taxed_patricians = calc_percentage(city_data.taxes.taxed_patricians, total_patricians);
    city_data.taxes.percentage_taxed_plebs = calc_percentage(city_data.taxes.taxed_plebs, total_plebs);
    city_data.taxes.percentage_taxed_people = calc_percentage(
        city_data.taxes.taxed_patricians + city_data.taxes.taxed_plebs,
        total_patricians + total_plebs);
}

static void pay_monthly_wages(void)
{
    int wages = city_data.labor.wages * city_data.labor.workers_employed / 10 / 12;
    city_data.finance.treasury -= wages;
    city_data.finance.wages_so_far += wages;
    city_data.finance.wage_rate_paid_this_year += city_data.labor.wages;
}

static void pay_monthly_interest(void)
{
    if (city_data.finance.treasury < 0) {
        int interest = calc_adjust_with_percentage(-city_data.finance.treasury, 10) / 12;
        city_data.finance.treasury -= interest;
        city_data.finance.interest_so_far += interest;
    }
}

static void pay_monthly_salary(void)
{
    if (!city_finance_out_of_money()) {
        city_data.finance.salary_so_far += city_data.emperor.salary_amount;
        city_data.emperor.personal_savings += city_data.emperor.salary_amount;
        city_data.finance.treasury -= city_data.emperor.salary_amount;
    }
}

static void pay_monthly_building_levies(void)
{
    int levies = 0;
    for (int i = 0; i < BUILDINGS_WITH_LEVIES; i++) {
        building_type type = building_levies[i].type;
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            b->monthly_levy = building_levies[i].amount;
            int levy = building_get_levy(b);
            levies += levy;
        }
    }

    int grid_offset = map_data.start_offset;
    for (int y = 0; y < map_data.height; y++, grid_offset += map_data.border_size) {
        for (int x = 0; x < map_data.width; x++, grid_offset++) {
            if (map_terrain_is(grid_offset, TERRAIN_HIGHWAY_TOP_LEFT)) {
                levies += HIGHWAY_LEVY_MONTHLY;
            }
        }
    }

    city_data.finance.treasury -= levies;
    city_data.finance.this_year.expenses.levies += levies;
}

static void activate_monthly_tourism(void)
{
    for (int i = 0; i < BUILDINGS_WITH_TOURISM; i++) {
        building_type type = tourism_modifiers[i].type;
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state != BUILDING_STATE_IN_USE || !b->num_workers) {
                continue;
            }
            b->is_tourism_venue = 1;
            if (game_time_month() == 0) {
                b->tourism_income_this_year = 0;
            }
            tourism_modifiers[i].count++;
            // disable redundant venues for tourism
            if ((tourism_modifiers[i].count * tourism_modifiers[i].coverage) > city_data.population.population) {
                b->tourism_disabled = 1;
                b->tourism_income = 0;
            } else {
                b->tourism_disabled = 0;
                b->tourism_income = tourism_modifiers[i].income_modifier;
            }
        }
    }
}

void city_finance_handle_month_change(void)
{
    collect_monthly_taxes();
    activate_monthly_tourism();
    pay_monthly_wages();
    pay_monthly_interest();
    pay_monthly_salary();
    pay_monthly_building_levies();
}

static void reset_taxes(void)
{
    city_data.finance.last_year.income.taxes =
        city_data.taxes.yearly.collected_plebs + city_data.taxes.yearly.collected_patricians;
    city_data.taxes.yearly.collected_plebs = 0;
    city_data.taxes.yearly.collected_patricians = 0;
    city_data.taxes.yearly.uncollected_plebs = 0;
    city_data.taxes.yearly.uncollected_patricians = 0;

    // reset tax income in building list
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state == BUILDING_STATE_IN_USE && b->house_size) {
                b->tax_income_or_storage = 0;
            }
        }
    }
}

static void copy_amounts_to_last_year(void)
{
    finance_overview *last_year = &city_data.finance.last_year;
    finance_overview *this_year = &city_data.finance.this_year;

    // wages
    last_year->expenses.wages = city_data.finance.wages_so_far;
    city_data.finance.wages_so_far = 0;
    city_data.finance.wage_rate_paid_last_year = city_data.finance.wage_rate_paid_this_year;
    city_data.finance.wage_rate_paid_this_year = 0;

    //levies
    last_year->expenses.levies = this_year->expenses.levies;
    this_year->expenses.levies = 0;

    // import/export
    last_year->income.exports = this_year->income.exports;
    this_year->income.exports = 0;
    last_year->expenses.imports = this_year->expenses.imports;
    this_year->expenses.imports = 0;

    // construction
    last_year->expenses.construction = this_year->expenses.construction;
    this_year->expenses.construction = 0;

    // interest
    last_year->expenses.interest = city_data.finance.interest_so_far;
    city_data.finance.interest_so_far = 0;

    // salary
    city_data.finance.last_year.expenses.salary = city_data.finance.salary_so_far;
    city_data.finance.salary_so_far = 0;

    // sundries
    last_year->expenses.sundries = this_year->expenses.sundries;
    this_year->expenses.sundries = 0;
    city_data.finance.stolen_last_year = city_data.finance.stolen_this_year;
    city_data.finance.stolen_this_year = 0;

    // donations
    last_year->income.donated = this_year->income.donated;
    this_year->income.donated = 0;

    //tourism 
    city_data.finance.tourism_last_year = city_data.finance.tourism_this_year;
    city_data.finance.tourism_this_year = 0;
}

static void pay_tribute(void)
{
    finance_overview *last_year = &city_data.finance.last_year;
    int income =
        last_year->income.donated +
        last_year->income.taxes +
        last_year->income.exports;
    int expenses =
        last_year->expenses.sundries +
        last_year->expenses.salary +
        last_year->expenses.interest +
        last_year->expenses.construction +
        last_year->expenses.wages +
        last_year->expenses.levies +
        last_year->expenses.imports;

    city_data.finance.tribute_not_paid_last_year = 0;
    if (city_data.finance.treasury <= 0) {
        // city is in debt
        city_data.finance.tribute_not_paid_last_year = 1;
        city_data.finance.tribute_not_paid_total_years++;
        last_year->expenses.tribute = 0;
    } else if (income <= expenses) {
        // city made a loss: fixed tribute based on population
        city_data.finance.tribute_not_paid_total_years = 0;
        if (city_data.population.population > 2000) {
            last_year->expenses.tribute = 200;
        } else if (city_data.population.population > 1000) {
            last_year->expenses.tribute = 100;
        } else {
            last_year->expenses.tribute = 0;
        }
    } else {
        // city made a profit: tribute is max of: 25% of profit, fixed tribute based on population
        city_data.finance.tribute_not_paid_total_years = 0;
        if (city_data.population.population > 5000) {
            last_year->expenses.tribute = 500;
        } else if (city_data.population.population > 3000) {
            last_year->expenses.tribute = 400;
        } else if (city_data.population.population > 2000) {
            last_year->expenses.tribute = 300;
        } else if (city_data.population.population > 1000) {
            last_year->expenses.tribute = 225;
        } else if (city_data.population.population > 500) {
            last_year->expenses.tribute = 150;
        } else {
            last_year->expenses.tribute = 50;
        }
        int pct_profit = calc_adjust_with_percentage(income - expenses, 25);
        if (pct_profit > last_year->expenses.tribute) {
            last_year->expenses.tribute = pct_profit;
        }
    }

    city_data.finance.treasury -= last_year->expenses.tribute;
    city_data.finance.this_year.expenses.tribute = 0;

    last_year->balance = city_data.finance.treasury;
    last_year->income.total = income;
    last_year->expenses.total = last_year->expenses.tribute + expenses;
}

void city_finance_handle_year_change(void)
{
    reset_taxes();
    copy_amounts_to_last_year();
    pay_tribute();
}

int city_finance_tourism_income_last_month(void)
{
    return city_data.finance.tourism_last_month;
}

int city_finance_tourism_lowest_factor(void)
{
    return city_data.finance.tourism_lowest_factor;
}


const finance_overview *city_finance_overview_last_year(void)
{
    return &city_data.finance.last_year;
}

const finance_overview *city_finance_overview_this_year(void)
{
    return &city_data.finance.this_year;
}

int city_finance_spawn_tourist(void)
{
    if (!city_festival_games_active()) {
        return 0;
    }
    int tick_increase = random_byte() % city_data.ratings.culture;
    city_data.finance.tourist_spawn_delay += tick_increase;
    if (city_data.finance.tourist_spawn_delay > 500) {
        figure_spawn_tourist();
        city_data.finance.tourist_spawn_delay = 0;
    }

    return 1;
}
