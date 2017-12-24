#ifndef FIGUREACTION_H
#define FIGUREACTION_H

#include "figure/figure.h"

enum {
	TradeShipState_None = 0,
	TradeShipState_Buying = 1,
	TradeShipState_Selling = 2,
};

void FigureAction_handle();

int FigureAction_Rioter_collapseBuilding(figure *f);

int FigureAction_TradeCaravan_canBuy(int figureId, int buildingId, int empireCityId);
int FigureAction_TradeCaravan_canSell(int figureId, int buildingId, int empireCityId);

int FigureAction_TradeShip_isBuyingOrSelling(int figureId);

void FigureAction_TowerSentry_reroute();

int FigureAction_HerdEnemy_moveFormationTo(int formationId, int x, int y, int *xTile, int *yTile);

void FigureAction_Common_handleCorpse(figure *f);
void FigureAction_Common_handleAttack(figure *f);
void FigureAction_Common_setCrossCountryDestination(figure *f, int xDst, int yDst);

int FigureAction_CombatSoldier_getMissileTarget(figure *shooter, int maxDistance, int *xTile, int *yTile);
int FigureAction_CombatSoldier_getTarget(int x, int y, int maxDistance);
int FigureAction_CombatWolf_getTarget(int x, int y, int maxDistance);
int FigureAction_CombatEnemy_getTarget(int x, int y);
int FigureAction_CombatEnemy_getMissileTarget(figure *enemy, int maxDistance, int attackCitizens, int *xTile, int *yTile);

void FigureAction_Combat_attackFigureAt(figure *f, int grid_offset);

// figure action callbacks
// cartpusher
void FigureAction_cartpusher(figure *f);
void FigureAction_warehouseman(figure *f);
// docker
void FigureAction_docker(figure *f);
// service
void FigureAction_taxCollector(figure *f);
void FigureAction_engineer(figure *f);
void FigureAction_prefect(figure *f);
void FigureAction_worker(figure *f);
// culture/entertainer
void FigureAction_priest(figure *f);
void FigureAction_schoolChild(figure *f);
void FigureAction_teacher(figure *f);
void FigureAction_librarian(figure *f);
void FigureAction_barber(figure *f);
void FigureAction_bathhouseWorker(figure *f);
void FigureAction_doctor(figure *f);
void FigureAction_surgeon(figure *f);
void FigureAction_missionary(figure *f);
void FigureAction_patrician(figure *f);
void FigureAction_laborSeeker(figure *f);
void FigureAction_marketTrader(figure *f);
// market
void FigureAction_marketBuyer(figure *f);
void FigureAction_deliveryBoy(figure *f);
// trader
void FigureAction_tradeCaravan(figure *f);
void FigureAction_tradeCaravanDonkey(figure *f);
void FigureAction_tradeShip(figure *f);
void FigureAction_nativeTrader(figure *f);
// native
void FigureAction_indigenousNative(figure *f);
// soldier
void FigureAction_soldier(figure *f);
void FigureAction_militaryStandard(figure *f);
// missile
void FigureAction_explosionCloud(figure *f);
void FigureAction_arrow(figure *f);
void FigureAction_spear(figure *f);
void FigureAction_javelin(figure *f);
void FigureAction_bolt(figure *f);
// rioter
void FigureAction_protestor(figure *f);
void FigureAction_criminal(figure *f);
void FigureAction_rioter(figure *f);
// wall
void FigureAction_ballista(figure *f);
void FigureAction_towerSentry(figure *f);
// enemy
void FigureAction_enemy43_Spear(figure *f);
void FigureAction_enemy44_Sword(figure *f);
void FigureAction_enemy45_Sword(figure *f);
void FigureAction_enemy46_Camel(figure *f);
void FigureAction_enemy47_Elephant(figure *f);
void FigureAction_enemy48_Chariot(figure *f);
void FigureAction_enemy49_FastSword(figure *f);
void FigureAction_enemy50_Sword(figure *f);
void FigureAction_enemy51_Spear(figure *f);
void FigureAction_enemy52_MountedArcher(figure *f);
void FigureAction_enemy53_Axe(figure *f);
void FigureAction_enemy54_Gladiator(figure *f);
void FigureAction_enemyCaesarLegionary(figure *f);

#endif
