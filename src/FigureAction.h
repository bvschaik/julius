#ifndef FIGUREACTION_H
#define FIGUREACTION_H

#include "data/figure.hpp"

enum
{
    TradeShipState_None = 0,
    TradeShipState_Buying = 1,
    TradeShipState_Selling = 2,
};

void FigureAction_handle();

int FigureAction_Rioter_collapseBuilding(int figureId);

int FigureAction_TradeCaravan_canBuy(int figureId, int buildingId, int empireCityId);
int FigureAction_TradeCaravan_canSell(int figureId, int buildingId, int empireCityId);

int FigureAction_TradeShip_isBuyingOrSelling(int figureId);

void FigureAction_TowerSentry_reroute();
void FigureAction_HippodromeHorse_reroute();

int FigureAction_HerdEnemy_moveFormationTo(int formationId, int x, int y, int *xTile, int *yTile);

void FigureAction_Common_handleCorpse(int figureId);
void FigureAction_Common_handleAttack(int figureId);
void FigureAction_Common_setCartOffset(int figureId, int direction);
void FigureAction_Common_setCrossCountryDestination(int figureId, struct Data_Figure* f, int xDst, int yDst);

int FigureAction_CombatSoldier_getMissileTarget(int figureId, int maxDistance, int *xTile, int *yTile);
int FigureAction_CombatSoldier_getTarget(int x, int y, int maxDistance);
int FigureAction_CombatWolf_getTarget(int x, int y, int maxDistance);
int FigureAction_CombatEnemy_getTarget(int x, int y);
int FigureAction_CombatEnemy_getMissileTarget(int enemyId, int maxDistance, int attackCitizens, int *xTile, int *yTile);

void FigureAction_Combat_attackFigure(int figureId, int targetfigureId);

// figure action callbacks
void FigureAction_nobody(int figureId);
// migrant
void FigureAction_immigrant(int figureId);
void FigureAction_emigrant(int figureId);
void FigureAction_homeless(int figureId);
// cartpusher
void FigureAction_cartpusher(int figureId);
void FigureAction_warehouseman(int figureId);
// docker
void FigureAction_docker(int figureId);
// service
void FigureAction_taxCollector(int figureId);
void FigureAction_engineer(int figureId);
void FigureAction_prefect(int figureId);
void FigureAction_worker(int figureId);
// culture/entertainer
void FigureAction_entertainer(int figureId);
void FigureAction_priest(int figureId);
void FigureAction_schoolChild(int figureId);
void FigureAction_teacher(int figureId);
void FigureAction_librarian(int figureId);
void FigureAction_barber(int figureId);
void FigureAction_bathhouseWorker(int figureId);
void FigureAction_doctor(int figureId);
void FigureAction_surgeon(int figureId);
void FigureAction_missionary(int figureId);
void FigureAction_patrician(int figureId);
void FigureAction_laborSeeker(int figureId);
void FigureAction_marketTrader(int figureId);
// market
void FigureAction_marketBuyer(int figureId);
void FigureAction_deliveryBoy(int figureId);
// trader
void FigureAction_tradeCaravan(int figureId);
void FigureAction_tradeCaravanDonkey(int figureId);
void FigureAction_tradeShip(int figureId);
void FigureAction_nativeTrader(int figureId);
// native
void FigureAction_indigenousNative(int figureId);
// soldier
void FigureAction_soldier(int figureId);
void FigureAction_militaryStandard(int figureId);
// missile
void FigureAction_explosionCloud(int figureId);
void FigureAction_arrow(int figureId);
void FigureAction_spear(int figureId);
void FigureAction_javelin(int figureId);
void FigureAction_bolt(int figureId);
// rioter
void FigureAction_protestor(int figureId);
void FigureAction_criminal(int figureId);
void FigureAction_rioter(int figureId);
// water
void FigureAction_fishingBoat(int figureId);
void FigureAction_flotsam(int figureId);
void FigureAction_shipwreck(int figureId);
// wall
void FigureAction_ballista(int figureId);
void FigureAction_towerSentry(int figureId);
// enemy
void FigureAction_enemy43_Spear(int figureId);
void FigureAction_enemy44_Sword(int figureId);
void FigureAction_enemy45_Sword(int figureId);
void FigureAction_enemy46_Camel(int figureId);
void FigureAction_enemy47_Elephant(int figureId);
void FigureAction_enemy48_Chariot(int figureId);
void FigureAction_enemy49_FastSword(int figureId);
void FigureAction_enemy50_Sword(int figureId);
void FigureAction_enemy51_Spear(int figureId);
void FigureAction_enemy52_MountedArcher(int figureId);
void FigureAction_enemy53_Axe(int figureId);
void FigureAction_enemy54_Gladiator(int figureId);
void FigureAction_enemyCaesarLegionary(int figureId);
// animal
void FigureAction_seagulls(int figureId);
void FigureAction_sheep(int figureId);
void FigureAction_wolf(int figureId);
void FigureAction_zebra(int figureId);
void FigureAction_hippodromeHorse(int figureId);

#endif
