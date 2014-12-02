#ifndef DATA_TRADE_H
#define DATA_TRADE_H

struct Data_TradePrice {
	int buy;
	int sell;
};

extern struct Data_TradePrice Data_TradePrices[16];

#endif
