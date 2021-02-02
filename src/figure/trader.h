#ifndef FIGURE_TRADER_H
#define FIGURE_TRADER_H

#include "core/buffer.h"
#include "game/resource.h"

/**
 * @file
 * Trade figure extra info
 */

/**
 * Clears all traders
 */
void traders_clear(void);

/**
 * Creates a trader
 * @return ID of the new trader
 */
int trader_create(void);

/**
 * Record that the trader has bought a resource from the city
 * @param trader_id Trader
 * @param resource Resource bought
 */
void trader_record_bought_resource(int trader_id, resource_type resource);

/**
 * Record that the trader has sold a resource to the city
 * @param trader_id Trader
 * @param resource Resource sold
 */
void trader_record_sold_resource(int trader_id, resource_type resource);

/**
 * Gets the amount bought of the given resource
 * @param trader_id Trader
 * @param resource Resource
 * @return Amount of resource bought by the trader from the city
 */
int trader_bought_resources(int trader_id, resource_type resource);

/**
 * Gets the amount sold of the given resource
 * @param trader_id Trader
 * @param resource Resource
 * @return Amount of resource sold by the trader to the city
 */
int trader_sold_resources(int trader_id, resource_type resource);

/**
 * Check whether this trader has bought/sold any items
 * @param trader_id Trader
 * @return True if the trader has bought or sold at least one item
 */
int trader_has_traded(int trader_id);

/**
 * Check whether a trade ship has traded the maximum amount
 * @param trader_id Trader
 * @return True if the trader has either bought or sold the max amount (12 or 16 with mercury monument)
 */
int trader_has_traded_max(int trader_id);

/**
 * Check whether a trade ship has bought the maximum amount
 * @param trader_id Trader
 * @return True if the trader has bought the max amount (12 or 16 with mercury monument)
 */
int trader_has_bought_max(int trader_id);


/**
 * Check whether a trade ship has sold the maximum amount
 * @param trader_id Trader
 * @return True if the trader has sold the max amount (12 or 16 with mercury monument)
 */
int trader_has_sold_max(int trader_id);

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
