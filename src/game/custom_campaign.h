#ifndef GAME_CUSTOM_CAMPAIGN_H
#define GAME_CUSTOM_CAMPAIGN_H

#include <stdint.h>

typedef struct {
    const uint8_t *name;
    const uint8_t *description;
    int number_of_missions;
} custom_campaign_info;

const custom_campaign_info *custom_campaign_load_info(const char *filename);

void custom_campaign_clear(void);

#endif // GAME_CUSTOM_CAMPAIGN_H
