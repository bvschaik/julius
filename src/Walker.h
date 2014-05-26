#ifndef WALKER_H
#define WALKER_H

void Walker_clearList();
int Walker_create(int walkerType, int x, int y, char direction);
void Walker_delete(int walkerId);

int Walker_createDustCloud(int x, int y, int size);
void Walker_createFishingPoints();
void Walker_createHerds();
void Walker_createFlotsam(int xEntry, int yEntry, int hasWater);

void Walker_addToTileList(int walkerId);
void Walker_updatePositionInTileList(int walkerId);
void Walker_removeFromTileList(int walkerId);

void Walker_killTowerSentriesAt(int x, int y);
void Walker_sinkAllShips();
int Walker_getCitizenOnSameTile(int walkerId);
int Walker_getNonCitizenOnSameTile(int walkerId);
int Walker_hasNearbyEnemy(int x1, int y1, int x2, int y2);

int Walker_determinePhrase(int walkerId);
int Walker_playPhrase(int walkerId);

int Walker_TradeCaravan_isBuying(int walkerId, int buildingId, int empireCityId);
int Walker_TradeCaravan_isSelling(int walkerId, int buildingId, int empireCityId);

int Walker_TradeShip_isBuyingOrSelling(int walkerId);

void WalkerName_set(int walkerId);
void WalkerName_init();

void WalkerRoute_clearList();
void WalkerRoute_clean();
int WalkerRoute_getNumAvailable();
void WalkerRoute_remove(int walkerId);

#endif
