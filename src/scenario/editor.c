#include "editor.h"

#include "scenario/data.h"

void scenario_editor_request_get(int index, editor_request *request)
{
    request->year = scenario.requests[index].year;
    request->amount = scenario.requests[index].amount;
    request->resource = scenario.requests[index].resource;
    request->deadline_years = scenario.requests[index].deadline_years;
    request->favor= scenario.requests[index].favor;
}

void scenario_editor_invasion_get(int index, editor_invasion *invasion)
{
    invasion->year = scenario.invasions[index].year;
    invasion->type = scenario.invasions[index].type;
    invasion->amount = scenario.invasions[index].amount;
    invasion->from = scenario.invasions[index].from;
    invasion->attack_type = scenario.invasions[index].attack_type;
}

void scenario_editor_cycle_image(int forward)
{
    if (forward) {
        scenario.image_id++;
    } else {
        scenario.image_id--;
    }
    if (scenario.image_id < 0) {
        scenario.image_id = 15;
    }
    if (scenario.image_id > 15) {
        scenario.image_id = 0;
    }
}

void scenario_editor_set_enemy(int enemy_id)
{
    scenario.enemy_id = enemy_id;
}
