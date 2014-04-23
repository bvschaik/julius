#ifndef DATA_TRADER_H
#define DATA_TRADER_H

#define MAX_TRADERS 100

struct Data_Trader {
	int totalBought;
	int totalSold;
	unsigned char boughtResources[16];
	unsigned char soldResources[16];
	int moneyBoughtResources;
	int moneySoldResources;
} Data_Traders[MAX_TRADERS];

#endif
