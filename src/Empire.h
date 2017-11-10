#ifndef EMPIRE_H
#define EMPIRE_H

void Empire_load(int isCustomScenario, int empireId);
void Empire_initScroll();
void Empire_initCities();
void Empire_initTradeAmountCodes();
void Empire_determineDistantBattleCity();
void Empire_handleExpandEvent();

int Empire_cityBuysResource(int objectId, int resource);
int Empire_citySellsResource(int objectId, int resource);
int Empire_canExportResourceToCity(int cityId, int resource);
int Empire_canImportResourceFromCity(int cityId, int resource);

int Empire_ourCityCanProduceResource(int resource);
int Empire_ourCityCanProduceResourcePotentially(int resource);

#endif
