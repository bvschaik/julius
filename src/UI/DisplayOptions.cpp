#include "../Widget.h"
#include "../Graphics.h"
#include "../Data/Constants.h"
#include "../Data/Graphics.h"
#include "../Data/Mouse.h"
#include "../Data/Screen.h"
#include "../Data/Settings.h"

static int focusButtonId;

void UI_DisplayOptions_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	Widget_Panel_drawOuterPanel(
		baseOffsetX + 96, baseOffsetY + 80,
		18, 12
	);
	
	Widget_Panel_drawSmallLabelButton(2,
		baseOffsetX + 128, baseOffsetY + 136,
		14, 0, focusButtonId == 1 ? 1 : 2
	);
	Widget_Panel_drawSmallLabelButton(2,
		baseOffsetX + 128, baseOffsetY + 160,
		14, 0, focusButtonId == 2 ? 1 : 2
	);
	Widget_Panel_drawSmallLabelButton(2,
		baseOffsetX + 128, baseOffsetY + 184,
		14, 0, focusButtonId == 3 ? 1 : 2
	);
	Widget_Panel_drawSmallLabelButton(2,
		baseOffsetX + 128, baseOffsetY + 208,
		14, 0, focusButtonId == 4 ? 1 : 2
	);
	Widget_Panel_drawSmallLabelButton(2,
		baseOffsetX + 128, baseOffsetY + 232,
		14, 0, focusButtonId == 5 ? 1 : 2
	);
	
	Color colorNormal = 0x02bf;
	Color colorHighlight = 0xfac1;
	Widget_GameText_drawCentered(42, 0,
		baseOffsetX + 128, baseOffsetY + 94,
		224, Font_LargeBlack, 0
	);
	
	Widget_GameText_drawCentered(42,
		Data_Settings.fullscreen ? 2 : 1,
		baseOffsetX + 128, baseOffsetY + 140,
		224, Font_SmallBrown,
		focusButtonId == 1 ? colorHighlight : colorNormal
	);
	
	Widget_GameText_drawCentered(42, 3,
		baseOffsetX + 128, baseOffsetY + 164,
		224, Font_SmallBrown,
		focusButtonId == 2 ? colorHighlight : colorNormal
	);
	Widget_GameText_drawCentered(42, 4,
		baseOffsetX + 128, baseOffsetY + 188,
		224, Font_SmallBrown,
		focusButtonId == 3 ? colorHighlight : colorNormal
	);
	Widget_GameText_drawCentered(42, 5,
		baseOffsetX + 128, baseOffsetY + 212,
		224, Font_SmallBrown,
		focusButtonId == 4 ? colorHighlight : colorNormal
	);
	Widget_GameText_drawCentered(42, 6,
		baseOffsetX + 128, baseOffsetY + 236,
		224, Font_SmallBrown,
		focusButtonId == 5 ? colorHighlight : colorNormal
	);
	
	// TODO: reset clip region() ?
}

