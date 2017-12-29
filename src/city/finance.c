#include "finance.h"

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
