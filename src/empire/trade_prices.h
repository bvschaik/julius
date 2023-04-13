#ifndef EMPIRE_TRADE_PRICES_H
#define EMPIRE_TRADE_PRICES_H

#include "core/buffer.h"
#include "game/resource.h"

/**
 * @file
 * Trade prices.
 */

/**
 * Reset trade prices to the default
 */
void trade_prices_reset(void);

/**
 * Get the base buy price for the resource, without any discounts or penalties applied
 * @param resource Resource
 */
int trade_price_base_buy(resource_type resource);

/**
 * Get the buy price for the resource
 * @param resource Resource
 * @param int land_trader 1 if land_trader 0 else sea_trader
 */
int trade_price_buy(resource_type resource, int land_trader);

/**
 * Get the base sell price for the resource, without any discounts or penalties applied
 * @param resource Resource
 */
int trade_price_base_sell(resource_type resource);

/**
 * Get the sell price for the resource
 * @param resource Resource
 * @param int land_trader 1 if land_trader 0 else sea_trader
 */
int trade_price_sell(resource_type resource, int land_trader);

/**
 * Change the trade price for resource by amount
 * @param resource Resource to change
 * @param amount Amount to change, can be positive or negative
 * @return True if the price has been changed
 */
int trade_price_change(resource_type resource, int amount);

/**
 * Save trade prices to buffer
 * @param buf Buffer
 */
void trade_prices_save_state(buffer *buf);

/**
 * Load trade prices from buffer
 * @param buf Buffer
 */
void trade_prices_load_state(buffer *buf);

#endif // EMPIRE_TRADE_PRICES_H
