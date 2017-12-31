#include "finance.h"

#include "building/building.h"
#include "building/model.h"
#include "core/calc.h"
#include "game/difficulty.h"
#include "game/time.h"

#include "Data/CityInfo.h"

#define MIN_TREASURY (-5000)

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
