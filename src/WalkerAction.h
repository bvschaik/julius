#ifndef WALKERACTION_H
#define WALKERACTION_H

int WalkerAction_Rioter_collapseBuilding(int walkerId);
int WalkerAction_Rioter_getTargetBuilding(int *xTile, int *yTile);

int WalkerAction_TradeCaravan_canBuy(int walkerId, int buildingId, int empireCityId);
int WalkerAction_TradeCaravan_canSell(int walkerId, int buildingId, int empireCityId);

int WalkerAction_TradeShip_canBuyOrSell(int walkerId);

void WalkerAction_Common_handleCorpse(int walkerId);
void WalkerAction_Common_handleAttack(int walkerId);
void WalkerAction_Common_setCartOffset(int walkerId, int direction);
void WalkerAction_Common_setCrossCountryDestination(int walkerId, struct Data_Walker *w, int xDst, int yDst);


#endif
