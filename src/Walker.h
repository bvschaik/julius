#ifndef WALKER_H
#define WALKER_H

int Walker_determinePhrase(int walkerId);
void Walker_playPhrase(int walkerId);


int Walker_TradeCaravan_isBuying(int walkerId, int buildingId, int empireCityId);
int Walker_TradeCaravan_isSelling(int walkerId, int buildingId, int empireCityId);

int Walker_TradeShip_isBuyingOrSelling(int walkerId);

#endif
