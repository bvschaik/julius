#ifndef DATA_TRADE_H
#define DATA_TRADE_H

struct Data_TradePrice {
	short buy;
	short sell;
};

extern struct Data_TradePrice Data_TradePrices[16];

#endif
