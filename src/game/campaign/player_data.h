#ifndef PLAYER_DATA_H
#define PLAYER_DATA_H

/**
 * Indicates the latest played mission of a campaign.
 * 
 * @param campaign_file_name The file name of the campaign.
 * @return The last mission number of the campaign, or 0 if the campaign wasn't played.
 */
int campaign_player_data_get_current_mission(const char *campaign_file_name);

/**
 * Updates the campaign data with the latest mission played for a campaign.
 * The update will only occour if the value of the latest_mission is higher than the stored value.
 * This will also save the changes to the campaign_player_data.xml file.
 *
 * @param campaign_file_name The file name of the campaign.
 * @param latest_mission The latest mission to store.
 */
void campaign_player_data_update_current_mission(const char *campaign_file_name, int latest_mission);

#endif // PLAYER_DATA_H
