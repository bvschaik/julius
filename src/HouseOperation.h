#ifndef HOUSEOPERATION_H
#define HOUSEOPERATION_H

void HouseOperation_checkMerge(int buildingId);

int HouseOperation_canExpand(int buildingId, int tiles);

void HouseOperation_split(int buildingId, int tiles);

// TODO split + storeInventory to HouseOperation internal
void HouseOperation_expandToLargeInsula(int buildingId);
void HouseOperation_expandToLargeVilla(int buildingId);
void HouseOperation_expandToLargePalace(int buildingId);

void HouseOperation_splitFromLargeInsula(int buildingId);
void HouseOperation_splitFromLargeVilla(int buildingId);
void HouseOperation_splitFromLargePalace(int buildingId);

void HouseOperation_changeTo(int buildingId, int buildingType);
void HouseOperation_changeToVacantLot(int buildingId);

#endif
