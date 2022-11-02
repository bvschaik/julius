#include "trade_route.h"

#define MAX_ROUTES 20

struct route_resource {
    int limit;
    int traded;
};

static struct route_resource data[MAX_ROUTES][RESOURCE_MAX];

void trade_route_init(int route_id, resource_type resource, int limit)
{
    data[route_id][resource].limit = limit;
    data[route_id][resource].traded = 0;
}

int trade_route_limit(int route_id, resource_type resource)
{
    return data[route_id][resource].limit;
}

int trade_route_traded(int route_id, resource_type resource)
{
    return data[route_id][resource].traded;
}

void trade_route_set_limit(int route_id, resource_type resource, int amount)
{
    data[route_id][resource].limit = amount;
}

int trade_route_legacy_increase_limit(int route_id, resource_type resource)
{
    switch (data[route_id][resource].limit) {
        case 0: data[route_id][resource].limit = 15; break;
        case 15: data[route_id][resource].limit = 25; break;
        case 25: data[route_id][resource].limit = 40; break;
        default: return 0;
    }
    return 1;
}

int trade_route_legacy_decrease_limit(int route_id, resource_type resource)
{
    switch (data[route_id][resource].limit) {
        case 40: data[route_id][resource].limit = 25; break;
        case 25: data[route_id][resource].limit = 15; break;
        case 15: data[route_id][resource].limit = 0; break;
        default: return 0;
    }
    return 1;
}

void trade_route_increase_traded(int route_id, resource_type resource)
{
    data[route_id][resource].traded++;
}

void trade_route_reset_traded(int route_id)
{
    for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        data[route_id][r].traded = 0;
    }
}

int trade_route_limit_reached(int route_id, resource_type resource)
{
    return data[route_id][resource].traded >= data[route_id][resource].limit;
}

void trade_routes_save_state(buffer *limit, buffer *traded)
{
    for (int route_id = 0; route_id < MAX_ROUTES; route_id++) {
        for (int r = 0; r < RESOURCE_MAX; r++) {
            buffer_write_i32(limit, data[route_id][r].limit);
            buffer_write_i32(traded, data[route_id][r].traded);
        }
    }
}

void trade_routes_load_state(buffer *limit, buffer *traded)
{
    for (int route_id = 0; route_id < MAX_ROUTES; route_id++) {
        for (int r = 0; r < RESOURCE_MAX; r++) {
            data[route_id][r].limit = buffer_read_i32(limit);
            data[route_id][r].traded = buffer_read_i32(traded);
        }
    }
}
