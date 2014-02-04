#ifndef DATA_GRAPHICS_H
#define DATA_GRAPHICS_H

#define GraphicId(index) (Data_Graphics_Main.graphicIds[index])
#define GraphicHasAnimationSprite(id) (Data_Graphics_Main.index[id].animationSpeedId)
#define GraphicAnimationTopOffset(id) (Data_Graphics_Main.index[id].spriteTopOffset)
#define GraphicAnimationLeftOffset(id) (Data_Graphics_Main.index[id].spriteLeftOffset)

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
	short spriteTopOffset;
	short spriteLeftOffset;
	short unused_26[6];
	unsigned char type;
	char isFullyCompressed;
	char isExternal;
	char hasCompressedPart;
	char unknown_36;
	char buildingSize;
	char bitmapId;
	char unused_39;
	char animationSpeedId;
	char unused_3B;
	char unknown_3C;
	char unknown_3D;
	short unused_3E;
};

extern struct Data_Graphics_Main {
	struct {
		int hdrFields[20];
	} header;
	short graphicIds[300];
	char bitmaps[100][200];
	struct Data_Graphics_Index index[10000];
} Data_Graphics_Main;

extern struct Data_Graphics_Enemy {
	struct Data_Graphics_Index index[801];
} Data_Graphics_Enemy;

extern struct Data_Graphics_PixelData {
	char *main;
	char *enemy;
	//void *empire;
} Data_Graphics_PixelData;

#endif
