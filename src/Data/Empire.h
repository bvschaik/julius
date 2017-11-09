#ifndef DATA_EMPIRE_H
#define DATA_EMPIRE_H

#define MAX_EMPIRE_OBJECTS 200

extern struct _Data_Empire {
	int scrollX;
	int scrollY;
	int selectedObject;
} Data_Empire;

extern struct _Data_Empire_Sizes {
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
	unsigned char type;
	unsigned char inUse;
	unsigned char animationIndex;
	unsigned char __padding;
	short x;
	short y;
	short width;
	short height;
	short graphicId;
	short graphicIdExpanded;
	unsigned char __unused2;
	unsigned char distantBattleTravelMonths;
	short __unused3;
	short xExpanded;
	short yExpanded;
	unsigned char cityType;
	unsigned char cityNameId;
	unsigned char tradeRouteId;
	unsigned char tradeRouteOpen;
	short tradeRouteCostToOpen;
	unsigned char citySells[10];
	unsigned char ownerCityIndex;
	unsigned char __unused4;
	unsigned char cityBuys[8];
	unsigned char invasionPathId;
	unsigned char invasionYears;
	short trade40;
	short trade25;
	short trade15;
	unsigned char __padding2[6];
} Data_Empire_Objects[MAX_EMPIRE_OBJECTS];

#endif
