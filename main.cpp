#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "src/Zip.h"
#include "src/Loader.h"
#include "src/Language.h"
#include "src/Widget.h"
#include "src/Graphics.h"
#include "src/Graphics_Footprint.h"
#include "src/String.h"
#include "src/FileSystem.h"
#include "src/GameFile.h"

#include "src/UI/AllWindows.h"

#include "src/Data/Screen.h"
#include "src/Data/Settings.h"
#include "src/Data/Scenario.h"
#include "src/Data/Empire.h"
#include "src/Data/Graphics.h"
#include "src/Data/Mouse.h"
#include "src/Data/CityInfo.h"

Color screen[480000];

void assert(const char *msg, int expected, int actual)
{
	if (expected != actual) {
		printf("Assert failed: %s; expected: %d, actual %d\n", msg, expected, actual);
	}
}

int main(int argc, char **argv)
{
	Data_Screen.drawBuffer = screen;
	const char inputData[100] = "some fairly nice random input string to compress and then decompress";

	char compressedBuf[100];
	char decompressedBuf[100];
	int compressedLength = 100;
	int decompressedLength = 100;

	//zip_compress(const char *inputBuffer, int inputLength, char *outputBuffer, int *outputLength);
	Zip_compress(inputData, strlen(inputData)+1, compressedBuf, &compressedLength);
	Zip_decompress(compressedBuf, compressedLength, decompressedBuf, &decompressedLength);

	assert("Scenario settings", 1720, sizeof(Data_Scenario));
	assert("Empire object", 64, sizeof(struct Data_Empire_Object));
	assert("Empire object list", 12800, sizeof(Data_Empire_Objects));
	assert("Trade city", 66, sizeof(struct Data_Empire_City));
	assert("Trade city list", 2706, sizeof(Data_Empire_Cities));
	assert("City info", 2*18068, sizeof(Data_CityInfo));
	printf("sizeof(Data_Scenario_settings) = %x\n", sizeof(Data_Settings));

	Loader_Graphics_initGraphics();
	printf("Load images: %d\n", Loader_Graphics_loadMainGraphics(0));
	printf("Load model: %d\n", Loader_Model_loadC3ModelTxt());
	printf("Load language: %d\n", Language_load("c3.eng", 0));
	GameFile_loadSavedGame("1.sav");
	/*
	Widget_Panel_drawOuterPanel(2, 3, 50, 37);
	Widget_Panel_drawInnerPanel(100, 100, 10, 10);
	Widget_Panel_drawInnerPanelBottom(100, 250, 10);
	Widget_Panel_drawUnborderedPanel(200, 200, 10, 10);
	Widget_Panel_drawButtonBorder(220, 220, 160, 160, 0);
	Widget_Panel_drawSmallLabelButton(1, 300, 140, 12, 1, 2);
	*/
	//Data_Mouse.focusButtonId = 1;
	//UI_DifficultyOptions_draw();
	//UI_SpeedOptions_draw();
	//UI_DisplayOptions_draw();
	//UI_SoundOptions_draw();
	//int offset = Widget_GameText_drawNumberWithDescription(8, 18, 1, 10, 10, Font_NormalRed, 0);
	//Widget_Text_draw("test", 10 + offset, 10, Font_NormalWhite, 0);
	Widget_Panel_drawOuterPanel(0, 0, 8, 8);
	Widget_Text_drawMultiline("this is a very long body of text which should be split across multiple lines",
		10, 10, 108, Font_NormalBlack, 0);
	Widget_Text_drawNumber(100, '@', "test", 200, 200, Font_NormalWhite, 0);
	//Graphics_drawImage(GraphicId(160), 0, 0);

	Graphics_drawRect(200, 200, 191, 191, 0xf000);
	Graphics_setClipRectangle(200, 200, 190, 190);

	Graphics_Footprint_drawSize5(2826, 300, 300, 0);


	Graphics_setClipRectangle(499, 99, 52, 52);
	Graphics_drawRect(499, 99, 52, 52, 0xf000);
	Graphics_setClipRectangle(500, 100, 500, 500);
	Graphics_drawImage(7961, 540, 140);

	Graphics_setClipRectangle(0, 0, 800, 600);
	UI_MainMenu_drawBackground();
	UI_MainMenu_drawForeground();

	Graphics_saveScreenshot("test.bmp");
	printf("GraphicID: %d\n", GraphicId(136));

	return 0;
}

