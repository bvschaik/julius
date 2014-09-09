#ifndef WALKERACTION_H
#define WALKERACTION_H

void WalkerAction_handle();

int WalkerAction_Rioter_collapseBuilding(int walkerId);
int WalkerAction_Rioter_getTargetBuilding(int *xTile, int *yTile);

int WalkerAction_TradeCaravan_canBuy(int walkerId, int buildingId, int empireCityId);
int WalkerAction_TradeCaravan_canSell(int walkerId, int buildingId, int empireCityId);

int WalkerAction_TradeShip_isBuyingOrSelling(int walkerId);

void WalkerAction_TowerSentry_reroute();
void WalkerAction_HippodromeHorse_reroute();

void WalkerAction_Common_handleCorpse(int walkerId);
void WalkerAction_Common_handleAttack(int walkerId);
void WalkerAction_Common_setCartOffset(int walkerId, int direction);
void WalkerAction_Common_setCrossCountryDestination(int walkerId, struct Data_Walker *w, int xDst, int yDst);

int WalkerAction_CombatSoldier_getMissileTarget(int walkerId, int maxDistance, int *xTile, int *yTile);
int WalkerAction_CombatSoldier_getTarget(int x, int y, int maxDistance);
int WalkerAction_CombatWolf_getTarget(int x, int y, int maxDistance);
int WalkerAction_CombatEnemy_getTarget(int x, int y);
int WalkerAction_CombatEnemy_getMissileTarget(int enemyId, int maxDistance, int attackCitizens, int *xTile, int *yTile);

void WalkerAction_Combat_attackWalker(int walkerId, int targetWalkerId);

// walker action callbacks
void WalkerAction_nobody(int walkerId);
// migrant
void WalkerAction_immigrant(int walkerId);
void WalkerAction_emigrant(int walkerId);
void WalkerAction_homeless(int walkerId);
// cartpusher
void WalkerAction_cartpusher(int walkerId);
void WalkerAction_warehouseman(int walkerId);
// docker
void WalkerAction_docker(int walkerId);
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
void WalkerAction_nativeTrader(int walkerId);
// native
void WalkerAction_indigenousNative(int walkerId);
// soldier
void WalkerAction_soldier(int walkerId);
void WalkerAction_militaryStandard(int walkerId);
// missile
void WalkerAction_explosionCloud(int walkerId);
void WalkerAction_arrow(int walkerId);
void WalkerAction_spear(int walkerId);
void WalkerAction_javelin(int walkerId);
void WalkerAction_bolt(int walkerId);
// rioter
void WalkerAction_protestor(int walkerId);
void WalkerAction_criminal(int walkerId);
void WalkerAction_rioter(int walkerId);
// water
void WalkerAction_fishingBoat(int walkerId);
void WalkerAction_flotsam(int walkerId);
void WalkerAction_shipwreck(int walkerId);
// wall
void WalkerAction_ballista(int walkerId);
void WalkerAction_towerSentry(int walkerId);
// enemy
void WalkerAction_enemy43_Spear(int walkerId);
void WalkerAction_enemy44_Sword(int walkerId);
void WalkerAction_enemy45_Sword(int walkerId);
void WalkerAction_enemy46_Camel(int walkerId);
void WalkerAction_enemy47_Elephant(int walkerId);
void WalkerAction_enemy48_Chariot(int walkerId);
void WalkerAction_enemy49_FastSword(int walkerId);
void WalkerAction_enemy50_Sword(int walkerId);
void WalkerAction_enemy51_Spear(int walkerId);
void WalkerAction_enemy52_MountedArcher(int walkerId);
void WalkerAction_enemy53_Axe(int walkerId);
void WalkerAction_enemy54_Gladiator(int walkerId);
void WalkerAction_enemyCaesarLegionary(int walkerId);
// animal
void WalkerAction_seagulls(int walkerId);
void WalkerAction_sheep(int walkerId);
void WalkerAction_wolf(int walkerId);
void WalkerAction_zebra(int walkerId);
void WalkerAction_hippodromeHorse(int walkerId);

#endif
