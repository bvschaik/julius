#ifndef EMPIRE_H
#define EMPIRE_H

void Empire_load(int isCustomScenario, int empireId);
void Empire_initTradeRoutes();
void Empire_determineDistantBattleCity();
void Empire_resetYearlyTradeAmounts();

int Empire_cityBuysResource(int index, int resource);
int Empire_citySellsResource(int index, int resource);

#endif
