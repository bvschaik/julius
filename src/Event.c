#include "Event.h"

#include "Building.h"
#include "CityInfo.h"
#include "Figure.h"
#include "Formation.h"
#include "Resource.h"
#include "Routing.h"
#include "SidebarMenu.h"
#include "TerrainGraphics.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Grid.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"

#include "building/count.h"
#include "city/message.h"
#include "core/calc.h"
#include "core/random.h"
#include "empire/city.h"
#include "empire/object.h"
#include "empire/trade_prices.h"
#include "empire/trade_route.h"
#include "empire/type.h"
#include "game/time.h"
#include "game/tutorial.h"
#include "scenario/property.h"
#include "scenario/data.h"
#include "sound/effect.h"

#include <string.h>

enum {
	RandomEvent_RomeRaisesWages = 1,
	RandomEvent_RomeLowersWages = 2,
	RandomEvent_LandTradeDisrupted = 3,
	RandomEvent_SeaTradeDisrupted = 4,
	RandomEvent_ContaminatedWater = 5,
	RandomEvent_IronMineCollapsed = 6,
	RandomEvent_ClayPitFlooded = 7
};

static const int randomEventProbability[128] = {
	0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 0, 3, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 6,
	0, 0, 2, 0, 0, 0, 7, 0, 5, 0, 0, 7, 0, 0, 0, 0,
	0, 7, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
	6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 6, 0, 0,
	0, 7, 0, 1, 6, 0, 0, 0, 0, 0, 2, 0, 0, 4, 0, 0,
	0, 0, 3, 0, 7, 4, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0
};

void Event_handleRandomEvents()
{
	int event = randomEventProbability[random_byte()];
	if (event <= 0) {
		return;
	}
	switch (event) {
		case RandomEvent_RomeRaisesWages:
			if (Data_Scenario.raiseWagesEnabled) {
				if (Data_CityInfo.wagesRome < 45) {
					Data_CityInfo.wagesRome += 1 + (random_byte_alt() & 3);
					if (Data_CityInfo.wagesRome > 45) {
						Data_CityInfo.wagesRome = 45;
					}
					city_message_post(1, MESSAGE_ROME_RAISES_WAGES, 0, 0);
				}
			}
			break;
		case RandomEvent_RomeLowersWages:
			if (Data_Scenario.lowerWagesEnabled) {
				if (Data_CityInfo.wagesRome > 5) {
					Data_CityInfo.wagesRome -= 1 + (random_byte_alt() & 3);
					city_message_post(1, MESSAGE_ROME_LOWERS_WAGES, 0, 0);
				}
			}
			break;
		case RandomEvent_LandTradeDisrupted:
			if (Data_Scenario.landTradeProblemEnabled) {
				if (Data_CityInfo.tradeNumOpenLandRoutes > 0) {
					Data_CityInfo.tradeLandProblemDuration = 48;
					if (scenario_property_climate() == CLIMATE_DESERT) {
						city_message_post(1, MESSAGE_LAND_TRADE_DISRUPTED_SANDSTORMS, 0, 0);
					} else {
						city_message_post(1, MESSAGE_LAND_TRADE_DISRUPTED_LANDSLIDES, 0, 0);
					}
				}
			}
			break;
		case RandomEvent_SeaTradeDisrupted:
			if (Data_Scenario.seaTradeProblemEnabled) {
				if (Data_CityInfo.tradeNumOpenSeaRoutes > 0) {
					Data_CityInfo.tradeSeaProblemDuration = 48;
					city_message_post(1, MESSAGE_SEA_TRADE_DISRUPTED, 0, 0);
				}
			}
			break;
		case RandomEvent_ContaminatedWater:
			if (Data_Scenario.contaminatedWaterEnabled) {
				if (Data_CityInfo.population > 200) {
					int change;
					if (Data_CityInfo.healthRate > 80) {
						change = -50;
					} else if (Data_CityInfo.healthRate > 60) {
						change = -40;
					} else {
						change = -25;
					}
					CityInfo_Population_changeHealthRate(change);
					city_message_post(1, MESSAGE_CONTAMINATED_WATER, 0, 0);
				}
			}
			break;
		case RandomEvent_IronMineCollapsed:
			if (Data_Scenario.ironMineCollapseEnabled) {
				int gridOffset = Building_collapseFirstOfType(BUILDING_IRON_MINE);
				if (gridOffset) {
					city_message_post(1, MESSAGE_IRON_MINE_COLLAPED, 0, gridOffset);
				}
			}
			break;
		case RandomEvent_ClayPitFlooded:
			if (Data_Scenario.clayPitFloodEnabled) {
				int gridOffset = Building_collapseFirstOfType(BUILDING_CLAY_PIT);
				if (gridOffset) {
					city_message_post(1, MESSAGE_CLAY_PIT_FLOODED, 0, gridOffset);
				}
			}
			break;
	}
}
