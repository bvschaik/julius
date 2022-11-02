#ifndef EMPIRE_TRADE_ROUTE_H
#define EMPIRE_TRADE_ROUTE_H

#include "core/buffer.h"
#include "game/resource.h"

void trade_route_init(int route_id, resource_type resource, int limit);

int trade_route_limit(int route_id, resource_type resource);

int trade_route_traded(int route_id, resource_type resource);

void trade_route_set_limit(int route_id, resource_type resource, int amount);

/**
 * Increases the trade limit of the resource
 * @param route_id Trade route
 * @param resource Resource
 * @return True on success, false if the limit couldn't be increased
 */
int trade_route_legacy_increase_limit(int route_id, resource_type resource);

/**
 * Decreases the trade limit of the resource
 * @param route_id Trade route
 * @param resource Resource
 * @return True on success, false if the limit couldn't be decreased
 */
int trade_route_legacy_decrease_limit(int route_id, resource_type resource);

void trade_route_increase_traded(int route_id, resource_type resource);

void trade_route_reset_traded(int route_id);

int trade_route_limit_reached(int route_id, resource_type resource);

void trade_routes_save_state(buffer *limit, buffer *traded);

void trade_routes_load_state(buffer *limit, buffer *traded);

#endif // EMPIRE_TRADE_ROUTE_H
