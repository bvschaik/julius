#ifndef CITY_FINANCE_H
#define CITY_FINANCE_H

#include "building/type.h"

#define SMALL_TEMPLE_LEVY_MONTHLY 4
#define FORT_LEVY_MONTHLY 8
#define TOWER_LEVY_MONTHLY 2
#define GRAND_TEMPLE_LEVY_MONTHLY 44
#define PANTHEON_LEVY_MONTHLY 48
#define LIGHTHOUSE_LEVY_MONTHLY 8
#define COLOSSEUM_LEVY_MONTHLY 36
#define HIPPODROME_LEVY_MONTHLY 72
#define CARAVANSERAI_LEVY_MONTHLY 8
#define LARGE_TEMPLE_LEVY_MONTHLY 8
#define SMALL_MAUSOLEUM_LEVY_MONTHLY 2
#define HIGHWAY_LEVY_MONTHLY 1

#define BUILDINGS_WITH_LEVIES 26
#define BUILDINGS_WITH_TOURISM 12

typedef struct {
    int type;
    int amount;
} building_levy_for_type;

typedef struct {
    int type;
    int income_modifier;
    int coverage;
    int count;
} tourism_for_type;

int city_finance_treasury(void);

void city_finance_treasury_add(int amount);

void city_finance_treasury_add_tourism(int amount);

int city_finance_out_of_money(void);

int city_finance_tax_percentage(void);

void city_finance_change_tax_percentage(int change);

int city_finance_percentage_taxed_people(void);

int city_finance_estimated_tax_income(void);

int city_finance_estimated_wages(void);

void city_finance_process_import(int price);

void city_finance_process_export(int price);

void city_finance_process_cheat(void);

void city_finance_process_console(int amount);

void city_finance_process_stolen(int stolen);

void city_finance_process_donation(int amount);

void city_finance_process_sundry(int cost);

void city_finance_process_construction(int cost);

void city_finance_update_interest(void);

void city_finance_update_salary(void);

void city_finance_calculate_totals(void);

void city_finance_estimate_wages(void);

void city_finance_estimate_taxes(void);

void city_finance_handle_month_change(void);

void city_finance_handle_year_change(void);

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
        int levies;
    } expenses;
    int net_in_out;
    int balance;
} finance_overview;

int city_finance_tourism_income_last_month(void);

const finance_overview *city_finance_overview_last_year(void);

const finance_overview *city_finance_overview_this_year(void);

int city_finance_spawn_tourist(void);
#endif // CITY_FINANCE_H
