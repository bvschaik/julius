#ifndef CAMPAIGN_H
#define CAMPAIGN_H

#include <stddef.h>
#include <stdint.h>

#define CAMPAIGN_FIRST_MISSION -1

typedef struct {
    const uint8_t *name;
    const uint8_t *description;
    int number_of_missions;
} campaign_info;

typedef struct {
    int id;
    int x;
    int y;
    const char *name;
    const char *description;
    const char *path;
    struct {
        int x;
        int y;
        const char *path;
    } image;
} campaign_mission_option;

typedef struct {
    const char *background_image;
    const campaign_mission_option *(*get_next_option)(void);
} campaign_mission_info;

/**
 * Loads a campaign from the filename.
 * @param filename The filename to load. Can be a directory or a file with the .campaign extension.
 * @return 1 if the campaign was loaded, 0 if there was a problem.
 */
int campaign_load(const char *filename);

/**
 * Checks if the campaign is active.
 * @return 1 if the campaign is active, 0 otherwise.
 */
int campaign_is_active(void);

/**
 * Gets the campaign filename.
 * @return The campaign filename if the campaign is active, 0 otherwise.
 */
const char *campaign_get_name(void);

/**
 * Gets the basic info for the campaign.
 * @return A campaign_info struct with the data if successful, 0 otherwise.
 */
const campaign_info *campaign_get_info(void);

/**
 * Checks if a file exists in the current campaign.
 * @param filename The filename to check.
 * @return 1 if the file exists, 0 otherwise.
 */
int campaign_has_file(const char *filename);

/**
 * Loads a file from the campaign.
 * @param filename The filename to load.
 * @param length The length of the file.
 * @return A pointer to the file data if successful, 0 otherwise. The caller is responsible for freeing the memory.
 */
uint8_t *campaign_load_file(const char *filename, size_t *length);

/**
 * Gets the next mission from the campaign.
 * @param last_scenario_id The last played scenario.
 * @return A pointer to the first mission whose first scenario is higher than last_scenario_id,
 * or 0 if there's an error or if there are no new missions.
 */
const campaign_mission_info *campaign_get_next_mission(int last_scenario_id);

/**
 * Suspends the campaign.
 */
void campaign_suspend(void);

/**
 * Restores a suspended campaign.
 */
void campaign_restore(void);

/**
 * Clears the campaign data.
 */
void campaign_clear(void);

#endif // CAMPAIGN_H
