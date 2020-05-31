#include "finance.h"

#include "building/building.h"
#include "building/model.h"
#include "city/data_private.h"
#include "core/calc.h"
#include "game/difficulty.h"
#include "game/time.h"

#define MAX_HOUSE_LEVELS 20

int city_finance_treasury(void)
{
    return city_data.finance.treasury;
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
        city_data.finance.treasury += price;
        city_data.finance.this_year.income.exports += price;
    }
}

void city_finance_process_cheat(void)
{
    if (city_data.finance.treasury < 5000) {
        city_data.finance.treasury += 1000;
        city_data.finance.cheated_money += 1000;
    }
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
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->house_size && b->house_tax_coverage) {
            int is_patrician = b->subtype.house_level >= HOUSE_SMALL_VILLA;
            int trm = difficulty_adjust_money(
                model_get_house(b->subtype.house_level)->tax_multiplier);
            if (is_patrician) {
                city_data.taxes.monthly.collected_patricians += b->house_population * trm;
            } else {
                city_data.taxes.monthly.collected_plebs += b->house_population * trm;
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

    city_data.finance.this_year.income.taxes = city_data.taxes.yearly.collected_plebs + city_data.taxes.yearly.collected_patricians;
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
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE || !b->house_size) {
            continue;
        }

        int is_patrician = b->subtype.house_level >= HOUSE_SMALL_VILLA;
        int population = b->house_population;
        int trm = difficulty_adjust_money(
            model_get_house(b->subtype.house_level)->tax_multiplier);
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

void city_finance_handle_month_change(void)
{
    collect_monthly_taxes();
    pay_monthly_wages();
    pay_monthly_interest();
    pay_monthly_salary();
}

static void reset_taxes(void)
{
    city_data.finance.last_year.income.taxes = city_data.taxes.yearly.collected_plebs + city_data.taxes.yearly.collected_patricians;
    city_data.taxes.yearly.collected_plebs = 0;
    city_data.taxes.yearly.collected_patricians = 0;
    city_data.taxes.yearly.uncollected_plebs = 0;
    city_data.taxes.yearly.uncollected_patricians = 0;

    // reset tax income in building list
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->house_size) {
            b->tax_income_or_storage = 0;
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

const finance_overview *city_finance_overview_last_year(void)
{
    return &city_data.finance.last_year;
}

const finance_overview *city_finance_overview_this_year(void)
{
    return &city_data.finance.this_year;
}
