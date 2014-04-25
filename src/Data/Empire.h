#ifndef DATA_EMPIRE_H
#define DATA_EMPIRE_H

extern struct Data_Empire {
	int scrollX;
	int scrollY;
	int selectedObject;
} Data_Empire;

extern struct Data_Empire_Sizes {
	int width;
	int height;
	int borderTop;
	int borderSides;
	int borderBottom;
} Data_Empire_Sizes;

extern struct Data_Empire_Index {
	short initialScrollX;
	short initialScrollY;
	short inUse;
	short __padding[13];
} Data_Empire_Index[40];

extern struct Data_Empire_Object {
	char type;
	char inUse;
	short animationIndex;
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
	unsigned char ownerCityIndex;
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
	unsigned char cityType;
	unsigned char cityNameId;
	unsigned char routeId;
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
	int maxPerYear[20][16];
	int tradedThisYear[20][16];
} Data_Empire_Trade;

#endif
