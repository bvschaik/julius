#ifndef GYMNASIUM_GYMNASIUM_H
#define GYMNASIUM_GYMNASIUM_H

#include "observation.h"
#include "action.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Environment handle (opaque pointer for Python)
 */
typedef struct julius_env julius_env_t;

/**
 * Environment configuration
 */
typedef struct {
    const char *data_directory;     // Path to Caesar III data files
    const char *scenario_file;      // Path to scenario file (optional, can be NULL for sandbox)
    int32_t max_ticks;             // Maximum ticks per episode (0 = unlimited)
    int32_t tick_speed_ms;         // Milliseconds per tick (1 = fastest)
} julius_env_config_t;

/**
 * Step result structure
 * Returned after each env_step() call
 */
typedef struct {
    gymnasium_observation_t observation;  // Current state
    float reward;                        // Reward for this step
    int32_t terminated;                  // Episode ended (goal reached or failed)
    int32_t truncated;                   // Episode cut off (max steps reached)
    char info[256];                      // Additional information
} julius_env_step_result_t;

/**
 * Create and initialize a Julius environment
 * @param config Configuration for the environment
 * @return Environment handle, or NULL on error
 */
julius_env_t *julius_env_create(const julius_env_config_t *config);

/**
 * Destroy an environment and free resources
 * @param env Environment handle
 */
void julius_env_destroy(julius_env_t *env);

/**
 * Reset the environment to initial state
 * @param env Environment handle
 * @param observation Output: initial observation (can be NULL)
 * @return 0 on success, -1 on error
 */
int julius_env_reset(julius_env_t *env, gymnasium_observation_t *observation);

/**
 * Execute one step in the environment
 * @param env Environment handle
 * @param action Action to execute
 * @param result Output: step result
 * @return 0 on success, -1 on error
 */
int julius_env_step(julius_env_t *env, const gymnasium_action_t *action, julius_env_step_result_t *result);

/**
 * Get current observation without advancing time
 * @param env Environment handle
 * @param observation Output: current observation
 * @return 0 on success, -1 on error
 */
int julius_env_get_observation(julius_env_t *env, gymnasium_observation_t *observation);

/**
 * Get last error message
 * @param env Environment handle
 * @return Error message string, or NULL if no error
 */
const char *julius_env_get_error(julius_env_t *env);

/**
 * Check if environment is initialized
 * @param env Environment handle
 * @return 1 if initialized, 0 otherwise
 */
int julius_env_is_initialized(julius_env_t *env);

/**
 * Get map data for rendering
 * @param env Environment handle
 * @param terrain_data Output: 162x162 array of terrain flags (must be pre-allocated)
 * @param building_data Output: 162x162 array of building IDs (must be pre-allocated)
 * @param width Output: actual map width
 * @param height Output: actual map height
 * @return 0 on success, -1 on error
 */
int julius_env_get_map_data(julius_env_t *env, uint16_t *terrain_data,
                             uint16_t *building_data, int *width, int *height);

#ifdef __cplusplus
}
#endif

#endif // GYMNASIUM_GYMNASIUM_H
