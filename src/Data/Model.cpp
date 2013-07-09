#include "Model.h"
#include "Scenario.h"
#include "Settings.h"
#include "Message.h"
#include "Random.h"
#include "Empire.h"
#include "Graphics.h"
#include "Screen.h"
#include "Mouse.h"
#include "Buttons.h"
#include "Language.h"
#include "CityInfo.h"

struct Data_Model_House Data_Model_Houses[20];
struct Data_Model_Building Data_Model_Buildings[130];

struct Data_Scenario Data_Scenario;

struct Data_Settings Data_Settings;
struct Data_Settings_Map Data_Settings_Map;

struct Data_Message Data_Message;
struct Data_Random Data_Random;

struct Data_Empire_Object Data_Empire_Objects[200];
struct Data_Empire_City Data_Empire_Cities[41];
struct Data_Empire_Trade Data_Empire_Trade;

struct Data_Graphics_Main Data_Graphics_Main;
struct Data_Graphics_Enemy Data_Graphics_Enemy;
struct Data_Graphics_PixelData Data_Graphics_PixelData;

struct Data_Screen Data_Screen = { 800, 600, {80, 60} };

struct Data_Mouse Data_Mouse;

struct Data_Buttons_Arrow Data_Buttons_Arrow;

struct Data_Language_Text Data_Language_Text;

struct Data_CityInfo Data_CityInfo;
struct Data_CityInfo_Buildings Data_CityInfo_Buildings;
struct Data_CityInfo_CultureCoverage Data_CityInfo_CultureCoverage;
struct Data_CityInfo_Extra Data_CityInfo_Extra;
