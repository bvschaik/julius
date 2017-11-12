#ifndef EMPIRE_H
#define EMPIRE_H

void Empire_load(int isCustomScenario, int empireId);
void Empire_initScroll();
void Empire_initCities();
void Empire_initTradeAmountCodes();
void Empire_determineDistantBattleCity();

int Empire_cityBuysResource(int objectId, int resource);
int Empire_citySellsResource(int objectId, int resource);

#endif
