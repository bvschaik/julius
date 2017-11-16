#include "alldata.hpp"

#define DATA_INTERN 1
#include "data/grid.hpp"
#include "empire/object.h"

_Data_Scenario Data_Scenario;

struct _Data_Event Data_Event = {0};

struct _Data_Settings Data_Settings;
struct _Data_Settings_Map Data_Settings_Map;

struct _Data_Screen Data_Screen;// = { 800, 600, {80, 60} };

struct _Data_CityInfo Data_CityInfo;

struct _Data_CityInfo_Extra Data_CityInfo_Extra;
struct _Data_CityInfo_Resource Data_CityInfo_Resource;

struct Data_KeyboardInput Data_KeyboardInput;

struct _Data_State Data_State;

struct Data_Figure Data_Figures[MAX_FIGURES];
struct _Data_Figure_Extra Data_Figure_Extra;

struct Data_Building Data_Buildings[MAX_BUILDINGS];
struct _Data_Buildings_Extra Data_Buildings_Extra;

struct Data_Sound_City Data_Sound_City[70];

struct Data_CityView Data_CityView;

struct Data_InvasionWarning Data_InvasionWarnings[MAX_INVASION_WARNINGS];

struct _Data_FileList Data_FileList;

struct _Data_Routes Data_Routes;

struct _Data_Debug Data_Debug;

const int Constant_SalaryForRank[11] = {0, 2, 5, 8, 12, 20, 30, 40, 60, 80, 100};
const int Constant_DirectionGridOffsets[8] = {-162, -161, 1, 163, 162, 161, -1, -163};

const struct MissionId Constant_MissionIds[12] =
{
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
