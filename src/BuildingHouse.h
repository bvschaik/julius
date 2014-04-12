#ifndef BuildingHouse_H
#define BuildingHouse_H

int BuildingHouse_canExpand(int buildingId, int tiles);

void BuildingHouse_checkForCorruption(int buildingId);

void BuildingHouse_checkMerge(int buildingId);

void BuildingHouse_expandToLargeInsula(int buildingId);
void BuildingHouse_expandToLargeVilla(int buildingId);
void BuildingHouse_expandToLargePalace(int buildingId);

void BuildingHouse_devolveFromLargeInsula(int buildingId);
void BuildingHouse_devolveFromLargeVilla(int buildingId);
void BuildingHouse_devolveFromLargePalace(int buildingId);

void BuildingHouse_changeTo(int buildingId, int buildingType);
void BuildingHouse_changeToVacantLot(int buildingId);

#endif
