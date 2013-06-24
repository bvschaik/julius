#ifndef DATA_EMPIRE_H
#define DATA_EMPIRE_H

/*extern struct Data_Empire_Index {
	short scrollX;
	short scrollY;
	short inUse;
} Data_Empire_Index;*/

extern struct Data_Empire_Object {
	char type;
	char inUse;
	char unknown03;
	char __unused1;
	short x;
	short y;
	short width;
	short height;
	short graphicId;
	short graphicIdExpanded;
	char __unused2;
	char distantBattleTravelMonths;
	char __unused3[2];
	short xExpanded;
	short yExpanded;
	char cityType;
	char cityNameId;
	char tradeRouteId;
	char tradeRouteOpen;
	short tradeRouteCostToOpen;
	char citySells[10];
	char ownerCityIndex;
	char unknown990d29;
	char cityBuys[8];
	char invasionPathId;
	char invasionYears;
	short trade40;
	short trade25;
	short trade15;
	char __padding[6];
} Data_Empire_Objects[200];

extern struct Data_Empire_City {
	char inUse;
	char __unused1;
	char cityType;
	char cityNameId;
	char routeId;
	char isOpen;
	char buysResourceFlag[16];
	char sellsResourceFlag[16];
	short costToOpen;
	short __unused2;
	short traderEntryDelay;
	short __unused3;
	short empireObjectId;
	char isSeaTrade;
	char __unused4;
	short traderWalkerId1;
	short traderWalkerId2;
	short traderWalkerId3;
	char __padding[10];
} Data_Empire_Cities[41];

extern struct Data_Empire_Trade {
	char maxPerYear[20][16];
	char tradedThisYear[20][16];
} Data_Empire_Trade;

#endif
