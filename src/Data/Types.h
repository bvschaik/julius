#ifndef TYPES_H
#define TYPES_H

typedef unsigned short Color;
typedef unsigned int ScreenColor;
typedef unsigned int TimeMillis;

#define Color_Black 0
#define Color_Red 0x7c00
#define Color_Orange 0x7d61
#define Color_Yellow 0x738b
#define Color_White 0x7fff
#define Color_Transparent 0xf81f
#define Color_InsetLight 0x7fff
#define Color_InsetDark 0x4210
#define Color_Minimap_Dark 0x2108
#define Color_Minimap_Light 0x6318
#define Color_NoMask 0x7fff
#define Color_MaskRed 0x7c23 // bit lost 0xf863
#define Color_MaskGreen 0x0fe3
#define Color_Tooltip 0x2108
#define Color_Soldier 0x7800
#define Color_EnemyCentral 0x3c00
#define Color_EnemyNorthern 0x0c1f
#define Color_EnemyDesert 0x040f

typedef enum {
	Font_NormalPlain = 0,
	Font_NormalBlack = 134,
	Font_NormalWhite = 268,
	Font_NormalRed = 402,
	Font_LargePlain = 536,
	Font_LargeBlack = 670,
	Font_LargeBrown = 804,
	Font_SmallPlain = 938,
	Font_NormalGreen = 1072,
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
