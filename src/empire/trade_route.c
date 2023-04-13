#include "trade_route.h"

#include "core/array.h"
#include "core/log.h"
#include "game/save_version.h"

#include <string.h>

typedef struct {
    int limit[RESOURCE_MAX];
    int traded[RESOURCE_MAX];
} route_resource;

static array(route_resource) routes;

int trade_route_init(void)
{
    if (!array_init(routes, LEGACY_MAX_ROUTES, 0, 0)) {
        log_error("Unable to create memory for trade routes. The game will now crash.", 0, 0);
        return 0;
    }
    // Discard route 0
    array_advance(routes);
    return 1;
}

int trade_route_new(void)
{
    array_advance(routes);
    return routes.size - 1;
}

int trade_route_count(void)
{
    return routes.size;
}

int trade_route_is_valid(int route_id)
{
    route_resource *route = array_item(routes, route_id);
    return route != 0;
}

void trade_route_set(int route_id, resource_type resource, int limit)
{
    route_resource *route = array_item(routes, route_id);
    route->limit[resource] = limit;
    route->traded[resource] = 0;
}

int trade_route_limit(int route_id, resource_type resource)
{
    return array_item(routes, route_id)->limit[resource];
}

int trade_route_traded(int route_id, resource_type resource)
{
    return array_item(routes, route_id)->traded[resource];
}

void trade_route_set_limit(int route_id, resource_type resource, int amount)
{
    array_item(routes, route_id)->limit[resource] = amount;
}

int trade_route_legacy_increase_limit(int route_id, resource_type resource)
{
    route_resource *route = array_item(routes, route_id);
    switch (route->limit[resource]) {
        case 0: route->limit[resource] = 15; break;
        case 15: route->limit[resource] = 25; break;
        case 25: route->limit[resource] = 40; break;
    }
    return route->limit[resource];
}

int trade_route_legacy_decrease_limit(int route_id, resource_type resource)
{
    route_resource *route = array_item(routes, route_id);
    switch (route->limit[resource]) {
        case 40: route->limit[resource] = 25; break;
        case 25: route->limit[resource] = 15; break;
        case 15: route->limit[resource] = 0; break;
    }
    return route->limit[resource];
}

void trade_route_increase_traded(int route_id, resource_type resource)
{
    array_item(routes, route_id)->traded[resource]++;
}

void trade_route_reset_traded(int route_id)
{
    route_resource *route = array_item(routes, route_id);
    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        route->traded[r] = 0;
    }
}

int trade_route_limit_reached(int route_id, resource_type resource)
{
    route_resource *route = array_item(routes, route_id);
    return route->traded[resource] >= route->limit[resource];
}

void trade_routes_save_state(buffer *limit, buffer *traded)
{   
    int buf_size = sizeof(int32_t) * RESOURCE_MAX * routes.size;
    uint8_t *buf_data = malloc(buf_size + sizeof(int32_t));
    buffer_init(limit, buf_data, buf_size + sizeof(int32_t));
    buffer_write_i32(limit, routes.size);

    buf_data = malloc(buf_size);
    buffer_init(traded, buf_data, buf_size);  

    route_resource *route;
    array_foreach(routes, route) {
        for (resource_type r = 0; r < RESOURCE_MAX; r++) {
            buffer_write_i32(limit, route->limit[r]);
            buffer_write_i32(traded, route->traded[r]);
        }
    }
}

void trade_routes_load_state(buffer *limit, buffer *traded, int version)
{
    int routes_to_load = version <= SAVE_GAME_LAST_STATIC_SCENARIO_OBJECTS ? LEGACY_MAX_ROUTES : buffer_read_i32(limit);
    if (!array_init(routes, LEGACY_MAX_ROUTES, 0, 0) || !array_expand(routes, routes_to_load)) {
        log_error("Unable to create memory for trade routes. The game will now crash.", 0, 0);
        return;
    }
    for (int i = 0; i < routes_to_load; i++) {
        route_resource *route = array_next(routes);
        for (int r = 0; r < resource_total_mapped(); r++) {
            route->limit[resource_remap(r)] = buffer_read_i32(limit);
            route->traded[resource_remap(r)] = buffer_read_i32(traded);
        }
    }
}
