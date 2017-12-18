#ifndef BuildingHouse_H
#define BuildingHouse_H

#include "building/building.h"

int BuildingHouse_canExpand(building *b, int numTiles);

void BuildingHouse_checkForCorruption(building *b);

void BuildingHouse_checkMerge(building *b);

void BuildingHouse_expandToLargeInsula(building *b);
void BuildingHouse_expandToLargeVilla(building *b);
void BuildingHouse_expandToLargePalace(building *b);

void BuildingHouse_devolveFromLargeInsula(building *b);
void BuildingHouse_devolveFromLargeVilla(building *b);
void BuildingHouse_devolveFromLargePalace(building *b);

void BuildingHouse_changeTo(building *b, int buildingType);
void BuildingHouse_changeToVacantLot(building *b);

#endif
