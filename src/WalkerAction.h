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

int WalkerAction_CombatSoldier_getMissileTarget(int walkerId, int distance, int *xTile, int *yTile);
int WalkerAction_CombatSoldier_getTarget(int x, int y, int distance);
void WalkerAction_CombatSoldier_attackWalker(int walkerId, int targetWalkerId);
int WalkerAction_CombatWolf_getTarget(int x, int y, int range);

// walker action callbacks
void WalkerAction_nobody(int walkerId);
// migrant
void WalkerAction_immigrant(int walkerId);
void WalkerAction_emigrant(int walkerId);
void WalkerAction_homeless(int walkerId);
// cartpusher
void WalkerAction_cartpusher(int walkerId);
void WalkerAction_warehouseman(int walkerId);
// service
void WalkerAction_taxCollector(int walkerId);
void WalkerAction_engineer(int walkerId);
void WalkerAction_prefect(int walkerId);
void WalkerAction_worker(int walkerId);

// culture/entertainer
void WalkerAction_entertainer(int walkerId);
void WalkerAction_priest(int walkerId);
void WalkerAction_schoolChild(int walkerId);
void WalkerAction_teacher(int walkerId);
void WalkerAction_librarian(int walkerId);
void WalkerAction_barber(int walkerId);
void WalkerAction_bathhouseWorker(int walkerId);
void WalkerAction_doctor(int walkerId);
void WalkerAction_surgeon(int walkerId);
void WalkerAction_missionary(int walkerId);
void WalkerAction_patrician(int walkerId);
void WalkerAction_laborSeeker(int walkerId);
void WalkerAction_marketTrader(int walkerId);
// market
void WalkerAction_marketBuyer(int walkerId);
void WalkerAction_deliveryBoy(int walkerId);
// trader
void WalkerAction_tradeCaravan(int walkerId);
void WalkerAction_tradeCaravanDonkey(int walkerId);
void WalkerAction_tradeShip(int walkerId);
// soldier
void WalkerAction_soldier(int walkerId);
// missile
void WalkerAction_explosionCloud(int walkerId);
void WalkerAction_arrow(int walkerId);
void WalkerAction_spear(int walkerId);
void WalkerAction_javelin(int walkerId);
void WalkerAction_bolt(int walkerId);
// rioter
void WalkerAction_protestor(int walkerId);
void WalkerAction_criminal(int walkerId);
// map
void WalkerAction_shipwreck(int walkerId);

// animal
void WalkerAction_seagulls(int walkerId);
void WalkerAction_sheep(int walkerId);
void WalkerAction_wolf(int walkerId);
void WalkerAction_zebra(int walkerId);

#endif
