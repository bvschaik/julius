#ifndef WALKERACTION_H
#define WALKERACTION_H

int WalkerAction_rioterCollapseBuilding(int walkerId);

void WalkerAction_Common_handleCorpse(int walkerId);
void WalkerAction_Common_handleAttack(int walkerId);

int WalkerAction_TradeCaravan_canBuy(int walkerId, int buildingId, int empireCityId);
int WalkerAction_TradeCaravan_canSell(int walkerId, int buildingId, int empireCityId);

int WalkerAction_TradeShip_canBuyOrSell(int walkerId);


#endif
