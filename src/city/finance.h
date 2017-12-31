#ifndef CITY_FINANCE_H
#define CITY_FINANCE_H

int city_finance_treasury();

int city_finance_out_of_money();

void city_finance_process_import(int price);

void city_finance_process_export(int price);

void city_finance_process_cheat();

void city_finance_process_stolen(int stolen);

void city_finance_process_donation(int amount);

void city_finance_process_sundry(int cost);

void city_finance_process_construction(int cost);

void city_finance_update_interest();

void city_finance_update_salary();

void city_finance_calculate_totals();

void city_finance_estimate_wages();

void city_finance_estimate_taxes();

void CityInfo_Finance_handleMonthChange();
void CityInfo_Finance_handleYearChange();

#endif // CITY_FINANCE_H
