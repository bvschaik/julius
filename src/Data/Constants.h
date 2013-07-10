#ifndef DATA_CONSTANTS_H
#define DATA_CONSTANTS_H

enum {
	ID_Graphic_MainMenuBackground = 14,
	ID_Graphic_PanelButton = 15,
	ID_Graphic_Font = 16,
	ID_Graphic_HouseTent = 18,
	ID_Graphic_HouseShack = 19,
	ID_Graphic_LaborPriorityLock = 94,
	ID_Graphic_PanelWindows = 95,
	ID_Graphic_AdvisorIcons = 128,
	ID_Graphic_DialogBackground = 132,
	ID_Graphic_SunkenTextboxBackground = 133,
	ID_Graphic_AdvisorBackground = 136,
	ID_Graphic_PopulationGraphBar = 157,
	ID_Graphic_WinGame = 160,
	ID_Graphic_EmpireCity = 174,
	ID_Graphic_RatingsColumn = 189,
	ID_Graphic_RatingsBackground = 195,
	ID_Graphic_BorderedButton = 208,
	ID_Graphic_GodBolt = 225,
	ID_Graphic_LoadingScreen = 251,
};

#define Color_Black 0
#define Color_Red 0xf800
#define Color_Transparent 0xf81f
#define Color_InsetLight 0xffff
#define Color_InsetDark 0x8410
#define Color_NoMask 0xffff

enum {
	EmpireObject_Ornament = 0,
	EmpireObject_City = 1,
	EmpireObject_BattleIcon = 2,
	EmpireObject_LandTradeRoute = 4,
	EmpireObject_SeaTradeRoute = 5,
	EmpireObject_RomanArmy = 6,
	EmpireObject_EnemyArmy = 7,
};

enum {
	EmpireCity_DistantRoman = 0,
	EmpireCity_Ours = 1,
	EmpireCity_Trade = 2,
	EmpireCity_FutureTrade = 3,
	EmpireCity_DistantForeign = 4,
	EmpireCity_VulnerableRoman = 5,
	EmpireCity_FutureRoman = 6,
};

#endif
