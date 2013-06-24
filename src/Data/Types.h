#ifndef TYPES_H
#define TYPES_H

typedef unsigned short Color;

typedef enum {
	Font_NormalPlain = 0,
	Font_NormalBlack = 134,
	Font_NormalWhite = 268,
	Font_NormalRed = 402,
	Font_LargePlain = 536,
	Font_LargeBlack = 670,
	Font_LargeBrown = 804,
	Font_SmallPlain = 938,
	Font_SmallBrown = 1072,
	Font_SmallBlack = 1206
} Font;

enum ClipCode {
	ClipNone,
	ClipLeft,
	ClipRight,
	ClipTop,
	ClipBottom,
	ClipBoth,
	ClipInvisible
};

typedef struct {
	enum ClipCode clipX;
	enum ClipCode clipY;
	int clippedPixelsLeft;
	int clippedPixelsRight;
	int clippedPixelsTop;
	int clippedPixelsBottom;
	int visiblePixelsX;
	int visiblePixelsY;
	int isVisible;
} GraphicsClipInfo;


#endif
