#ifndef DATA_GRAPHICS_H
#define DATA_GRAPHICS_H

#include "Types.h"

#define GraphicId(index) (Data_Graphics_Main.graphicIds[index])
#define GraphicHeight(id) (Data_Graphics_Main.index[id].height)
#define GraphicWidth(id) (Data_Graphics_Main.index[id].width)
#define GraphicAnimationSpeed(id) (Data_Graphics_Main.index[id].animationSpeedId)
#define GraphicNumAnimationSprites(id) (Data_Graphics_Main.index[id].numAnimationSprites)
#define GraphicAnimationCanReverse(id) (Data_Graphics_Main.index[id].animationCanReverse)
#define GraphicSpriteOffsetX(id) (Data_Graphics_Main.index[id].spriteOffsetX)
#define GraphicSpriteOffsetY(id) (Data_Graphics_Main.index[id].spriteOffsetY)
#define GraphicEnemySpriteOffsetX(id) (Data_Graphics_Enemy.index[id].spriteOffsetX)
#define GraphicEnemySpriteOffsetY(id) (Data_Graphics_Enemy.index[id].spriteOffsetY)

struct Data_Graphics_Index {
	int offset;
	int dataLength;
	int uncompressedLength;
	int unused_0C;
	int invertedImageOffset;
	short width;
	short height;
	int unused_18;
	short unused_1C;
	short numAnimationSprites;
	short unused_20;
	short spriteOffsetX;
	short spriteOffsetY;
	short unused_26[5];
	char animationCanReverse;
	char unused_31;
	unsigned char type;
	char isFullyCompressed;
	char isExternal;
	char hasCompressedPart;
	char unknown_36;
	char buildingSize;
	char bitmapId;
	char unused_39;
	unsigned char animationSpeedId;
	char unused_3B;
	char unknown_3C;
	char unknown_3D;
	short unused_3E;
};

extern struct _Data_Graphics_Main {
	struct {
		int hdrFields[20];
	} header;
	short graphicIds[300];
	char bitmaps[100][200];
	struct Data_Graphics_Index index[10000];
} Data_Graphics_Main;

extern struct _Data_Graphics_Enemy {
	struct Data_Graphics_Index index[801];
} Data_Graphics_Enemy;

extern struct _Data_Graphics_PixelData {
	Color *main;
	Color *enemy;
	//void *empire;
} Data_Graphics_PixelData;

#endif
