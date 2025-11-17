#ifndef GYMNASIUM_OBSERVATION_H
#define GYMNASIUM_OBSERVATION_H

#include <stdint.h>

/**
 * Observation structure for Gymnasium integration
 * This structure contains all observable state from the game
 * that can be used for reinforcement learning
 */
typedef struct {
    // Ratings (0-100 each)
    struct {
        int32_t culture;
        int32_t prosperity;
        int32_t peace;
        int32_t favor;
    } ratings;

    // Finance
    struct {
        int32_t treasury;           // Current denarii in treasury
        int32_t tax_percentage;     // Tax rate (0-25%)
        int32_t estimated_tax_income; // Estimated tax income this year
        int32_t estimated_wages;    // Estimated wages this year
        int32_t last_year_income;   // Total income last year
        int32_t last_year_expenses; // Total expenses last year
        int32_t last_year_net;      // Net income/loss last year
    } finance;

    // Population
    struct {
        int32_t total;              // Total population
        int32_t school_age;         // School age population
        int32_t academy_age;        // Academy age population
        int32_t working_age;        // Working age population
        int32_t sentiment;          // Sentiment value (0-100)
    } population;

    // Labor
    struct {
        int32_t workers_available;  // Available workers
        int32_t workers_employed;   // Employed workers
        int32_t workers_needed;     // Workers needed
        int32_t unemployment_pct;   // Unemployment percentage
        int32_t wages;              // Current wage rate (denarii)
    } labor;

    // Resources
    struct {
        int32_t food_stocks;        // Total food in granaries
        int32_t food_types_available; // Number of different food types
        int32_t food_supply_months; // Months of food supply
        int32_t food_consumed_last_month; // Food consumed last month
        int32_t food_produced_last_month; // Food produced last month
    } resources;

    // Buildings (aggregated counts)
    struct {
        int32_t housing;            // Total housing buildings
        int32_t housing_capacity;   // Total housing capacity
        int32_t food_buildings;     // Farms, granaries, etc.
        int32_t industrial_buildings; // Workshops, industries
        int32_t entertainment;      // Entertainment venues
        int32_t education;          // Schools, libraries, academies
        int32_t health;             // Doctors, hospitals, baths
        int32_t religious;          // Temples, oracles
        int32_t total_buildings;    // Total count of all buildings
    } buildings;

    // Migration
    struct {
        int32_t immigration_amount; // Immigration per batch
        int32_t emigration_amount;  // Emigration per batch
        int32_t newcomers;          // Newcomers this period
    } migration;

    // Health & Culture
    struct {
        int32_t health_value;       // Health rating (0-100)
        int32_t average_entertainment; // Entertainment coverage
        int32_t average_education;  // Education coverage
        int32_t average_health;     // Health coverage
        int32_t average_religion;   // Religion coverage
    } culture;

    // Time
    struct {
        int32_t year;
        int32_t month;
        int32_t total_months;       // Total game time in months
    } time;

    // Victory/Game State
    struct {
        int32_t is_active;          // 1 if game is running, 0 if ended
        int32_t has_won;            // 1 if won, 0 otherwise
        int32_t population_goal;    // Population goal for scenario
        int32_t culture_goal;       // Culture goal for scenario
        int32_t prosperity_goal;    // Prosperity goal for scenario
        int32_t peace_goal;         // Peace goal for scenario
        int32_t favor_goal;         // Favor goal for scenario
    } victory;

} gymnasium_observation_t;

/**
 * Get current game state as an observation
 * @param obs Pointer to observation structure to fill
 * @return 0 on success, non-zero on error
 */
int gymnasium_get_observation(gymnasium_observation_t *obs);

/**
 * Reset/clear an observation structure
 * @param obs Pointer to observation structure to clear
 */
void gymnasium_clear_observation(gymnasium_observation_t *obs);

#endif // GYMNASIUM_OBSERVATION_H
