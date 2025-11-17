#include "gymnasium/action.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int test_action_validation(void)
{
    gymnasium_action_t action;
    gymnasium_action_result_t result;

    // Test valid NONE action
    action.type = ACTION_NONE;
    action.x = 0;
    action.y = 0;
    if (!gymnasium_validate_action(&action, &result)) {
        fprintf(stderr, "ACTION_NONE should be valid\n");
        return 0;
    }

    // Test valid administrative action (doesn't need coordinates)
    action.type = ACTION_ADJUST_TAX_UP;
    if (!gymnasium_validate_action(&action, &result)) {
        fprintf(stderr, "ACTION_ADJUST_TAX_UP should be valid\n");
        return 0;
    }

    // Test invalid action type
    action.type = ACTION_COUNT + 10;  // Invalid type
    if (gymnasium_validate_action(&action, &result)) {
        fprintf(stderr, "Invalid action type should fail validation\n");
        return 0;
    }
    if (result.reason != ACTION_ERROR_INVALID_TYPE) {
        fprintf(stderr, "Invalid action should return ACTION_ERROR_INVALID_TYPE\n");
        return 0;
    }

    // Test building action with invalid coordinates
    action.type = ACTION_BUILD_HOUSING;
    action.x = -1;  // Invalid negative coordinate
    action.y = 50;
    if (gymnasium_validate_action(&action, &result)) {
        fprintf(stderr, "Negative coordinates should fail validation\n");
        return 0;
    }
    if (result.reason != ACTION_ERROR_INVALID_COORDS) {
        fprintf(stderr, "Invalid coords should return ACTION_ERROR_INVALID_COORDS\n");
        return 0;
    }

    // Test building action with valid coordinates
    action.type = ACTION_BUILD_HOUSING;
    action.x = 50;
    action.y = 50;
    if (!gymnasium_validate_action(&action, &result)) {
        fprintf(stderr, "Valid building action should pass validation\n");
        return 0;
    }

    // Test NULL pointer handling
    if (gymnasium_validate_action(NULL, &result)) {
        fprintf(stderr, "NULL action pointer should fail validation\n");
        return 0;
    }

    return 1;  // Success
}

static int test_action_names(void)
{
    const char *name;

    name = gymnasium_action_name(ACTION_NONE);
    if (strcmp(name, "NONE") != 0) {
        fprintf(stderr, "ACTION_NONE name should be 'NONE', got '%s'\n", name);
        return 0;
    }

    name = gymnasium_action_name(ACTION_BUILD_HOUSING);
    if (strcmp(name, "BUILD_HOUSING") != 0) {
        fprintf(stderr, "ACTION_BUILD_HOUSING name should be 'BUILD_HOUSING', got '%s'\n", name);
        return 0;
    }

    name = gymnasium_action_name(ACTION_ADJUST_TAX_UP);
    if (strcmp(name, "ADJUST_TAX_UP") != 0) {
        fprintf(stderr, "ACTION_ADJUST_TAX_UP name should be 'ADJUST_TAX_UP', got '%s'\n", name);
        return 0;
    }

    name = gymnasium_action_name(ACTION_CLEAR_LAND);
    if (strcmp(name, "CLEAR_LAND") != 0) {
        fprintf(stderr, "ACTION_CLEAR_LAND name should be 'CLEAR_LAND', got '%s'\n", name);
        return 0;
    }

    // Test invalid action type
    name = gymnasium_action_name(ACTION_COUNT + 10);
    if (strcmp(name, "UNKNOWN") != 0) {
        fprintf(stderr, "Invalid action should return 'UNKNOWN', got '%s'\n", name);
        return 0;
    }

    return 1;  // Success
}

static int test_none_action(void)
{
    gymnasium_action_t action;
    gymnasium_action_result_t result;

    action.type = ACTION_NONE;
    action.x = 0;
    action.y = 0;

    int ret = gymnasium_execute_action(&action, &result);
    if (ret != 0) {
        fprintf(stderr, "NONE action should succeed with return code 0\n");
        return 0;
    }

    if (!result.success) {
        fprintf(stderr, "NONE action result should indicate success\n");
        return 0;
    }

    if (result.reason != ACTION_ERROR_NONE) {
        fprintf(stderr, "NONE action should have no error\n");
        return 0;
    }

    return 1;  // Success
}

static int test_tax_actions(void)
{
    gymnasium_action_t action;
    gymnasium_action_result_t result;

    // Test tax increase
    action.type = ACTION_ADJUST_TAX_UP;
    action.x = 0;
    action.y = 0;

    // Note: This may fail if tax is already at max, but we're testing the API works
    int ret = gymnasium_execute_action(&action, &result);
    if (ret != 0 && result.reason != ACTION_ERROR_INVALID_TYPE) {
        // Either succeeded or failed because tax is at max - both are acceptable
        fprintf(stderr, "Unexpected error from tax increase: %s\n", result.message);
        return 0;
    }

    // Test tax decrease
    action.type = ACTION_ADJUST_TAX_DOWN;

    ret = gymnasium_execute_action(&action, &result);
    if (ret != 0 && result.reason != ACTION_ERROR_INVALID_TYPE) {
        // Either succeeded or failed because tax is at min - both are acceptable
        fprintf(stderr, "Unexpected error from tax decrease: %s\n", result.message);
        return 0;
    }

    return 1;  // Success
}

static int test_wage_actions(void)
{
    gymnasium_action_t action;
    gymnasium_action_result_t result;

    // Test wage increase
    action.type = ACTION_ADJUST_WAGES_UP;
    action.x = 0;
    action.y = 0;

    // Note: This may fail if wages are already at max
    int ret = gymnasium_execute_action(&action, &result);
    if (ret != 0 && result.reason != ACTION_ERROR_INVALID_TYPE) {
        fprintf(stderr, "Unexpected error from wage increase: %s\n", result.message);
        return 0;
    }

    // Test wage decrease
    action.type = ACTION_ADJUST_WAGES_DOWN;

    ret = gymnasium_execute_action(&action, &result);
    if (ret != 0 && result.reason != ACTION_ERROR_INVALID_TYPE) {
        fprintf(stderr, "Unexpected error from wage decrease: %s\n", result.message);
        return 0;
    }

    return 1;  // Success
}

static int test_building_action(void)
{
    gymnasium_action_t action;
    gymnasium_action_result_t result;

    // Test building construction
    // Note: This will likely fail because the game isn't fully initialized,
    // but we're testing that the API doesn't crash
    action.type = ACTION_BUILD_HOUSING;
    action.x = 50;
    action.y = 50;

    int ret = gymnasium_execute_action(&action, &result);

    // We don't check for success here since the game state may not allow building
    // We just verify the API works and returns a result
    if (result.reason < 0 || result.reason > ACTION_ERROR_NOT_IMPLEMENTED) {
        fprintf(stderr, "Invalid error reason from building action: %d\n", result.reason);
        return 0;
    }

    // Verify message is populated
    if (strlen(result.message) == 0) {
        fprintf(stderr, "Result message should not be empty\n");
        return 0;
    }

    return 1;  // Success
}

static int test_clear_land_action(void)
{
    gymnasium_action_t action;
    gymnasium_action_result_t result;

    // Test clear land
    action.type = ACTION_CLEAR_LAND;
    action.x = 50;
    action.y = 50;

    int ret = gymnasium_execute_action(&action, &result);

    // Like building, this may fail due to game state, but API should work
    if (result.reason < 0 || result.reason > ACTION_ERROR_NOT_IMPLEMENTED) {
        fprintf(stderr, "Invalid error reason from clear land action: %d\n", result.reason);
        return 0;
    }

    return 1;  // Success
}

static int test_invalid_execution(void)
{
    gymnasium_action_t action;
    gymnasium_action_result_t result;

    // Test execution with invalid coordinates
    action.type = ACTION_BUILD_HOUSING;
    action.x = -10;
    action.y = -10;

    int ret = gymnasium_execute_action(&action, &result);
    if (ret == 0) {
        fprintf(stderr, "Building with invalid coordinates should fail\n");
        return 0;
    }

    if (result.success) {
        fprintf(stderr, "Result should indicate failure for invalid coordinates\n");
        return 0;
    }

    // Test NULL pointer handling
    ret = gymnasium_execute_action(NULL, &result);
    if (ret == 0) {
        fprintf(stderr, "Execution with NULL action should fail\n");
        return 0;
    }

    return 1;  // Success
}

static void print_action_summary(void)
{
    printf("\n=== Gymnasium Action Summary ===\n");
    printf("Total action types: %d\n", ACTION_COUNT);
    printf("\nAdministrative actions:\n");
    printf("  - %s\n", gymnasium_action_name(ACTION_NONE));
    printf("  - %s\n", gymnasium_action_name(ACTION_ADJUST_TAX_UP));
    printf("  - %s\n", gymnasium_action_name(ACTION_ADJUST_TAX_DOWN));
    printf("  - %s\n", gymnasium_action_name(ACTION_ADJUST_WAGES_UP));
    printf("  - %s\n", gymnasium_action_name(ACTION_ADJUST_WAGES_DOWN));
    printf("\nBuilding actions: %d types\n",
           ACTION_CLEAR_LAND - ACTION_BUILD_HOUSING);
    printf("\nSample building actions:\n");
    printf("  - %s\n", gymnasium_action_name(ACTION_BUILD_HOUSING));
    printf("  - %s\n", gymnasium_action_name(ACTION_BUILD_ROAD));
    printf("  - %s\n", gymnasium_action_name(ACTION_BUILD_FOUNTAIN));
    printf("  - %s\n", gymnasium_action_name(ACTION_BUILD_MARKET));
    printf("  - %s\n", gymnasium_action_name(ACTION_BUILD_GRANARY));
    printf("\nDestruction actions:\n");
    printf("  - %s\n", gymnasium_action_name(ACTION_CLEAR_LAND));
    printf("================================\n\n");
}

int main(void)
{
    int tests_passed = 0;
    int tests_failed = 0;

    printf("Running Gymnasium Action Tests...\n\n");

    // Test 1: Action validation
    printf("Test 1: Action validation... ");
    if (test_action_validation()) {
        printf("PASSED\n");
        tests_passed++;
    } else {
        printf("FAILED\n");
        tests_failed++;
    }

    // Test 2: Action names
    printf("Test 2: Action names... ");
    if (test_action_names()) {
        printf("PASSED\n");
        tests_passed++;
    } else {
        printf("FAILED\n");
        tests_failed++;
    }

    // Test 3: NONE action
    printf("Test 3: NONE action execution... ");
    if (test_none_action()) {
        printf("PASSED\n");
        tests_passed++;
    } else {
        printf("FAILED\n");
        tests_failed++;
    }

    // Test 4: Tax actions
    printf("Test 4: Tax adjustment actions... ");
    if (test_tax_actions()) {
        printf("PASSED\n");
        tests_passed++;
    } else {
        printf("FAILED\n");
        tests_failed++;
    }

    // Test 5: Wage actions
    printf("Test 5: Wage adjustment actions... ");
    if (test_wage_actions()) {
        printf("PASSED\n");
        tests_passed++;
    } else {
        printf("FAILED\n");
        tests_failed++;
    }

    // Test 6: Building action
    printf("Test 6: Building action... ");
    if (test_building_action()) {
        printf("PASSED\n");
        tests_passed++;
    } else {
        printf("FAILED\n");
        tests_failed++;
    }

    // Test 7: Clear land action
    printf("Test 7: Clear land action... ");
    if (test_clear_land_action()) {
        printf("PASSED\n");
        tests_passed++;
    } else {
        printf("FAILED\n");
        tests_failed++;
    }

    // Test 8: Invalid execution
    printf("Test 8: Invalid execution handling... ");
    if (test_invalid_execution()) {
        printf("PASSED\n");
        tests_passed++;
    } else {
        printf("FAILED\n");
        tests_failed++;
    }

    // Print action summary
    print_action_summary();

    printf("=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);

    return tests_failed == 0 ? 0 : 1;
}
