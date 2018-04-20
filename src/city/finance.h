#ifndef CITY_FINANCE_H
#define CITY_FINANCE_H

int city_finance_treasury();

int city_finance_out_of_money();

int city_finance_tax_percentage();

void city_finance_change_tax_percentage(int change);

int city_finance_percentage_taxed_people();

int city_finance_estimated_tax_income();

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

void city_finance_handle_month_change();

void city_finance_handle_year_change();

typedef struct {
    struct {
        int taxes;
        int exports;
        int donated;
        int total;
    } income;
    struct {
        int imports;
        int wages;
        int construction;
        int interest;
        int salary;
        int sundries;
        int tribute;
        int total;
    } expenses;
    int net_in_out;
    int balance;
} finance_overview;

const finance_overview *city_finance_overview_last_year();

const finance_overview *city_finance_overview_this_year();

#endif // CITY_FINANCE_H
