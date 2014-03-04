#ifndef EMPIRE_H
#define EMPIRE_H

void Empire_scrollMap(int direction);
void Empire_checkScrollBoundaries();

void Empire_load(int isCustomScenario, int empireId);
void Empire_initCities();
void Empire_initTradeAmountCodes();
void Empire_determineDistantBattleCity();
void Empire_resetYearlyTradeAmounts();

int Empire_cityBuysResource(int objectId, int resource);
int Empire_citySellsResource(int objectId, int resource);
int Empire_canExportResourceToCity(int cityId, int resource);
int Empire_canImportResourceFromCity(int cityId, int resource);

int Empire_canImportResource(int resource);
int Empire_canImportResourcePotentially(int resource);
int Empire_canExportResource(int resource);

int Empire_ourCityCanProduceResource(int resource);
int Empire_ourCityCanProduceResourcePotentially(int resource);

int Empire_getCityForObject(int empireObjectId);
int Empire_getCityForTradeRoute(int routeId);
int Empire_isTradeWithCityOpen(int routeId);

#endif
