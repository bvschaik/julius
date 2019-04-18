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
