#include "AllData.h"

struct Data_Model_House Data_Model_Houses[20];
struct Data_Model_Building Data_Model_Buildings[130];
struct Data_Model_Difficulty Data_Model_Difficulty = {
	{300, 200, 150, 100, 75}, // money percentage
	{40, 60, 80, 100, 120}, // enemy percentage
	{70, 60, 50, 50, 40}, // starting favor
	{80, 70, 60, 50, 40}, // sentiment
};

struct Data_Scenario Data_Scenario;

struct Data_Event Data_Event = {0};

struct _Data_Settings Data_Settings;
struct _Data_Settings_Map Data_Settings_Map;

struct _Data_Tutorial Data_Tutorial;

struct _Data_Message Data_Message;

struct _Data_Random Data_Random;

struct _Data_Empire Data_Empire;
struct _Data_Empire_Sizes Data_Empire_Sizes = {2000, 1000, 16, 16, 120};
struct Data_Empire_Index Data_Empire_Index[40];
struct Data_Empire_Object Data_Empire_Objects[MAX_EMPIRE_OBJECTS];
struct Data_Empire_City Data_Empire_Cities[MAX_EMPIRE_CITIES];
struct _Data_Empire_Trade Data_Empire_Trade;

struct _Data_Graphics_Main Data_Graphics_Main;
struct _Data_Graphics_Enemy Data_Graphics_Enemy;
struct _Data_Graphics_PixelData Data_Graphics_PixelData;

struct Data_Screen Data_Screen;// = { 800, 600, {80, 60} };

struct Data_Mouse Data_Mouse;

struct Data_Language_Text Data_Language_Text;
struct Data_Language_Message Data_Language_Message;

struct _Data_CityInfo Data_CityInfo;
struct _Data_CityInfo_Buildings Data_CityInfo_Buildings;
struct _Data_CityInfo_CultureCoverage Data_CityInfo_CultureCoverage;
struct _Data_CityInfo_Extra Data_CityInfo_Extra;
struct _Data_CityInfo_Resource Data_CityInfo_Resource;

struct Data_KeyboardInput Data_KeyboardInput;

struct _Data_State Data_State;

struct Data_Walker_NameSequence Data_Walker_NameSequence;
struct Data_Walker Data_Walkers[MAX_WALKERS];
struct Data_Walker_Extra Data_Walker_Extra;
struct Data_Walker_Trader Data_Walker_Traders[MAX_TRADERS];

struct Data_Building Data_Buildings[MAX_BUILDINGS];
struct Data_Building_Storage Data_Building_Storages[MAX_STORAGES];
struct Data_Buildings_Extra Data_Buildings_Extra;
struct Data_BuildingList Data_BuildingList;

struct Data_Sound_City Data_Sound_City[70];

struct Data_CityView Data_CityView;

struct Data_InvasionWarning Data_InvasionWarnings[MAX_INVASION_WARNINGS];

struct Data_TradePrice Data_TradePrices[16];

struct Data_Formation Data_Formations[MAX_FORMATIONS];
struct _Data_Formation_Extra Data_Formation_Extra;
struct _Data_Formation_Invasion Data_Formation_Invasion;

struct _Data_FileList Data_FileList;

struct _Data_Routes Data_Routes;

struct _Data_Debug Data_Debug;

const int Constant_SalaryForRank[11] = {0, 2, 5, 8, 12, 20, 30, 40, 60, 80, 100};
const int Constant_DirectionGridOffsets[8] = {-162, -161, 1, 163, 162, 161, -1, -163};

const struct BuildingProperties Constant_BuildingProperties[140] = {
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{1, 0, 0x70, 0},
	{1, 0, 0x18, 0x1A},
	{1, 0, 0, 0},
	{1, 0, 0x13, 2},
	{0, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{2, 0, 0, 0},
	{2, 0, 0, 0},
	{2, 0, 0, 0},
	{2, 0, 0, 0},
	{3, 0, 0, 0},
	{3, 0, 0, 0},
	{3, 0, 0, 0},
	{3, 0, 0, 0},
	{4, 0, 0, 0},
	{4, 0, 0, 0},
	{3, 0, 0x2D, 0},
	{2, 0, 0x2E, 0},
	{5, 0, 0xD5, 0},
	{5, 0, 0x30, 0},
	{3, 0, 0x31, 0},
	{3, 0, 0x32, 0},
	{3, 0, 0x33, 0},
	{3, 0, 0x34, 0},
	{1, 1, 0x3A, 0},
	{1, 1, 0x3B, 0},
	{3, 1, 0x42, 0},
	{1, 1, 0x3D, 0},
	{2, 1, 0x3D, 1},
	{3, 1, 0x3D, 2},
	{3, 1, 0x42, 0},
	{3, 1, 0x42, 0},
	{1, 0, 0x44, 0},
	{3, 0, 0x46, 0},
	{2, 0, 0x0B9, 0},
	{1, 0, 0x43, 0},
	{3, 0, 0x42, 0},
	{2, 0, 0x29, 0},
	{3, 0, 0x2B, 0},
	{2, 0, 0x2A, 0},
	{4, 1, 0x42, 1},
	{1, 0, 0x40, 0},
	{3, 1, 0x0CD, 0},
	{3, 1, 0x42, 0},
	{2, 1, 0x11, 1},
	{2, 1, 0x11, 0},
	{2, 0, 0x47, 0},
	{2, 0, 0x48, 0},
	{2, 0, 0x49, 0},
	{2, 0, 0x4A, 0},
	{2, 0, 0x4B, 0},
	{3, 0, 0x47, 1},
	{3, 0, 0x48, 1},
	{3, 0, 0x49, 1},
	{3, 0, 0x4A, 1},
	{3, 0, 0x4B, 1},
	{2, 0, 0x16, 0},
	{3, 0, 0x63, 0},
	{1, 1, 0x52, 0},
	{1, 1, 0x52, 0},
	{2, 0, 0x4D, 0},
	{3, 0, 0x4E, 0},
	{2, 0, 0x4F, 0},
	{3, 0, 0x55, 0},
	{4, 0, 0x56, 0},
	{5, 0, 0x57, 0},
	{2, 1, 0x0B8, 0},
	{1, 1, 0x51, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{0, 0, 0, 0},
	{5, 0, 0x3E, 0},
	{2, 0, 0x3F, 0},
	{0, 0, 0, 0},
	{1, 1, 0x0B7, 0},
	{2, 1, 0x0B7, 2},
	{3, 1, 0x19, 0},
	{1, 1, 0x36, 0},
	{1, 1, 0x17, 0},
	{1, 1, 0x64, 0},
	{3, 0, 0x0C9, 0},
	{3, 0, 0x0A6, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{2, 0, 0x4C, 0},
	{1, 1, 0, 0},
	{3, 0, 0x25, 0},
	{3, 0, 0x25, 0},
	{3, 0, 0x25, 0},
	{3, 0, 0x25, 0},
	{3, 0, 0x25, 0},
	{3, 0, 0x25, 0},
	{2, 0, 0x26, 0},
	{2, 0, 0x27, 0},
	{2, 0, 0x41, 0},
	{2, 0, 0x28, 0},
	{2, 0, 0x2C, 0},
	{2, 0, 0x7A, 0},
	{2, 0, 0x7B, 0},
	{2, 0, 0x7C, 0},
	{2, 0, 0x7D, 0},
	{0, 0, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{2, 1, 0x0D8, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

const struct WalkerProperties Constant_WalkerProperties[80] = {
	{0,   0,   0,   0,   0,   0,   0,   0},
	{1,  20,   0,   0,   0,   0,   0,   0},
	{1,  20,   0,   0,   0,   0,   0,   0},
	{1,  20,   0,   0,   0,   0,   0,   0},
	{1,  20,   0,   0,   0,   0,   0,   0},
	{0,   0,   0,   0,   0,   0,   0,   0},
	{0,   0,   0,   0,   0,   0,   0,   0},
	{1,  20,   0,   0,   0,   0,   0,   0},
	{1,  20,   0,   0,   0,   0,   0,   0},
	{1,  20,   0,   0,   0,   0,   0,   0},
	{2,  50,   5,   0,   0,   0,   0,   0},
	{2,  80,   4,   0,   0,   4,  10, 100},
	{2, 120,   8,   0,   0,   0,   0,   0},
	{2, 150,  10,   0,   0,   0,   0,   0},
	{0,   0,   0,   0,   0,   0,   0,   0},
	{1,  20,   0,   0,   0,   0,   0,   0},
	{2, 100,   9,   2,   0,   0,   0,   0},
	{2, 100,  15,   0,   0,   0,   0,   0},
	{1,  20,   0,   0,   0,   0,   0,   0},
	{1,  10,   0,   0,   0,   0,   0,   0},
	{0,   0,   0,   0,   0,   0,   0,   0},
	{1,  10,   0,   0,   0,   0,   0,   0},
	{4,  12,   0,   0,   0,   0,   0,   0},
	{4,  12,   0,   0,   0,   0,   0,   0},
	{4,  12,   0,   0,   0,   0,   0,   0},
	{0,   0,   0,   0,   0,   0,   0,   0},
	{1,  20,   0,   0,   0,   0,   0,   0},
	{1,  20,   0,   0,   0,   0,   0,   0},
	{1,  10,   0,   0,   0,   0,   0,   0},
	{1,  20,   0,   0,   0,   0,   0,   0},
	{1,  20,   0,   0,   0,   0,   0,   0},
	{1,  20,   0,   0,   0,   0,   0,   0},
	{1,  20,   0,   0,   0,   0,   0,   0},
	{1,  20,   0,   0,   0,   0,   0,   0},
	{1,  20,   0,   0,   0,   0,   0,   0},
	{1,  20,   0,   0,   0,   0,   0,   0},
	{0,   0,   0,   0,   0,   0,   0,   0},
	{0,   0,   0,   0,   0,   0,   0,   0},
	{1,  20,   0,   0,   0,   0,   0,   0},
	{1,  20,   0,   0,   0,   0,   0,   0},
	{1,  10,   0,   0,   0,   0,   0,   0},
	{5,  40,   6,   0,   0,   0,   0,   0},
	{2,  50,   6,   0,   0,   6,  20,  50},
	{3,  70,   5,   0,   0,   4,  10,  70},
	{3,  90,   7,   0,   0,   0,   0,   0},
	{3, 120,  12,   2,   2,   0,   0,   0},
	{3, 120,   7,   1,   0,   5,  10,  70},
	{3, 200,  20,   5,   8,   6,  10,  70},
	{3, 120,  15,   4,   4,   0,   0,   0},
	{3,  90,   7,   1,   0,   0,   0,   0},
	{3, 110,  10,   1,   2,   0,   0,   0},
	{3,  70,   5,   0,   0,   3,  10, 100},
	{3, 100,   6,   1,   0,   4,  10,  70},
	{3, 120,  15,   2,   3,   0,   0,   0},
	{3, 100,   9,   2,   0,   0,   0,   0},
	{3,  90,   4,   0,   0,   4,  10, 100},
	{3, 100,   8,   0,   0,   0,   0,   0},
	{3, 150,  13,   2,   0,   0,   0,   0},
	{5,  40,   0,   0,   0,   0,   0,   0},
	{0, 100,   0,   0,   0,  12,   0,   0},
	{0, 100,   0,   0,   0,  20,   0,   0},
	{0, 100,   0,   0,   0, 200,   0,   0},
	{0, 100,   0,   0,   0,   0,  15, 200},
	{0, 100,   0,   0,   0,   0,   0,   0},
	{0, 100,   0,   0,   0,   0,   0,   0},
	{0, 100,   0,   0,   0,   0,   0,   0},
	{0, 100,   0,   0,   0,   0,   0,   0},
	{0, 100,   0,   0,   0,   0,   0,   0},
	{6,  10,   0,   0,   0,   0,   0,   0},
	{3,  80,   8,   0,   0,   0,   0,   0},
	{6,  20,   0,   0,   0,   0,   0,   0},
	{0, 100,   0,   0,   0,  10,   0,   0},
	{0, 100,   0,   0,   0,   0,   0,   0},
	{0, 100,   0,   0,   0,   0,   0,   0},
	{0, 100,   0,   0,   0,   0,   0,   0},
	{0, 100,   0,   0,   0,   0,   0,   0},
	{0, 100,   0,   0,   0,   0,   0,   0},
	{0, 100,   0,   0,   0,   0,   0,   0},
	{0, 100,   0,   0,   0,   0,   0,   0},
	{0, 100,   0,   0,   0,   0,   0,   0}
};

const struct MissionId Constant_MissionIds[12] = {
	{0, 0},
	{1, 1},
	{2, 3},
	{4, 5},
	{6, 7},
	{8, 9},
	{10, 11},
	{12, 13},
	{14, 15},
	{16, 17},
	{18, 19},
	{20, 21},
};
