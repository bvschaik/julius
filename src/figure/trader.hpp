#ifndef FIGURE_TRADER_H
#define FIGURE_TRADER_H

#include "core/buffer.hpp"
#include "game/resource.hpp"

/**
 * @file
 * Trade figure extra info
 */

/**
 * Clears all traders
 */
void traders_clear();

/**
 * Creates a trader
 * @return ID of the new trader
 */
int trader_create();

/**
 * Record that the trader has bought a resource from the city
 * @param traderId Trader
 * @param resource Resource bought
 */
void trader_record_bought_resource(int traderId, resource_type resource);

/**
 * Record that the trader has sold a resource to the city
 * @param traderId Trader
 * @param resource Resource sold
 */
void trader_record_sold_resource(int traderId, resource_type resource);

/**
 * Gets the amount bought of the given resource
 * @param traderId Trader
 * @param resource Resource
 * @return Amount of resource bought by the trader from the city
 */
int trader_bought_resources(int traderId, resource_type resource);

/**
 * Gets the amount sold of the given resource
 * @param traderId Trader
 * @param resource Resource
 * @return Amount of resource sold by the trader to the city
 */
int trader_sold_resources(int traderId, resource_type resource);

/**
 * Check whether this trader has bought/sold any items
 * @param traderId Trader
 * @return True if the trader has bought or sold at least one item
 */
int trader_has_traded(int traderId);

/**
 * Check whether a trade ship has traded the maximum amount
 * @param traderId Trader
 * @return True if the trader has either bought or sold the max amount (12)
 */
int trader_has_traded_max(int traderId);

/**
 * Save state to buffer
 * @param buf Buffer
 */
void traders_save_state(buffer *buf);

/**
 * Load state from buffer
 * @param buf Buffer
 */
void traders_load_state(buffer *buf);

#endif // FIGURE_TRADE_INFO_H
