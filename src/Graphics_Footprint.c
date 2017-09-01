#include "Graphics_Footprint.h"
#include "Graphics.h"
#include "Graphics_private.h"

#include "Data/Graphics.h"
#include "Data/Screen.h"
#include "Data/Constants.h"

#define FOOTPRINT_WIDTH 58
#define FOOTPRINT_HEIGHT 30
#define FOOTPRINT_DATA_LENGTH 1800

#define GETDATA(graphicId) Color *data = &Data_Graphics_PixelData.main[Data_Graphics_Main.index[graphicId].offset]
#define DATA(index) &data[900 * index]

static void drawFootprintTopFullNoMask(Color *src, int xOffset, int yOffset)
{
	ScreenColor *dst;
	dst = &ScreenPixel(xOffset + 28, yOffset + 0);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 26, yOffset + 1);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 24, yOffset + 2);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 22, yOffset + 3);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 20, yOffset + 4);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 18, yOffset + 5);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst[18] = ColorLookup[*(src++)];
	dst[19] = ColorLookup[*(src++)];
	dst[20] = ColorLookup[*(src++)];
	dst[21] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 16, yOffset + 6);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst[18] = ColorLookup[*(src++)];
	dst[19] = ColorLookup[*(src++)];
	dst[20] = ColorLookup[*(src++)];
	dst[21] = ColorLookup[*(src++)];
	dst[22] = ColorLookup[*(src++)];
	dst[23] = ColorLookup[*(src++)];
	dst[24] = ColorLookup[*(src++)];
	dst[25] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 14, yOffset + 7);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst[18] = ColorLookup[*(src++)];
	dst[19] = ColorLookup[*(src++)];
	dst[20] = ColorLookup[*(src++)];
	dst[21] = ColorLookup[*(src++)];
	dst[22] = ColorLookup[*(src++)];
	dst[23] = ColorLookup[*(src++)];
	dst[24] = ColorLookup[*(src++)];
	dst[25] = ColorLookup[*(src++)];
	dst[26] = ColorLookup[*(src++)];
	dst[27] = ColorLookup[*(src++)];
	dst[28] = ColorLookup[*(src++)];
	dst[29] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 12, yOffset + 8);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst[18] = ColorLookup[*(src++)];
	dst[19] = ColorLookup[*(src++)];
	dst[20] = ColorLookup[*(src++)];
	dst[21] = ColorLookup[*(src++)];
	dst[22] = ColorLookup[*(src++)];
	dst[23] = ColorLookup[*(src++)];
	dst[24] = ColorLookup[*(src++)];
	dst[25] = ColorLookup[*(src++)];
	dst[26] = ColorLookup[*(src++)];
	dst[27] = ColorLookup[*(src++)];
	dst[28] = ColorLookup[*(src++)];
	dst[29] = ColorLookup[*(src++)];
	dst[30] = ColorLookup[*(src++)];
	dst[31] = ColorLookup[*(src++)];
	dst[32] = ColorLookup[*(src++)];
	dst[33] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 10, yOffset + 9);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst[18] = ColorLookup[*(src++)];
	dst[19] = ColorLookup[*(src++)];
	dst[20] = ColorLookup[*(src++)];
	dst[21] = ColorLookup[*(src++)];
	dst[22] = ColorLookup[*(src++)];
	dst[23] = ColorLookup[*(src++)];
	dst[24] = ColorLookup[*(src++)];
	dst[25] = ColorLookup[*(src++)];
	dst[26] = ColorLookup[*(src++)];
	dst[27] = ColorLookup[*(src++)];
	dst[28] = ColorLookup[*(src++)];
	dst[29] = ColorLookup[*(src++)];
	dst[30] = ColorLookup[*(src++)];
	dst[31] = ColorLookup[*(src++)];
	dst[32] = ColorLookup[*(src++)];
	dst[33] = ColorLookup[*(src++)];
	dst[34] = ColorLookup[*(src++)];
	dst[35] = ColorLookup[*(src++)];
	dst[36] = ColorLookup[*(src++)];
	dst[37] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 8, yOffset + 10);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst[18] = ColorLookup[*(src++)];
	dst[19] = ColorLookup[*(src++)];
	dst[20] = ColorLookup[*(src++)];
	dst[21] = ColorLookup[*(src++)];
	dst[22] = ColorLookup[*(src++)];
	dst[23] = ColorLookup[*(src++)];
	dst[24] = ColorLookup[*(src++)];
	dst[25] = ColorLookup[*(src++)];
	dst[26] = ColorLookup[*(src++)];
	dst[27] = ColorLookup[*(src++)];
	dst[28] = ColorLookup[*(src++)];
	dst[29] = ColorLookup[*(src++)];
	dst[30] = ColorLookup[*(src++)];
	dst[31] = ColorLookup[*(src++)];
	dst[32] = ColorLookup[*(src++)];
	dst[33] = ColorLookup[*(src++)];
	dst[34] = ColorLookup[*(src++)];
	dst[35] = ColorLookup[*(src++)];
	dst[36] = ColorLookup[*(src++)];
	dst[37] = ColorLookup[*(src++)];
	dst[38] = ColorLookup[*(src++)];
	dst[39] = ColorLookup[*(src++)];
	dst[40] = ColorLookup[*(src++)];
	dst[41] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 6, yOffset + 11);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst[18] = ColorLookup[*(src++)];
	dst[19] = ColorLookup[*(src++)];
	dst[20] = ColorLookup[*(src++)];
	dst[21] = ColorLookup[*(src++)];
	dst[22] = ColorLookup[*(src++)];
	dst[23] = ColorLookup[*(src++)];
	dst[24] = ColorLookup[*(src++)];
	dst[25] = ColorLookup[*(src++)];
	dst[26] = ColorLookup[*(src++)];
	dst[27] = ColorLookup[*(src++)];
	dst[28] = ColorLookup[*(src++)];
	dst[29] = ColorLookup[*(src++)];
	dst[30] = ColorLookup[*(src++)];
	dst[31] = ColorLookup[*(src++)];
	dst[32] = ColorLookup[*(src++)];
	dst[33] = ColorLookup[*(src++)];
	dst[34] = ColorLookup[*(src++)];
	dst[35] = ColorLookup[*(src++)];
	dst[36] = ColorLookup[*(src++)];
	dst[37] = ColorLookup[*(src++)];
	dst[38] = ColorLookup[*(src++)];
	dst[39] = ColorLookup[*(src++)];
	dst[40] = ColorLookup[*(src++)];
	dst[41] = ColorLookup[*(src++)];
	dst[42] = ColorLookup[*(src++)];
	dst[43] = ColorLookup[*(src++)];
	dst[44] = ColorLookup[*(src++)];
	dst[45] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 4, yOffset + 12);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst[18] = ColorLookup[*(src++)];
	dst[19] = ColorLookup[*(src++)];
	dst[20] = ColorLookup[*(src++)];
	dst[21] = ColorLookup[*(src++)];
	dst[22] = ColorLookup[*(src++)];
	dst[23] = ColorLookup[*(src++)];
	dst[24] = ColorLookup[*(src++)];
	dst[25] = ColorLookup[*(src++)];
	dst[26] = ColorLookup[*(src++)];
	dst[27] = ColorLookup[*(src++)];
	dst[28] = ColorLookup[*(src++)];
	dst[29] = ColorLookup[*(src++)];
	dst[30] = ColorLookup[*(src++)];
	dst[31] = ColorLookup[*(src++)];
	dst[32] = ColorLookup[*(src++)];
	dst[33] = ColorLookup[*(src++)];
	dst[34] = ColorLookup[*(src++)];
	dst[35] = ColorLookup[*(src++)];
	dst[36] = ColorLookup[*(src++)];
	dst[37] = ColorLookup[*(src++)];
	dst[38] = ColorLookup[*(src++)];
	dst[39] = ColorLookup[*(src++)];
	dst[40] = ColorLookup[*(src++)];
	dst[41] = ColorLookup[*(src++)];
	dst[42] = ColorLookup[*(src++)];
	dst[43] = ColorLookup[*(src++)];
	dst[44] = ColorLookup[*(src++)];
	dst[45] = ColorLookup[*(src++)];
	dst[46] = ColorLookup[*(src++)];
	dst[47] = ColorLookup[*(src++)];
	dst[48] = ColorLookup[*(src++)];
	dst[49] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 2, yOffset + 13);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst[18] = ColorLookup[*(src++)];
	dst[19] = ColorLookup[*(src++)];
	dst[20] = ColorLookup[*(src++)];
	dst[21] = ColorLookup[*(src++)];
	dst[22] = ColorLookup[*(src++)];
	dst[23] = ColorLookup[*(src++)];
	dst[24] = ColorLookup[*(src++)];
	dst[25] = ColorLookup[*(src++)];
	dst[26] = ColorLookup[*(src++)];
	dst[27] = ColorLookup[*(src++)];
	dst[28] = ColorLookup[*(src++)];
	dst[29] = ColorLookup[*(src++)];
	dst[30] = ColorLookup[*(src++)];
	dst[31] = ColorLookup[*(src++)];
	dst[32] = ColorLookup[*(src++)];
	dst[33] = ColorLookup[*(src++)];
	dst[34] = ColorLookup[*(src++)];
	dst[35] = ColorLookup[*(src++)];
	dst[36] = ColorLookup[*(src++)];
	dst[37] = ColorLookup[*(src++)];
	dst[38] = ColorLookup[*(src++)];
	dst[39] = ColorLookup[*(src++)];
	dst[40] = ColorLookup[*(src++)];
	dst[41] = ColorLookup[*(src++)];
	dst[42] = ColorLookup[*(src++)];
	dst[43] = ColorLookup[*(src++)];
	dst[44] = ColorLookup[*(src++)];
	dst[45] = ColorLookup[*(src++)];
	dst[46] = ColorLookup[*(src++)];
	dst[47] = ColorLookup[*(src++)];
	dst[48] = ColorLookup[*(src++)];
	dst[49] = ColorLookup[*(src++)];
	dst[50] = ColorLookup[*(src++)];
	dst[51] = ColorLookup[*(src++)];
	dst[52] = ColorLookup[*(src++)];
	dst[53] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 0, yOffset + 14);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst[18] = ColorLookup[*(src++)];
	dst[19] = ColorLookup[*(src++)];
	dst[20] = ColorLookup[*(src++)];
	dst[21] = ColorLookup[*(src++)];
	dst[22] = ColorLookup[*(src++)];
	dst[23] = ColorLookup[*(src++)];
	dst[24] = ColorLookup[*(src++)];
	dst[25] = ColorLookup[*(src++)];
	dst[26] = ColorLookup[*(src++)];
	dst[27] = ColorLookup[*(src++)];
	dst[28] = ColorLookup[*(src++)];
	dst[29] = ColorLookup[*(src++)];
	dst[30] = ColorLookup[*(src++)];
	dst[31] = ColorLookup[*(src++)];
	dst[32] = ColorLookup[*(src++)];
	dst[33] = ColorLookup[*(src++)];
	dst[34] = ColorLookup[*(src++)];
	dst[35] = ColorLookup[*(src++)];
	dst[36] = ColorLookup[*(src++)];
	dst[37] = ColorLookup[*(src++)];
	dst[38] = ColorLookup[*(src++)];
	dst[39] = ColorLookup[*(src++)];
	dst[40] = ColorLookup[*(src++)];
	dst[41] = ColorLookup[*(src++)];
	dst[42] = ColorLookup[*(src++)];
	dst[43] = ColorLookup[*(src++)];
	dst[44] = ColorLookup[*(src++)];
	dst[45] = ColorLookup[*(src++)];
	dst[46] = ColorLookup[*(src++)];
	dst[47] = ColorLookup[*(src++)];
	dst[48] = ColorLookup[*(src++)];
	dst[49] = ColorLookup[*(src++)];
	dst[50] = ColorLookup[*(src++)];
	dst[51] = ColorLookup[*(src++)];
	dst[52] = ColorLookup[*(src++)];
	dst[53] = ColorLookup[*(src++)];
	dst[54] = ColorLookup[*(src++)];
	dst[55] = ColorLookup[*(src++)];
	dst[56] = ColorLookup[*(src++)];
	dst[57] = ColorLookup[*(src++)];
	// bottom part
	dst = &ScreenPixel(xOffset + 0, yOffset + 15);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst[18] = ColorLookup[*(src++)];
	dst[19] = ColorLookup[*(src++)];
	dst[20] = ColorLookup[*(src++)];
	dst[21] = ColorLookup[*(src++)];
	dst[22] = ColorLookup[*(src++)];
	dst[23] = ColorLookup[*(src++)];
	dst[24] = ColorLookup[*(src++)];
	dst[25] = ColorLookup[*(src++)];
	dst[26] = ColorLookup[*(src++)];
	dst[27] = ColorLookup[*(src++)];
	dst[28] = ColorLookup[*(src++)];
	dst[29] = ColorLookup[*(src++)];
	dst[30] = ColorLookup[*(src++)];
	dst[31] = ColorLookup[*(src++)];
	dst[32] = ColorLookup[*(src++)];
	dst[33] = ColorLookup[*(src++)];
	dst[34] = ColorLookup[*(src++)];
	dst[35] = ColorLookup[*(src++)];
	dst[36] = ColorLookup[*(src++)];
	dst[37] = ColorLookup[*(src++)];
	dst[38] = ColorLookup[*(src++)];
	dst[39] = ColorLookup[*(src++)];
	dst[40] = ColorLookup[*(src++)];
	dst[41] = ColorLookup[*(src++)];
	dst[42] = ColorLookup[*(src++)];
	dst[43] = ColorLookup[*(src++)];
	dst[44] = ColorLookup[*(src++)];
	dst[45] = ColorLookup[*(src++)];
	dst[46] = ColorLookup[*(src++)];
	dst[47] = ColorLookup[*(src++)];
	dst[48] = ColorLookup[*(src++)];
	dst[49] = ColorLookup[*(src++)];
	dst[50] = ColorLookup[*(src++)];
	dst[51] = ColorLookup[*(src++)];
	dst[52] = ColorLookup[*(src++)];
	dst[53] = ColorLookup[*(src++)];
	dst[54] = ColorLookup[*(src++)];
	dst[55] = ColorLookup[*(src++)];
	dst[56] = ColorLookup[*(src++)];
	dst[57] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 2, yOffset + 16);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst[18] = ColorLookup[*(src++)];
	dst[19] = ColorLookup[*(src++)];
	dst[20] = ColorLookup[*(src++)];
	dst[21] = ColorLookup[*(src++)];
	dst[22] = ColorLookup[*(src++)];
	dst[23] = ColorLookup[*(src++)];
	dst[24] = ColorLookup[*(src++)];
	dst[25] = ColorLookup[*(src++)];
	dst[26] = ColorLookup[*(src++)];
	dst[27] = ColorLookup[*(src++)];
	dst[28] = ColorLookup[*(src++)];
	dst[29] = ColorLookup[*(src++)];
	dst[30] = ColorLookup[*(src++)];
	dst[31] = ColorLookup[*(src++)];
	dst[32] = ColorLookup[*(src++)];
	dst[33] = ColorLookup[*(src++)];
	dst[34] = ColorLookup[*(src++)];
	dst[35] = ColorLookup[*(src++)];
	dst[36] = ColorLookup[*(src++)];
	dst[37] = ColorLookup[*(src++)];
	dst[38] = ColorLookup[*(src++)];
	dst[39] = ColorLookup[*(src++)];
	dst[40] = ColorLookup[*(src++)];
	dst[41] = ColorLookup[*(src++)];
	dst[42] = ColorLookup[*(src++)];
	dst[43] = ColorLookup[*(src++)];
	dst[44] = ColorLookup[*(src++)];
	dst[45] = ColorLookup[*(src++)];
	dst[46] = ColorLookup[*(src++)];
	dst[47] = ColorLookup[*(src++)];
	dst[48] = ColorLookup[*(src++)];
	dst[49] = ColorLookup[*(src++)];
	dst[50] = ColorLookup[*(src++)];
	dst[51] = ColorLookup[*(src++)];
	dst[52] = ColorLookup[*(src++)];
	dst[53] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 4, yOffset + 17);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst[18] = ColorLookup[*(src++)];
	dst[19] = ColorLookup[*(src++)];
	dst[20] = ColorLookup[*(src++)];
	dst[21] = ColorLookup[*(src++)];
	dst[22] = ColorLookup[*(src++)];
	dst[23] = ColorLookup[*(src++)];
	dst[24] = ColorLookup[*(src++)];
	dst[25] = ColorLookup[*(src++)];
	dst[26] = ColorLookup[*(src++)];
	dst[27] = ColorLookup[*(src++)];
	dst[28] = ColorLookup[*(src++)];
	dst[29] = ColorLookup[*(src++)];
	dst[30] = ColorLookup[*(src++)];
	dst[31] = ColorLookup[*(src++)];
	dst[32] = ColorLookup[*(src++)];
	dst[33] = ColorLookup[*(src++)];
	dst[34] = ColorLookup[*(src++)];
	dst[35] = ColorLookup[*(src++)];
	dst[36] = ColorLookup[*(src++)];
	dst[37] = ColorLookup[*(src++)];
	dst[38] = ColorLookup[*(src++)];
	dst[39] = ColorLookup[*(src++)];
	dst[40] = ColorLookup[*(src++)];
	dst[41] = ColorLookup[*(src++)];
	dst[42] = ColorLookup[*(src++)];
	dst[43] = ColorLookup[*(src++)];
	dst[44] = ColorLookup[*(src++)];
	dst[45] = ColorLookup[*(src++)];
	dst[46] = ColorLookup[*(src++)];
	dst[47] = ColorLookup[*(src++)];
	dst[48] = ColorLookup[*(src++)];
	dst[49] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 6, yOffset + 18);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst[18] = ColorLookup[*(src++)];
	dst[19] = ColorLookup[*(src++)];
	dst[20] = ColorLookup[*(src++)];
	dst[21] = ColorLookup[*(src++)];
	dst[22] = ColorLookup[*(src++)];
	dst[23] = ColorLookup[*(src++)];
	dst[24] = ColorLookup[*(src++)];
	dst[25] = ColorLookup[*(src++)];
	dst[26] = ColorLookup[*(src++)];
	dst[27] = ColorLookup[*(src++)];
	dst[28] = ColorLookup[*(src++)];
	dst[29] = ColorLookup[*(src++)];
	dst[30] = ColorLookup[*(src++)];
	dst[31] = ColorLookup[*(src++)];
	dst[32] = ColorLookup[*(src++)];
	dst[33] = ColorLookup[*(src++)];
	dst[34] = ColorLookup[*(src++)];
	dst[35] = ColorLookup[*(src++)];
	dst[36] = ColorLookup[*(src++)];
	dst[37] = ColorLookup[*(src++)];
	dst[38] = ColorLookup[*(src++)];
	dst[39] = ColorLookup[*(src++)];
	dst[40] = ColorLookup[*(src++)];
	dst[41] = ColorLookup[*(src++)];
	dst[42] = ColorLookup[*(src++)];
	dst[43] = ColorLookup[*(src++)];
	dst[44] = ColorLookup[*(src++)];
	dst[45] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 8, yOffset + 19);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst[18] = ColorLookup[*(src++)];
	dst[19] = ColorLookup[*(src++)];
	dst[20] = ColorLookup[*(src++)];
	dst[21] = ColorLookup[*(src++)];
	dst[22] = ColorLookup[*(src++)];
	dst[23] = ColorLookup[*(src++)];
	dst[24] = ColorLookup[*(src++)];
	dst[25] = ColorLookup[*(src++)];
	dst[26] = ColorLookup[*(src++)];
	dst[27] = ColorLookup[*(src++)];
	dst[28] = ColorLookup[*(src++)];
	dst[29] = ColorLookup[*(src++)];
	dst[30] = ColorLookup[*(src++)];
	dst[31] = ColorLookup[*(src++)];
	dst[32] = ColorLookup[*(src++)];
	dst[33] = ColorLookup[*(src++)];
	dst[34] = ColorLookup[*(src++)];
	dst[35] = ColorLookup[*(src++)];
	dst[36] = ColorLookup[*(src++)];
	dst[37] = ColorLookup[*(src++)];
	dst[38] = ColorLookup[*(src++)];
	dst[39] = ColorLookup[*(src++)];
	dst[40] = ColorLookup[*(src++)];
	dst[41] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 10, yOffset + 20);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst[18] = ColorLookup[*(src++)];
	dst[19] = ColorLookup[*(src++)];
	dst[20] = ColorLookup[*(src++)];
	dst[21] = ColorLookup[*(src++)];
	dst[22] = ColorLookup[*(src++)];
	dst[23] = ColorLookup[*(src++)];
	dst[24] = ColorLookup[*(src++)];
	dst[25] = ColorLookup[*(src++)];
	dst[26] = ColorLookup[*(src++)];
	dst[27] = ColorLookup[*(src++)];
	dst[28] = ColorLookup[*(src++)];
	dst[29] = ColorLookup[*(src++)];
	dst[30] = ColorLookup[*(src++)];
	dst[31] = ColorLookup[*(src++)];
	dst[32] = ColorLookup[*(src++)];
	dst[33] = ColorLookup[*(src++)];
	dst[34] = ColorLookup[*(src++)];
	dst[35] = ColorLookup[*(src++)];
	dst[36] = ColorLookup[*(src++)];
	dst[37] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 12, yOffset + 21);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst[18] = ColorLookup[*(src++)];
	dst[19] = ColorLookup[*(src++)];
	dst[20] = ColorLookup[*(src++)];
	dst[21] = ColorLookup[*(src++)];
	dst[22] = ColorLookup[*(src++)];
	dst[23] = ColorLookup[*(src++)];
	dst[24] = ColorLookup[*(src++)];
	dst[25] = ColorLookup[*(src++)];
	dst[26] = ColorLookup[*(src++)];
	dst[27] = ColorLookup[*(src++)];
	dst[28] = ColorLookup[*(src++)];
	dst[29] = ColorLookup[*(src++)];
	dst[30] = ColorLookup[*(src++)];
	dst[31] = ColorLookup[*(src++)];
	dst[32] = ColorLookup[*(src++)];
	dst[33] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 14, yOffset + 22);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst[18] = ColorLookup[*(src++)];
	dst[19] = ColorLookup[*(src++)];
	dst[20] = ColorLookup[*(src++)];
	dst[21] = ColorLookup[*(src++)];
	dst[22] = ColorLookup[*(src++)];
	dst[23] = ColorLookup[*(src++)];
	dst[24] = ColorLookup[*(src++)];
	dst[25] = ColorLookup[*(src++)];
	dst[26] = ColorLookup[*(src++)];
	dst[27] = ColorLookup[*(src++)];
	dst[28] = ColorLookup[*(src++)];
	dst[29] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 16, yOffset + 23);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst[18] = ColorLookup[*(src++)];
	dst[19] = ColorLookup[*(src++)];
	dst[20] = ColorLookup[*(src++)];
	dst[21] = ColorLookup[*(src++)];
	dst[22] = ColorLookup[*(src++)];
	dst[23] = ColorLookup[*(src++)];
	dst[24] = ColorLookup[*(src++)];
	dst[25] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 18, yOffset + 24);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst[18] = ColorLookup[*(src++)];
	dst[19] = ColorLookup[*(src++)];
	dst[20] = ColorLookup[*(src++)];
	dst[21] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 20, yOffset + 25);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst[14] = ColorLookup[*(src++)];
	dst[15] = ColorLookup[*(src++)];
	dst[16] = ColorLookup[*(src++)];
	dst[17] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 22, yOffset + 26);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst[10] = ColorLookup[*(src++)];
	dst[11] = ColorLookup[*(src++)];
	dst[12] = ColorLookup[*(src++)];
	dst[13] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 24, yOffset + 27);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst[6] = ColorLookup[*(src++)];
	dst[7] = ColorLookup[*(src++)];
	dst[8] = ColorLookup[*(src++)];
	dst[9] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 26, yOffset + 28);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
	dst[2] = ColorLookup[*(src++)];
	dst[3] = ColorLookup[*(src++)];
	dst[4] = ColorLookup[*(src++)];
	dst[5] = ColorLookup[*(src++)];
	dst = &ScreenPixel(xOffset + 28, yOffset + 29);
	dst[0] = ColorLookup[*(src++)];
	dst[1] = ColorLookup[*(src++)];
}

static void drawFootprintTile(Color *data, int xOffset, int yOffset, Color colorMask)
{
	if (!colorMask) {
		colorMask = Color_NoMask;
	}
	GraphicsClipInfo *clip = Graphics_getClipInfo(xOffset, yOffset, 58, 30);
	if (!clip->isVisible) {
		return;
	}
	// footprints are ALWAYS clipped in half, if they are clipped
	if (clip->clipY == ClipNone && clip->clipX == ClipNone && colorMask == Color_NoMask) {
		drawFootprintTopFullNoMask(data, xOffset, yOffset);
		return;
	}
	int clipLeft = clip->clipX == ClipLeft;
	int clipRight = clip->clipX == ClipRight;
	if (clip->clipY != ClipTop) {
		Color *src = data;
		for (int y = 0; y < 15; y++) {
			int xMax = 4 * y + 2;
			int xStart = 29 - 1 - 2 * y;
			if (clipLeft || clipRight) {
				xMax = 2 * y;
			}
			if (clipLeft) {
				xStart = 30;
				src += xMax + 2;
			}
			ScreenColor *buffer = &ScreenPixel(xOffset + xStart, yOffset + y);
			if (colorMask == Color_NoMask) {
				for (int x = 0; x < xMax; x++, buffer++, src++) {
					*buffer = ColorLookup[*src];
				}
			} else {
				for (int x = 0; x < xMax; x++, buffer++, src++) {
					*buffer = ColorLookup[*src & colorMask];
				}
			}
			if (clipRight) {
				src += xMax + 2;
			}
		}
	}
	if (clip->clipY != ClipBottom) {
		Color *src = &data[900 / 2];
		for (int y = 0; y < 15; y++) {
			int xMax = 4 * (15 - 1 - y) + 2;
			int xStart = 2 * y;
			if (clipLeft || clipRight) {
				xMax = xMax / 2 - 1;
			}
			if (clipLeft) {
				xStart = 30;
				src += xMax + 2;
			}
			ScreenColor *buffer = &ScreenPixel(xOffset + xStart, 15 + yOffset + y);
			if (colorMask == Color_NoMask) {
				for (int x = 0; x < xMax; x++, buffer++, src++) {
					*buffer = ColorLookup[*src];
				}
			} else {
				for (int x = 0; x < xMax; x++, buffer++, src++) {
					*buffer = ColorLookup[*src & colorMask];
				}
			}
			if (clipRight) {
				src += xMax + 2;
			}
		}
	}
}

int Graphics_Footprint_drawSize1(int graphicId, int xOffset, int yOffset, Color colorMask)
{
	GETDATA(graphicId);

	drawFootprintTile(DATA(0), xOffset, yOffset, colorMask);

	return FOOTPRINT_WIDTH;
}

int Graphics_Footprint_drawSize2(int graphicId, int xOffset, int yOffset, Color colorMask)
{
	GETDATA(graphicId);
	
	int index = 0;
	drawFootprintTile(DATA(index++), xOffset, yOffset, colorMask);
	
	drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 15, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 15, colorMask);
	
	drawFootprintTile(DATA(index++), xOffset, yOffset + 30, colorMask);
	
	return FOOTPRINT_WIDTH;
}

int Graphics_Footprint_drawSize3(int graphicId, int xOffset, int yOffset, Color colorMask)
{
	GETDATA(graphicId);
	
	int index = 0;
	drawFootprintTile(DATA(index++), xOffset, yOffset, colorMask);

	drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 15, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 15, colorMask);
	
	drawFootprintTile(DATA(index++), xOffset - 60, yOffset + 30, colorMask);
	drawFootprintTile(DATA(index++), xOffset, yOffset + 30, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 60, yOffset + 30, colorMask);
	
	drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 45, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 45, colorMask);
	
	drawFootprintTile(DATA(index++), xOffset, yOffset + 60, colorMask);

	return FOOTPRINT_WIDTH;
}

int Graphics_Footprint_drawSize4(int graphicId, int xOffset, int yOffset, Color colorMask)
{
	GETDATA(graphicId);

	int index = 0;
	drawFootprintTile(DATA(index++), xOffset, yOffset, colorMask);

	drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 15, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 15, colorMask);

	drawFootprintTile(DATA(index++), xOffset - 60, yOffset + 30, colorMask);
	drawFootprintTile(DATA(index++), xOffset, yOffset + 30, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 60, yOffset + 30, colorMask);

	drawFootprintTile(DATA(index++), xOffset - 90, yOffset + 45, colorMask);
	drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 45, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 45, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 90, yOffset + 45, colorMask);

	drawFootprintTile(DATA(index++), xOffset - 60, yOffset + 60, colorMask);
	drawFootprintTile(DATA(index++), xOffset, yOffset + 60, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 60, yOffset + 60, colorMask);
	
	drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 75, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 75, colorMask);

	drawFootprintTile(DATA(index++), xOffset, yOffset + 90, colorMask);
	
	return FOOTPRINT_WIDTH;
}

int Graphics_Footprint_drawSize5(int graphicId, int xOffset, int yOffset, Color colorMask)
{
	GETDATA(graphicId);

	int index = 0;
	drawFootprintTile(DATA(index++), xOffset, yOffset, colorMask);

	drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 15, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 15, colorMask);

	drawFootprintTile(DATA(index++), xOffset - 60, yOffset + 30, colorMask);
	drawFootprintTile(DATA(index++), xOffset, yOffset + 30, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 60, yOffset + 30, colorMask);

	drawFootprintTile(DATA(index++), xOffset - 90, yOffset + 45, colorMask);
	drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 45, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 45, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 90, yOffset + 45, colorMask);

	drawFootprintTile(DATA(index++), xOffset - 120, yOffset + 60, colorMask);
	drawFootprintTile(DATA(index++), xOffset - 60, yOffset + 60, colorMask);
	drawFootprintTile(DATA(index++), xOffset, yOffset + 60, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 60, yOffset + 60, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 120, yOffset + 60, colorMask);

	drawFootprintTile(DATA(index++), xOffset - 90, yOffset + 75, colorMask);
	drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 75, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 75, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 90, yOffset + 75, colorMask);
	
	drawFootprintTile(DATA(index++), xOffset - 60, yOffset + 90, colorMask);
	drawFootprintTile(DATA(index++), xOffset, yOffset + 90, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 60, yOffset + 90, colorMask);

	drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 105, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 105, colorMask);

	drawFootprintTile(DATA(index++), xOffset, yOffset + 120, colorMask);
	
	return FOOTPRINT_WIDTH;
}
