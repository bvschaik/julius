#ifndef BuildingHouse_H
#define BuildingHouse_H

void BuildingHouse_checkMerge(int buildingId);

int BuildingHouse_canExpand(int buildingId, int tiles);

void BuildingHouse_split(int buildingId, int tiles);

// TODO split + storeInventory to BuildingHouse internal
void BuildingHouse_expandToLargeInsula(int buildingId);
void BuildingHouse_expandToLargeVilla(int buildingId);
void BuildingHouse_expandToLargePalace(int buildingId);

void BuildingHouse_splitFromLargeInsula(int buildingId);
void BuildingHouse_splitFromLargeVilla(int buildingId);
void BuildingHouse_splitFromLargePalace(int buildingId);

void BuildingHouse_changeTo(int buildingId, int buildingType);
void BuildingHouse_changeToVacantLot(int buildingId);

#endif
