#ifndef CAMPAIGN_ORIGINAL_H
#define CAMPAIGN_ORIGINAL_H

#include "core/buffer.h"

int campaign_original_setup(void);
uint8_t *campaign_original_load_scenario(int scenario_id, size_t *length);

#endif // CAMPAIGN_ORIGINAL_H
