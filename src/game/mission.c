#include "mission.h"

#include "scenario/property.h"

static const struct {
    int peaceful;
    int military;
} MISSION_IDS[12] = {
    {0, 0},
    {1, 1},
    {2, 3},
    {4, 5},
    {6, 7},
    {8, 9},
    {10, 11},
    {12, 13},
    {14, 15},
    {16, 17},
    {18, 19},
    {20, 21},
};

int game_mission_peaceful()
{
    return MISSION_IDS[scenario_campaign_rank()].peaceful;
}

int game_mission_military()
{
    return MISSION_IDS[scenario_campaign_rank()].military;
}
