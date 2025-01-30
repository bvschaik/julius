#ifndef CAMPAIGN_MISSION_H
#define CAMPAIGN_MISSION_H

#include "game/campaign.h"

typedef struct {
    unsigned int id;
    const uint8_t *title;
    int first_scenario;
    int last_scenario;
    int next_rank;
    int max_personal_savings;
    struct {
        const char *path;
        int id;
    } background_image;
    const char *intro_video;
} campaign_mission;

campaign_mission *campaign_mission_new(void);
campaign_mission *campaign_mission_current(int index);
campaign_mission *campaign_mission_next(int last_index);
campaign_scenario *campaign_mission_new_scenario(void);
campaign_scenario *campaign_mission_get_scenario(int scenario_id);

int campaign_mission_init(void);
void campaign_mission_clear(void);

#endif // CAMPAIGN_MISSION_H
