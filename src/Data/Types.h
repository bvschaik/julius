#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#define Color_Black 0x000000
#define Color_Red 0xff0000
#define Color_Orange 0xff5a08
#define Color_Yellow 0xe7e75a
#define Color_White 0xffffff
#define Color_Transparent 0xf700ff
#define Color_InsetLight 0xffffff
#define Color_InsetDark 0x848484
#define Color_Minimap_Dark 0x424242
#define Color_Minimap_Light 0xc6c6c6
#define Color_NoMask 0xffffff
#define Color_MaskRed 0xff0818 // bit lost 0xf863
#define Color_MaskGreen 0x18ff18
#define Color_Tooltip 0x424242
#define Color_Soldier 0xf70000
#define Color_EnemyCentral 0x7b0000
#define Color_EnemyNorthern 0x1800ff
#define Color_EnemyDesert 0x08007b

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
