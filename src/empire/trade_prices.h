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
void trade_prices_reset();

/**
 * Get the buy price for the resource
 * @param resource Resource
 */
int trade_price_buy(resource_type resource);

/**
 * Get the sell price for the resource
 * @param resource Resource
 */
int trade_price_sell(resource_type resource);

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
