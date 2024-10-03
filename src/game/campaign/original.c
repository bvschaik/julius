#include "original.h"

#include "core/io.h"
#include "core/image.h"
#include "core/image_group.h"
#include "core/lang.h"
#include "core/log.h"
#include "core/string.h"
#include "game/campaign.h"
#include "game/campaign/mission.h"
#include "translation/translation.h"

#include <limits.h>
#include <stdint.h>
#include <string.h>

#define MISSION_PACK_FILE "mission1.pak"
#define NUM_ORIGINAL_SCENARIOS 20
#define FIRST_RANK_WITH_MULTIPLE_SCENARIOS 2
#define SCENARIOS_PER_RANK 2
#define MAX_RANK 11

static struct {
    int x;
    int y;
} ORIGINAL_SCENARIO_MAP_POSITIONS[NUM_ORIGINAL_SCENARIOS] = {
    { 0, 0 }, { 0, 0 }, { 288, 178 }, { 349, 228 }, { 114, 198 }, { 320, 282 }, { 545, 281 }, { 220, 117 },
    { 169, 105 }, { 236, 288 }, { 572, 279 }, { 15, 312 }, { 93, 236 }, { 152, 55 }, { 123, 296 }, { 575, 323 },
    { 99, 31 }, { 406, 105 }, { 187, 149 }, { 82, 4 }
};

static uint8_t *copy_new_string(const uint8_t *str)
{
    int length = string_length(str) + 1;
    uint8_t *dst = malloc(sizeof(uint8_t) * length);
    if (!dst) {
        return 0;
    }
    string_copy(str, dst, length);
    return dst;
}

static uint8_t *find_in_string(uint8_t *text, uint8_t value)
{
    while (*text) {
        if (*text == value) {
            return text;
        }
        text++;
    }
    return 0;
}

static campaign_scenario *new_scenario(int rank)
{
    campaign_scenario *scenario = campaign_mission_new_scenario();
    if (!scenario) {
        log_error("Problem generating original campaign data - memory full", 0, 0);
        return 0;
    }

    uint8_t *name;
    if (rank < FIRST_RANK_WITH_MULTIPLE_SCENARIOS) {
        name = copy_new_string(lang_get_string(CUSTOM_TRANSLATION, TR_ORIGINAL_CAMPAIGN_FIRST_MISSION + rank));
    } else {
        name = copy_new_string(lang_get_string(144, 2 + 3 * rank + (scenario->id % SCENARIOS_PER_RANK)));
    }
    if (!name) {
        log_error("Problem generating original campaign data - memory full", 0, 0);
        return 0;
    }
    uint8_t *description = find_in_string(name, ':');
    if (description) {
        *description = 0;
        // Get rid of the ":" and the proceeding whitespace
        description++;
        while (*description == ' ') {
            description++;
        }
        // Make first letter uppercase
        if (description[0] >= 'a' && description[0] <= 'z') {
            description[0] -= 32;
        }
        scenario->description = description;
    }
    scenario->name = name;
    scenario->fanfare = scenario->id % SCENARIOS_PER_RANK == 0 ? "wavs/fanfare_nu1.wav" : "wavs/fanfare_nu5.wav";
    if (rank >= FIRST_RANK_WITH_MULTIPLE_SCENARIOS) {
        scenario->x = ORIGINAL_SCENARIO_MAP_POSITIONS[scenario->id].x;
        scenario->y = ORIGINAL_SCENARIO_MAP_POSITIONS[scenario->id].y;
    }
    scenario->path = MISSION_PACK_FILE;

    return scenario;
}

int campaign_original_setup(void)
{
    if (!campaign_mission_init()) {
        log_error("Problem parsing campaign file - unable to allocate memory for campaigns", 0, 0);
        return 0;
    }

    for (int rank = 0; rank < MAX_RANK; rank++) {
        campaign_mission *mission = campaign_mission_new();
        if (!mission) {
            log_error("Problem generating original campaign data - memory full", 0, 0);
            return 0;
        }
        mission->title = lang_get_string(144, 1 + 3 * rank);
        mission->next_rank = rank + 1;
        mission->max_personal_savings = INT_MAX;

        campaign_scenario *scenario = new_scenario(rank);
        if (!scenario) {
            return 0;
        }
        mission->first_scenario = scenario->id;

        if (rank >= FIRST_RANK_WITH_MULTIPLE_SCENARIOS) {
            scenario = new_scenario(rank);
            if (!scenario) {
                return 0;
            }
            mission->background_image.id = image_group(GROUP_SELECT_MISSION) + rank - FIRST_RANK_WITH_MULTIPLE_SCENARIOS;
        }

        mission->last_scenario = scenario->id;
    }
    return 1;
}

static int get_scenario_offset(int scenario_id)
{
    uint8_t offset_data[4];
    buffer buf;
    buffer_init(&buf, offset_data, 4);
    if (!io_read_file_part_into_buffer(MISSION_PACK_FILE, NOT_LOCALIZED, offset_data, 4, 4 * scenario_id)) {
        return 0;
    }
    return buffer_read_i32(&buf);
}

static int get_scenario_size(int scenario_id)
{
    // Hack - we don't have an "end offset" for the last mission, so to be safe we use the second-to-last mission's size
    // multiplied by two.
    if (scenario_id == NUM_ORIGINAL_SCENARIOS - 1) {
        return get_scenario_size(scenario_id - 1) * 2;
    }
    return get_scenario_offset(scenario_id + 1) - get_scenario_offset(scenario_id);
}

uint8_t *campaign_original_load_scenario(int scenario_id, size_t *length)
{
    *length = 0;
    int offset = get_scenario_offset(scenario_id);
    int size = get_scenario_size(scenario_id);

    if (size <= 0) {
        return 0;
    }

    uint8_t *scenario_data = malloc(sizeof(uint8_t) * size);
    if (!scenario_data) {
        log_error("Problem getting scenario data - memory full", 0, 0);
        return 0;
    }
    buffer buf;
    buffer_init(&buf, scenario_data, size);
    size = io_read_file_part_into_buffer(MISSION_PACK_FILE, NOT_LOCALIZED, scenario_data, size, offset);
    if (size == 0) {
        free(scenario_data);
        return 0;
    }
    *length = size;
    return scenario_data;
}
