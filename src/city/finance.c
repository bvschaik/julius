#include "finance.h"

#include "building/building.h"
#include "building/model.h"
#include "core/calc.h"
#include "game/difficulty.h"
#include "game/time.h"

#include "Data/CityInfo.h"

#define MAX_HOUSE_LEVELS 20
#define MIN_TREASURY -5000

int city_finance_treasury()
{
    return Data_CityInfo.treasury;
}

int city_finance_out_of_money()
{
    return Data_CityInfo.treasury <= MIN_TREASURY;
}

void city_finance_process_import(int price)
{
    Data_CityInfo.treasury -= price;
    Data_CityInfo.financeImportsThisYear += price;
}

void city_finance_process_export(int price)
{
    Data_CityInfo.treasury += price;
    Data_CityInfo.financeExportsThisYear += price;
    if (Data_CityInfo.godBlessingNeptuneDoubleTrade) {
        Data_CityInfo.treasury += price;
        Data_CityInfo.financeExportsThisYear += price;
    }
}

void city_finance_process_cheat()
{
    if (Data_CityInfo.treasury < 5000) {
        Data_CityInfo.treasury += 1000;
        Data_CityInfo.cheatedMoney += 1000;
    }
}

void city_finance_process_stolen(int stolen)
{
    Data_CityInfo.financeStolenThisYear += stolen;
    city_finance_process_sundry(stolen);
}

void city_finance_process_donation(int amount)
{
    Data_CityInfo.treasury += amount;
    Data_CityInfo.financeDonatedThisYear += amount;
}

void city_finance_process_sundry(int cost)
{
    Data_CityInfo.treasury -= cost;
    Data_CityInfo.financeSundriesThisYear += cost;
}

void city_finance_process_construction(int cost)
{
    Data_CityInfo.treasury -= cost;
    Data_CityInfo.financeConstructionThisYear += cost;
}

void city_finance_update_interest()
{
    Data_CityInfo.financeInterestThisYear = Data_CityInfo.financeInterestPaidThisYear;
}

void city_finance_update_salary()
{
    Data_CityInfo.financeSalaryThisYear = Data_CityInfo.financeSalaryPaidThisYear;
}

void city_finance_calculate_totals()
{
    Data_CityInfo.financeTotalIncomeThisYear =
        Data_CityInfo.financeDonatedThisYear +
        Data_CityInfo.financeTaxesThisYear +
        Data_CityInfo.financeExportsThisYear;

    Data_CityInfo.financeTotalExpensesThisYear =
        Data_CityInfo.financeSundriesThisYear +
        Data_CityInfo.financeSalaryThisYear +
        Data_CityInfo.financeInterestThisYear +
        Data_CityInfo.financeConstructionThisYear +
        Data_CityInfo.financeWagesThisYear +
        Data_CityInfo.financeImportsThisYear;

    Data_CityInfo.financeNetInOutLastYear =
        Data_CityInfo.financeTotalIncomeLastYear -
        Data_CityInfo.financeTotalExpensesLastYear;
    Data_CityInfo.financeNetInOutThisYear =
        Data_CityInfo.financeTotalIncomeThisYear -
        Data_CityInfo.financeTotalExpensesThisYear;
    Data_CityInfo.financeBalanceThisYear =
        Data_CityInfo.financeNetInOutThisYear +
        Data_CityInfo.financeBalanceLastYear;

    Data_CityInfo.financeTributeThisYear = 0;
}

void city_finance_estimate_wages()
{
    int monthlyWages = Data_CityInfo.wages * Data_CityInfo.workersEmployed / 10 / 12;
    Data_CityInfo.financeWagesThisYear = Data_CityInfo.financeWagesPaidThisYear;
    Data_CityInfo.estimatedYearlyWages =
        (12 - game_time_month()) * monthlyWages +
        Data_CityInfo.financeWagesPaidThisYear;
}

void city_finance_estimate_taxes()
{
    Data_CityInfo.monthlyCollectedTaxFromPlebs = 0;
    Data_CityInfo.monthlyCollectedTaxFromPatricians = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (BuildingIsInUse(b) && b->houseSize && b->houseTaxCoverage) {
            int isPatrician = b->subtype.houseLevel >= HOUSE_SMALL_VILLA;
            int trm = difficulty_adjust_money(
                model_get_house(b->subtype.houseLevel)->tax_multiplier);
            if (isPatrician) {
                Data_CityInfo.monthlyCollectedTaxFromPatricians += b->housePopulation * trm;
            } else {
                Data_CityInfo.monthlyCollectedTaxFromPlebs += b->housePopulation * trm;
            }
        }
    }
    int monthly_patricians = calc_adjust_with_percentage(
        Data_CityInfo.monthlyCollectedTaxFromPatricians / 2,
        Data_CityInfo.taxPercentage);
    int monthly_plebs = calc_adjust_with_percentage(
        Data_CityInfo.monthlyCollectedTaxFromPlebs / 2,
        Data_CityInfo.taxPercentage);
    int estimated_rest_of_year = (12 - game_time_month()) * (monthly_patricians + monthly_plebs);

    Data_CityInfo.financeTaxesThisYear =
        Data_CityInfo.yearlyCollectedTaxFromPlebs + Data_CityInfo.yearlyCollectedTaxFromPatricians;
    Data_CityInfo.estimatedTaxIncome = Data_CityInfo.financeTaxesThisYear + estimated_rest_of_year;
}

static void collect_monthly_taxes()
{
    Data_CityInfo.monthlyTaxedPlebs = 0;
    Data_CityInfo.monthlyTaxedPatricians = 0;
    Data_CityInfo.monthlyUntaxedPlebs = 0;
    Data_CityInfo.monthlyUntaxedPatricians = 0;
    Data_CityInfo.monthlyUncollectedTaxFromPlebs = 0;
    Data_CityInfo.monthlyCollectedTaxFromPlebs = 0;
    Data_CityInfo.monthlyUncollectedTaxFromPatricians = 0;
    Data_CityInfo.monthlyCollectedTaxFromPatricians = 0;

    for (int i = 0; i < MAX_HOUSE_LEVELS; i++) {
        Data_CityInfo.populationPerLevel[i] = 0;
    }
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (!BuildingIsInUse(b) || !b->houseSize) {
            continue;
        }

        int is_patrician = b->subtype.houseLevel >= HOUSE_SMALL_VILLA;
        int population = b->housePopulation;
        int trm = difficulty_adjust_money(
            model_get_house(b->subtype.houseLevel)->tax_multiplier);
        Data_CityInfo.populationPerLevel[b->subtype.houseLevel] += population;

        int tax = population * trm;
        if (b->houseTaxCoverage) {
            if (is_patrician) {
                Data_CityInfo.monthlyTaxedPatricians += population;
                Data_CityInfo.monthlyCollectedTaxFromPatricians += tax;
            } else {
                Data_CityInfo.monthlyTaxedPlebs += population;
                Data_CityInfo.monthlyCollectedTaxFromPlebs += tax;
            }
            b->taxIncomeOrStorage += tax;
        } else {
            if (is_patrician) {
                Data_CityInfo.monthlyUntaxedPatricians += population;
                Data_CityInfo.monthlyUncollectedTaxFromPatricians += tax;
            } else {
                Data_CityInfo.monthlyUntaxedPlebs += population;
                Data_CityInfo.monthlyUncollectedTaxFromPlebs += tax;
            }
        }
    }

    int collected_patricians = calc_adjust_with_percentage(
        Data_CityInfo.monthlyCollectedTaxFromPatricians / 2,
        Data_CityInfo.taxPercentage);
    int collected_plebs = calc_adjust_with_percentage(
        Data_CityInfo.monthlyCollectedTaxFromPlebs / 2,
        Data_CityInfo.taxPercentage);
    int collected_total = collected_patricians + collected_plebs;

    Data_CityInfo.yearlyCollectedTaxFromPatricians += collected_patricians;
    Data_CityInfo.yearlyCollectedTaxFromPlebs += collected_plebs;
    Data_CityInfo.yearlyUncollectedTaxFromPatricians += calc_adjust_with_percentage(
        Data_CityInfo.monthlyUncollectedTaxFromPatricians / 2,
        Data_CityInfo.taxPercentage);
    Data_CityInfo.yearlyUncollectedTaxFromPlebs += calc_adjust_with_percentage(
        Data_CityInfo.monthlyUncollectedTaxFromPlebs / 2,
        Data_CityInfo.taxPercentage);

    Data_CityInfo.treasury += collected_total;

    int total_patricians = Data_CityInfo.monthlyTaxedPatricians + Data_CityInfo.monthlyUntaxedPatricians;
    int total_plebs = Data_CityInfo.monthlyTaxedPlebs + Data_CityInfo.monthlyUntaxedPlebs;
    Data_CityInfo.percentageTaxedPatricians = calc_percentage(Data_CityInfo.monthlyTaxedPatricians, total_patricians);
    Data_CityInfo.percentageTaxedPlebs = calc_percentage(Data_CityInfo.monthlyTaxedPlebs, total_plebs);
    Data_CityInfo.percentageTaxedPeople = calc_percentage(
        Data_CityInfo.monthlyTaxedPatricians + Data_CityInfo.monthlyTaxedPlebs,
        total_patricians + total_plebs);
}

static void pay_monthly_wages()
{
    int wages = Data_CityInfo.wages * Data_CityInfo.workersEmployed / 10 / 12;
    Data_CityInfo.treasury -= wages;
    Data_CityInfo.financeWagesPaidThisYear += wages;
    Data_CityInfo.wageRatePaidThisYear += Data_CityInfo.wages;
}

static void pay_monthly_interest()
{
    if (Data_CityInfo.treasury < 0) {
        int interest = calc_adjust_with_percentage(-Data_CityInfo.treasury, 10) / 12;
        Data_CityInfo.treasury -= interest;
        Data_CityInfo.financeInterestPaidThisYear += interest;
    }
}

static void pay_monthly_salary()
{
    if (!city_finance_out_of_money()) {
        Data_CityInfo.financeSalaryPaidThisYear += Data_CityInfo.salaryAmount;
        Data_CityInfo.personalSavings += Data_CityInfo.salaryAmount;
        Data_CityInfo.treasury -= Data_CityInfo.salaryAmount;
    }
}

void city_finance_handle_month_change()
{
    collect_monthly_taxes();
    pay_monthly_wages();
    pay_monthly_interest();
    pay_monthly_salary();
}

static void reset_taxes()
{
    Data_CityInfo.financeTaxesLastYear =
        Data_CityInfo.yearlyCollectedTaxFromPlebs + Data_CityInfo.yearlyCollectedTaxFromPatricians;
    Data_CityInfo.yearlyCollectedTaxFromPlebs = 0;
    Data_CityInfo.yearlyCollectedTaxFromPatricians = 0;
    Data_CityInfo.yearlyUncollectedTaxFromPlebs = 0;
    Data_CityInfo.yearlyUncollectedTaxFromPatricians = 0;
    
    // reset tax income in building list
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (BuildingIsInUse(b) && b->houseSize) {
            b->taxIncomeOrStorage = 0;
        }
    }
}

static void copy_amounts_to_last_year()
{
    // wages
    Data_CityInfo.financeWagesLastYear = Data_CityInfo.financeWagesPaidThisYear;
    Data_CityInfo.financeWagesPaidThisYear = 0;
    Data_CityInfo.wageRatePaidLastYear = Data_CityInfo.wageRatePaidThisYear;
    Data_CityInfo.wageRatePaidThisYear = 0;

    // import/export
    Data_CityInfo.financeExportsLastYear = Data_CityInfo.financeExportsThisYear;
    Data_CityInfo.financeExportsThisYear = 0;
    Data_CityInfo.financeImportsLastYear = Data_CityInfo.financeImportsThisYear;
    Data_CityInfo.financeImportsThisYear = 0;

    // construction
    Data_CityInfo.financeConstructionLastYear = Data_CityInfo.financeConstructionThisYear;
    Data_CityInfo.financeConstructionThisYear = 0;

    // interest
    Data_CityInfo.financeInterestLastYear = Data_CityInfo.financeInterestPaidThisYear;
    Data_CityInfo.financeInterestPaidThisYear = 0;

    // salary
    Data_CityInfo.financeSalaryLastYear = Data_CityInfo.financeSalaryPaidThisYear;
    Data_CityInfo.financeSalaryPaidThisYear = 0;

    // sundries
    Data_CityInfo.financeSundriesLastYear = Data_CityInfo.financeSundriesThisYear;
    Data_CityInfo.financeSundriesThisYear = 0;
    Data_CityInfo.financeStolenLastYear = Data_CityInfo.financeStolenThisYear;
    Data_CityInfo.financeStolenThisYear = 0;

    // donations
    Data_CityInfo.financeDonatedLastYear = Data_CityInfo.financeDonatedThisYear;
    Data_CityInfo.financeDonatedThisYear = 0;
}

static void pay_tribute()
{
    int income =
        Data_CityInfo.financeDonatedLastYear +
        Data_CityInfo.financeTaxesLastYear +
        Data_CityInfo.financeExportsLastYear;
    int expenses =
        Data_CityInfo.financeSundriesLastYear +
        Data_CityInfo.financeSalaryLastYear +
        Data_CityInfo.financeInterestLastYear +
        Data_CityInfo.financeConstructionLastYear +
        Data_CityInfo.financeWagesLastYear +
        Data_CityInfo.financeImportsLastYear;

    Data_CityInfo.tributeNotPaidLastYear = 0;
    if (Data_CityInfo.treasury <= 0) {
        // city is in debt
        Data_CityInfo.tributeNotPaidLastYear = 1;
        Data_CityInfo.tributeNotPaidTotalYears++;
        Data_CityInfo.financeTributeLastYear = 0;
    } else if (income <= expenses) {
        // city made a loss: fixed tribute based on population
        Data_CityInfo.tributeNotPaidTotalYears = 0;
        if (Data_CityInfo.population > 2000) {
            Data_CityInfo.financeTributeLastYear = 200;
        } else if (Data_CityInfo.population > 1000) {
            Data_CityInfo.financeTributeLastYear = 100;
        } else {
            Data_CityInfo.financeTributeLastYear = 0;
        }
    } else {
        // city made a profit: tribute is max of: 25% of profit, fixed tribute based on population
        Data_CityInfo.tributeNotPaidTotalYears = 0;
        if (Data_CityInfo.population > 5000) {
            Data_CityInfo.financeTributeLastYear = 500;
        } else if (Data_CityInfo.population > 3000) {
            Data_CityInfo.financeTributeLastYear = 400;
        } else if (Data_CityInfo.population > 2000) {
            Data_CityInfo.financeTributeLastYear = 300;
        } else if (Data_CityInfo.population > 1000) {
            Data_CityInfo.financeTributeLastYear = 225;
        } else if (Data_CityInfo.population > 500) {
            Data_CityInfo.financeTributeLastYear = 150;
        } else {
            Data_CityInfo.financeTributeLastYear = 50;
        }
        int pct_profit = calc_adjust_with_percentage(income - expenses, 25);
        if (pct_profit > Data_CityInfo.financeTributeLastYear) {
            Data_CityInfo.financeTributeLastYear = pct_profit;
        }
    }

    Data_CityInfo.treasury -= Data_CityInfo.financeTributeLastYear;
    Data_CityInfo.financeTributeThisYear = 0;

    Data_CityInfo.financeBalanceLastYear = Data_CityInfo.treasury;
    Data_CityInfo.financeTotalIncomeLastYear = income;
    Data_CityInfo.financeTotalExpensesLastYear = Data_CityInfo.financeTributeLastYear + expenses;
}

void city_finance_handle_year_change()
{
    reset_taxes();
    copy_amounts_to_last_year();
    pay_tribute();
}
