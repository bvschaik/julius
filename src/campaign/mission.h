#ifndef CAMPAIGN_MISSION_H
#define CAMPAIGN_MISSION_H

#include "campaign/campaign.h"

typedef struct {
    int id;
    int first_option;
    int last_option;
    const char *background_image;
} campaign_mission;

campaign_mission *campaign_mission_new(void);
campaign_mission *campaign_mission_next(int last_index);
campaign_mission_option *campaign_mission_new_option(void);
campaign_mission_option *campaign_mission_get_option(int option_id);

int campaign_mission_init(void);
void campaign_mission_clear(void);

#endif // CAMPAIGN_MISSION_H
