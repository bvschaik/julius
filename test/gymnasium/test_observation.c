#include "gymnasium/observation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int test_clear_observation(void)
{
    gymnasium_observation_t obs;

    // Fill with non-zero data
    memset(&obs, 0xFF, sizeof(obs));

    // Clear it
    gymnasium_clear_observation(&obs);

    // Verify all fields are zero
    if (obs.ratings.culture != 0) return 0;
    if (obs.ratings.prosperity != 0) return 0;
    if (obs.ratings.peace != 0) return 0;
    if (obs.ratings.favor != 0) return 0;

    if (obs.finance.treasury != 0) return 0;
    if (obs.population.total != 0) return 0;
    if (obs.labor.workers_available != 0) return 0;
    if (obs.resources.food_stocks != 0) return 0;
    if (obs.buildings.total_buildings != 0) return 0;
    if (obs.time.year != 0) return 0;
    if (obs.victory.has_won != 0) return 0;

    return 1; // Success
}

static int test_get_observation_structure(void)
{
    gymnasium_observation_t obs;

    // Try to get an observation
    int result = gymnasium_get_observation(&obs);

    // Should return 0 (success)
    if (result != 0) {
        fprintf(stderr, "gymnasium_get_observation returned %d, expected 0\n", result);
        return 0;
    }

    // Basic sanity checks - values should be reasonable
    // (even with uninitialized game state, they shouldn't be random garbage)

    // Ratings should be 0-100
    if (obs.ratings.culture < 0 || obs.ratings.culture > 100) {
        fprintf(stderr, "Invalid culture rating: %d\n", obs.ratings.culture);
        return 0;
    }
    if (obs.ratings.prosperity < 0 || obs.ratings.prosperity > 100) {
        fprintf(stderr, "Invalid prosperity rating: %d\n", obs.ratings.prosperity);
        return 0;
    }
    if (obs.ratings.peace < 0 || obs.ratings.peace > 100) {
        fprintf(stderr, "Invalid peace rating: %d\n", obs.ratings.peace);
        return 0;
    }
    if (obs.ratings.favor < 0 || obs.ratings.favor > 100) {
        fprintf(stderr, "Invalid favor rating: %d\n", obs.ratings.favor);
        return 0;
    }

    // Tax percentage should be 0-25
    if (obs.finance.tax_percentage < 0 || obs.finance.tax_percentage > 25) {
        fprintf(stderr, "Invalid tax percentage: %d\n", obs.finance.tax_percentage);
        return 0;
    }

    // Population should be non-negative
    if (obs.population.total < 0) {
        fprintf(stderr, "Invalid population: %d\n", obs.population.total);
        return 0;
    }

    // Unemployment percentage should be 0-100
    if (obs.labor.unemployment_pct < 0 || obs.labor.unemployment_pct > 100) {
        fprintf(stderr, "Invalid unemployment: %d\n", obs.labor.unemployment_pct);
        return 0;
    }

    // Victory flag should be 0 or 1
    if (obs.victory.has_won != 0 && obs.victory.has_won != 1) {
        fprintf(stderr, "Invalid has_won flag: %d\n", obs.victory.has_won);
        return 0;
    }

    return 1; // Success
}

static int test_observation_null_pointer(void)
{
    // Should handle NULL pointer gracefully
    int result = gymnasium_get_observation(NULL);

    if (result == 0) {
        fprintf(stderr, "gymnasium_get_observation(NULL) should return error\n");
        return 0;
    }

    return 1; // Success
}

static void print_observation(const gymnasium_observation_t *obs)
{
    printf("\n=== Gymnasium Observation ===\n");
    printf("Ratings:\n");
    printf("  Culture: %d, Prosperity: %d, Peace: %d, Favor: %d\n",
           obs->ratings.culture, obs->ratings.prosperity,
           obs->ratings.peace, obs->ratings.favor);

    printf("Finance:\n");
    printf("  Treasury: %d, Tax: %d%%, Income: %d, Expenses: %d\n",
           obs->finance.treasury, obs->finance.tax_percentage,
           obs->finance.last_year_income, obs->finance.last_year_expenses);

    printf("Population:\n");
    printf("  Total: %d, Working Age: %d, Sentiment: %d\n",
           obs->population.total, obs->population.working_age,
           obs->population.sentiment);

    printf("Labor:\n");
    printf("  Available: %d, Employed: %d, Needed: %d, Unemployment: %d%%\n",
           obs->labor.workers_available, obs->labor.workers_employed,
           obs->labor.workers_needed, obs->labor.unemployment_pct);

    printf("Resources:\n");
    printf("  Food Stocks: %d, Food Types: %d, Supply Months: %d\n",
           obs->resources.food_stocks, obs->resources.food_types_available,
           obs->resources.food_supply_months);

    printf("Buildings:\n");
    printf("  Housing: %d (capacity: %d), Food: %d, Industrial: %d\n",
           obs->buildings.housing, obs->buildings.housing_capacity,
           obs->buildings.food_buildings, obs->buildings.industrial_buildings);
    printf("  Entertainment: %d, Education: %d, Health: %d, Religious: %d\n",
           obs->buildings.entertainment, obs->buildings.education,
           obs->buildings.health, obs->buildings.religious);
    printf("  Total: %d\n", obs->buildings.total_buildings);

    printf("Time:\n");
    printf("  Year: %d, Month: %d (Total months: %d)\n",
           obs->time.year, obs->time.month, obs->time.total_months);

    printf("Victory:\n");
    printf("  Active: %d, Won: %d\n",
           obs->victory.is_active, obs->victory.has_won);
    printf("  Goals - Pop: %d, Culture: %d, Prosperity: %d, Peace: %d, Favor: %d\n",
           obs->victory.population_goal, obs->victory.culture_goal,
           obs->victory.prosperity_goal, obs->victory.peace_goal,
           obs->victory.favor_goal);
    printf("=============================\n\n");
}

int main(void)
{
    int tests_passed = 0;
    int tests_failed = 0;

    printf("Running Gymnasium Observation Tests...\n\n");

    // Test 1: Clear observation
    printf("Test 1: Clear observation... ");
    if (test_clear_observation()) {
        printf("PASSED\n");
        tests_passed++;
    } else {
        printf("FAILED\n");
        tests_failed++;
    }

    // Test 2: NULL pointer handling
    printf("Test 2: NULL pointer handling... ");
    if (test_observation_null_pointer()) {
        printf("PASSED\n");
        tests_passed++;
    } else {
        printf("FAILED\n");
        tests_failed++;
    }

    // Test 3: Get observation structure
    printf("Test 3: Get observation structure... ");
    if (test_get_observation_structure()) {
        printf("PASSED\n");
        tests_passed++;
    } else {
        printf("FAILED\n");
        tests_failed++;
    }

    // Print a sample observation for debugging
    gymnasium_observation_t obs;
    if (gymnasium_get_observation(&obs) == 0) {
        print_observation(&obs);
    }

    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);

    return tests_failed == 0 ? 0 : 1;
}
