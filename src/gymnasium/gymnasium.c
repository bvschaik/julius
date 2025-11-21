#include "gymnasium.h"

#include "city/finance.h"
#include "city/ratings.h"
#include "city/victory.h"
#include "game/file.h"
#include "game/game.h"
#include "game/time.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/terrain.h"
#include "platform/file_manager.h"
#include "scenario/criteria.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Internal environment state
 */
struct julius_env {
    int initialized;
    int game_initialized;
    char error_message[512];

    // Configuration
    julius_env_config_t config;

    // Episode state
    int current_tick;
    int episode_active;

    // Previous ratings for reward calculation
    int prev_culture;
    int prev_prosperity;
    int prev_peace;
    int prev_favor;
    int prev_treasury;
};

/**
 * Calculate reward based on rating changes and other factors
 */
static float calculate_reward(julius_env_t *env)
{
    float reward = 0.0f;

    // Get current ratings
    int culture = city_rating_culture();
    int prosperity = city_rating_prosperity();
    int peace = city_rating_peace();
    int favor = city_rating_favor();
    int treasury = city_finance_treasury();

    // Reward for improving ratings (scale by importance)
    reward += (culture - env->prev_culture) * 0.1f;
    reward += (prosperity - env->prev_prosperity) * 0.1f;
    reward += (peace - env->prev_peace) * 0.1f;
    reward += (favor - env->prev_favor) * 0.1f;

    // Small reward for maintaining positive treasury
    if (treasury > 0) {
        reward += 0.01f;
    }

    // Penalty for bankruptcy
    if (treasury < 0) {
        reward -= 1.0f;
    }

    // Update previous values
    env->prev_culture = culture;
    env->prev_prosperity = prosperity;
    env->prev_peace = peace;
    env->prev_favor = favor;
    env->prev_treasury = treasury;

    return reward;
}

/**
 * Check if episode should end
 */
static void check_termination(julius_env_t *env, julius_env_step_result_t *result)
{
    int victory = city_victory_state();

    if (victory == VICTORY_STATE_WON) {
        result->terminated = 1;
        result->reward += 100.0f;  // Large bonus for winning
        snprintf(result->info, sizeof(result->info), "Victory achieved!");
        return;
    }

    if (victory == VICTORY_STATE_LOST) {
        result->terminated = 1;
        result->reward -= 50.0f;  // Penalty for losing
        snprintf(result->info, sizeof(result->info), "Defeat - scenario lost");
        return;
    }

    // Check if max ticks reached
    if (env->config.max_ticks > 0 && env->current_tick >= env->config.max_ticks) {
        result->truncated = 1;
        snprintf(result->info, sizeof(result->info), "Max ticks reached");
        return;
    }

    // Check bankruptcy (severe treasury deficit)
    int treasury = city_finance_treasury();
    if (treasury < -5000) {
        result->terminated = 1;
        result->reward -= 50.0f;
        snprintf(result->info, sizeof(result->info), "Bankruptcy - treasury too low");
        return;
    }
}

julius_env_t *julius_env_create(const julius_env_config_t *config)
{
    if (!config) {
        return NULL;
    }

    julius_env_t *env = (julius_env_t *)calloc(1, sizeof(julius_env_t));
    if (!env) {
        return NULL;
    }

    // Copy configuration
    if (config->data_directory) {
        env->config.data_directory = strdup(config->data_directory);
    }
    if (config->scenario_file) {
        env->config.scenario_file = strdup(config->scenario_file);
    }
    env->config.max_ticks = config->max_ticks;
    env->config.tick_speed_ms = config->tick_speed_ms;

    // Set base path for game data
    if (config->data_directory) {
        if (!platform_file_manager_set_base_path(config->data_directory)) {
            snprintf(env->error_message, sizeof(env->error_message),
                    "Failed to set data directory: %s", config->data_directory);
            free(env);
            return NULL;
        }
    }

    // Pre-initialize game
    if (!game_pre_init()) {
        snprintf(env->error_message, sizeof(env->error_message),
                "Game pre-initialization failed");
        free(env);
        return NULL;
    }

    // Initialize game
    if (!game_init()) {
        snprintf(env->error_message, sizeof(env->error_message),
                "Game initialization failed");
        free(env);
        return NULL;
    }

    env->game_initialized = 1;
    env->initialized = 1;

    return env;
}

void julius_env_destroy(julius_env_t *env)
{
    if (!env) {
        return;
    }

    if (env->game_initialized) {
        game_exit();
    }

    if (env->config.data_directory) {
        free((void *)env->config.data_directory);
    }
    if (env->config.scenario_file) {
        free((void *)env->config.scenario_file);
    }

    free(env);
}

int julius_env_reset(julius_env_t *env, gymnasium_observation_t *observation)
{
    if (!env || !env->initialized) {
        return -1;
    }

    // Load scenario if specified
    if (env->config.scenario_file) {
        if (!game_file_start_scenario(env->config.scenario_file)) {
            snprintf(env->error_message, sizeof(env->error_message),
                    "Failed to load scenario: %s", env->config.scenario_file);
            return -1;
        }
    }

    // Reset episode state
    env->current_tick = 0;
    env->episode_active = 1;

    // Initialize previous ratings for reward calculation
    env->prev_culture = city_rating_culture();
    env->prev_prosperity = city_rating_prosperity();
    env->prev_peace = city_rating_peace();
    env->prev_favor = city_rating_favor();
    env->prev_treasury = city_finance_treasury();

    // Reset victory state
    city_victory_reset();

    // Get initial observation
    if (observation) {
        if (gymnasium_get_observation(observation) != 0) {
            snprintf(env->error_message, sizeof(env->error_message),
                    "Failed to get initial observation");
            return -1;
        }
    }

    return 0;
}

int julius_env_step(julius_env_t *env, const gymnasium_action_t *action,
                    julius_env_step_result_t *result)
{
    if (!env || !env->initialized || !action || !result) {
        if (env) {
            snprintf(env->error_message, sizeof(env->error_message),
                    "Invalid parameters to julius_env_step");
        }
        return -1;
    }

    if (!env->episode_active) {
        snprintf(env->error_message, sizeof(env->error_message),
                "Episode not active, call reset first");
        return -1;
    }

    // Initialize result
    memset(result, 0, sizeof(julius_env_step_result_t));

    // Execute the action
    gymnasium_action_result_t action_result;
    int action_status = gymnasium_execute_action(action, &action_result);

    // Small penalty for failed actions
    if (action_status != 0) {
        result->reward -= 0.1f;
    }

    // Run one game tick
    game_run();
    env->current_tick++;

    // Calculate reward based on state changes
    result->reward += calculate_reward(env);

    // Get current observation
    if (gymnasium_get_observation(&result->observation) != 0) {
        snprintf(env->error_message, sizeof(env->error_message),
                "Failed to get observation after step");
        return -1;
    }

    // Check for episode termination
    check_termination(env, result);

    if (result->terminated || result->truncated) {
        env->episode_active = 0;
    }

    // Add action result info if no termination info
    if (strlen(result->info) == 0) {
        snprintf(result->info, sizeof(result->info), "%s", action_result.message);
    }

    return 0;
}

int julius_env_get_observation(julius_env_t *env, gymnasium_observation_t *observation)
{
    if (!env || !env->initialized || !observation) {
        return -1;
    }

    return gymnasium_get_observation(observation);
}

const char *julius_env_get_error(julius_env_t *env)
{
    if (!env) {
        return NULL;
    }

    if (strlen(env->error_message) > 0) {
        return env->error_message;
    }

    return NULL;
}

int julius_env_is_initialized(julius_env_t *env)
{
    return env && env->initialized;
}

int julius_env_get_map_data(julius_env_t *env, uint16_t *terrain_data,
                             uint16_t *building_data, int *width, int *height)
{
    if (!env || !env->initialized || !terrain_data || !building_data || !width || !height) {
        return -1;
    }

    // Get actual map dimensions
    *width = map_grid_width();
    *height = map_grid_height();

    // Extract terrain and building data for each grid cell
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            int grid_offset = map_grid_offset(x, y);
            int index = y * GRID_SIZE + x;

            // Get terrain flags
            terrain_data[index] = (uint16_t) map_terrain_get(grid_offset);

            // Get building ID
            building_data[index] = (uint16_t) map_building_at(grid_offset);
        }
    }

    return 0;
}
