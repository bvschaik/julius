#include "Data/Building.h"
#include "Data/Settings.h"
#include "Data/Grid.h"
#include "Data/Model.h"
#include "Data/Walker.h"

#define FOR_XY_RADIUS(block) \
	int xMin = x - 2;\
	int yMin = y - 2;\
	int xMax = x + 2;\
	int yMax = y + 2;\
	if (xMin < 0) xMin = 0;\
	if (yMin < 0) yMin = 0;\
	if (xMax >= Data_Settings_Map.width) xMax = Data_Settings_Map.width - 1;\
	if (yMax >= Data_Settings_Map.height) yMax = Data_Settings_Map.height - 1;\
	int gridOffset = Data_Settings_Map.gridStartOffset + 162 * yMin + xMin;\
	for (int yy = yMin; yy <= yMax; yy++) {\
		for (int xx = xMin; xx <= xMax; xx++) {\
			int buildingId = Data_Grid_buildingIds[gridOffset];\
			if (buildingId) {\
				block;\
			}\
			++gridOffset;\
		}\
		gridOffset += 162 - (xMax - xMin + 1);\
	}

// TODO move to some building utility file
static int getMainBuildingId(int buildingId)
{
	return buildingId;
}

static int provideEngineerCoverage(int x, int y, int *maxDamageRiskSeen)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].type == Building_Hippodrome) {
			buildingId = getMainBuildingId(buildingId);
		}
		if (Data_Buildings[buildingId].damageRisk > *maxDamageRiskSeen) {
			*maxDamageRiskSeen = Data_Buildings[buildingId].damageRisk;
		}
		Data_Buildings[buildingId].damageRisk = 0;
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			serviced++;
		}
	);
	return serviced;
}

static int providePrefectFireCoverage(int x, int y, int *maxFireRiskSeen)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].type == Building_Hippodrome) {
			buildingId = getMainBuildingId(buildingId);
		}
		if (Data_Buildings[buildingId].fireRisk > *maxFireRiskSeen) {
			*maxFireRiskSeen = Data_Buildings[buildingId].fireRisk;
		}
		Data_Buildings[buildingId].fireRisk = 0;
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			serviced++;
		}
	);
	return serviced;
}

static int getPrefectCrimeCoverage(int x, int y)
{
	int minHappinessSeen = 100;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].sentiment.houseHappiness < minHappinessSeen) {
			minHappinessSeen = Data_Buildings[buildingId].sentiment.houseHappiness;
		}
	);
	return minHappinessSeen;
}

static int provideTheaterCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.theater = 96;
			serviced++;
		}
	);
	return serviced;
}

static int provideAmphitheaterCoverage(int x, int y, int numShows)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.amphitheaterActor = 96;
			if (numShows == 2) {
				Data_Buildings[buildingId].data.house.amphitheaterGladiator = 96;
			}
			serviced++;
		}
	);
	return serviced;
}

static int provideColosseumCoverage(int x, int y, int numShows)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.colosseumGladiator = 96;
			if (numShows == 2) {
				Data_Buildings[buildingId].data.house.colosseumLion = 96;
			}
			serviced++;
		}
	);
	return serviced;
}

static int provideHippodromeCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.hippodrome = 96;
			serviced++;
		}
	);
	return serviced;
}

static int provideBathhouseCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.bathhouse = 96;
			serviced++;
		}
	);
	return serviced;
}

static int provideReligionCoverage(int x, int y, int god)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			switch (god) {
				case 0:
					Data_Buildings[buildingId].data.house.templeCeres = 96;
					break;
				case 1:
					Data_Buildings[buildingId].data.house.templeNeptune = 96;
					break;
				case 2:
					Data_Buildings[buildingId].data.house.templeMercury = 96;
					break;
				case 3:
					Data_Buildings[buildingId].data.house.templeMars = 96;
					break;
				case 4:
					Data_Buildings[buildingId].data.house.templeVenus = 96;
					break;
			}
			serviced++;
		}
	);
	return serviced;
}

static int provideSchoolCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.school = 96;
			serviced++;
		}
	);
	return serviced;
}

static int provideAcademyCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.academy = 96;
			serviced++;
		}
	);
	return serviced;
}

static int provideLibraryCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.library = 96;
			serviced++;
		}
	);
	return serviced;
}

static int provideBarberCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.barber = 96;
			serviced++;
		}
	);
	return serviced;
}

static int provideClinicCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.clinic = 96;
			serviced++;
		}
	);
	return serviced;
}

static int provideHospitalCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.hospital = 96;
			serviced++;
		}
	);
	return serviced;
}

static int provideMissionaryCoverage(int x, int y)
{
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].type == Building_NativeHut ||
			Data_Buildings[buildingId].type == Building_NativeMeeting) {
			Data_Buildings[buildingId].sentiment.nativeRisk = 0;
		}
	);
	return 1;
}

static int provideLaborSeekerCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			serviced++;
		}
	);
	return serviced;
}

static int provideTaxCollectorCoverage(int x, int y, char *maxTaxMultiplier)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			int taxMultiplier = Data_Model_Houses[Data_Buildings[buildingId].subtype.houseLevel].taxMultiplier;
			if (taxMultiplier > *maxTaxMultiplier) {
				*maxTaxMultiplier = taxMultiplier;
			}
			Data_Buildings[buildingId].houseTaxCoverage = 50;
			serviced++;
		}
	);
	return serviced;
}

int Walker_provideServiceCoverage(int walkerId)
{
	int numHousesServiced = 0;
	int x = Data_Walkers[walkerId].x;
	int y = Data_Walkers[walkerId].y;
	switch (Data_Walkers[walkerId].type) {
		case Walker_Patrician:
			return 0;
		case Walker_LaborSeeker:
			numHousesServiced = provideLaborSeekerCoverage(x, y);
			break;
		case Walker_TaxCollector:
			numHousesServiced = provideTaxCollectorCoverage(x, y, &Data_Walkers[walkerId].minMaxSeen);
			break;
		case Walker_MarketTrader:
		case Walker_MarketBuyer:
			// TODO numHousesServiced = provideMarketGoods(Data_Walkers[walkerId].buildingId, x, y);
			break;
		case Walker_BathhouseWorker:
			numHousesServiced = provideBathhouseCoverage(x, y);
			break;
		case Walker_SchoolChild:
			numHousesServiced = provideSchoolCoverage(x, y);
			break;
		case Walker_Teacher:
			numHousesServiced = provideAcademyCoverage(x, y);
			break;
		case Walker_Librarian:
			numHousesServiced = provideLibraryCoverage(x, y);
			break;
		case Walker_Barber:
			numHousesServiced = provideBarberCoverage(x, y);
			break;
		case Walker_Doctor:
			numHousesServiced = provideClinicCoverage(x, y);
			break;
		case Walker_Surgeon:
			numHousesServiced = provideHospitalCoverage(x, y);
			break;
		case Walker_Missionary:
			numHousesServiced = provideMissionaryCoverage(x, y);
			break;
		case Walker_Priest:
			switch (Data_Buildings[Data_Walkers[walkerId].buildingId].type) {
				case Building_SmallTempleCeres:
				case Building_LargeTempleCeres:
					numHousesServiced = provideReligionCoverage(x, y, 0);
					break;
				case Building_SmallTempleNeptune:
				case Building_LargeTempleNeptune:
					numHousesServiced = provideReligionCoverage(x, y, 1);
					break;
				case Building_SmallTempleMercury:
				case Building_LargeTempleMercury:
					numHousesServiced = provideReligionCoverage(x, y, 2);
					break;
				case Building_SmallTempleMars:
				case Building_LargeTempleMars:
					numHousesServiced = provideReligionCoverage(x, y, 3);
					break;
				case Building_SmallTempleVenus:
				case Building_LargeTempleVenus:
					numHousesServiced = provideReligionCoverage(x, y, 4);
					break;
				default:
					break;
			}
			break;

	}
	return 0;
/*
TODO
  v2 = 0;
  switch ( walker_0a_type[128 * walkerId] )
  {
    case Walker_Engineer:
      v2 = j_fun_walkerProvideEngineerCoverage(walker_14_x[128 * walkerId], walker_15_y[128 * walkerId]);
      if ( *(_DWORD *)&engineerMaxDamageSeen <= (signed int)(unsigned __int8)walker_maxLevelOrMinHappinessSeen[128 * walkerId] )
      {
        if ( (signed int)(unsigned __int8)walker_maxLevelOrMinHappinessSeen[128 * walkerId] <= 10 )
          walker_maxLevelOrMinHappinessSeen[128 * walkerId] = 0;
        else
          walker_maxLevelOrMinHappinessSeen[128 * walkerId] -= 10;
      }
      else
      {
        walker_maxLevelOrMinHappinessSeen[128 * walkerId] = engineerMaxDamageSeen;
      }
      break;
    case Walker_Prefect:
      v2 = j_fun_walkerProvidePrefectFireCoverage(walker_14_x[128 * walkerId], walker_15_y[128 * walkerId]);
      j_fun_walkerProvidePrefectCrimeCoverage(walker_14_x[128 * walkerId], walker_15_y[128 * walkerId]);
      walker_maxLevelOrMinHappinessSeen[128 * walkerId] = prefectMinHappinessSeen;
      break;
    case Walker_Rioter:
      if ( sub_4026D5(walkerId) == 1 )
        return 1;
      break;
    case Walker_Actor:
      if ( (unsigned __int8)walker_actionState[128 * walkerId] != 94
        && (unsigned __int8)walker_actionState[128 * walkerId] != 95 )
        v4 = walker_destinationBuildingId[64 * walkerId];
      else
        v4 = walker_buildingId[64 * walkerId];
      if ( building_0a_type[64 * v4] == B_Theater )
      {
        v2 = j_fun_walkerProvideTheaterAccess(walker_14_x[128 * walkerId], walker_15_y[128 * walkerId]);
      }
      else
      {
        if ( building_0a_type[64 * v4] == B_Amphitheater )
        {
          if ( building_65_house_bathhouse_dock_numships_entert_days[128 * v4] )
            v2 = j_fun_walkerProvideAmphitheaterAccess(walker_14_x[128 * walkerId], walker_15_y[128 * walkerId], 2);
          else
            v2 = j_fun_walkerProvideAmphitheaterAccess(walker_14_x[128 * walkerId], walker_15_y[128 * walkerId], 1);
        }
      }
      break;
    case Walker_Gladiator:
      if ( (unsigned __int8)walker_actionState[128 * walkerId] != 94
        && (unsigned __int8)walker_actionState[128 * walkerId] != 95 )
        v5 = walker_destinationBuildingId[64 * walkerId];
      else
        v5 = walker_buildingId[64 * walkerId];
      if ( building_0a_type[64 * v5] == B_Amphitheater )
      {
        if ( building_66_house_hospital_entert_days2[128 * v5] )
          v2 = j_fun_walkerProvideAmphitheaterAccess(walker_14_x[128 * walkerId], walker_15_y[128 * walkerId], 2);
        else
          v2 = j_fun_walkerProvideAmphitheaterAccess(walker_14_x[128 * walkerId], walker_15_y[128 * walkerId], 1);
      }
      else
      {
        if ( building_0a_type[64 * v5] == B_Colosseum )
        {
          if ( building_65_house_bathhouse_dock_numships_entert_days[128 * v5] )
            v2 = j_fun_walkerProvideColosseumAccess(walker_14_x[128 * walkerId], walker_15_y[128 * walkerId], 2);
          else
            v2 = j_fun_walkerProvideColosseumAccess(walker_14_x[128 * walkerId], walker_15_y[128 * walkerId], 1);
        }
      }
      break;
    case Walker_LionTamer:
      if ( (unsigned __int8)walker_actionState[128 * walkerId] != 94
        && (unsigned __int8)walker_actionState[128 * walkerId] != 95 )
        v6 = walker_destinationBuildingId[64 * walkerId];
      else
        v6 = walker_buildingId[64 * walkerId];
      if ( building_66_house_hospital_entert_days2[128 * v6] )
        v2 = j_fun_walkerProvideColosseumAccess(walker_14_x[128 * walkerId], walker_15_y[128 * walkerId], 2);
      else
        v2 = j_fun_walkerProvideColosseumAccess(walker_14_x[128 * walkerId], walker_15_y[128 * walkerId], 1);
      break;
    case Walker_Charioteer:
      v2 = j_fun_walkerProvideHippodromeAccess(walker_14_x[128 * walkerId], walker_15_y[128 * walkerId]);
      break;
  }
  v7 = walker_buildingId[64 * walkerId];
  if ( walker_buildingId[64 * walkerId] )
  {
    building_12_walkerServiceAccess[64 * v7] += v2;
    if ( building_12_walkerServiceAccess[64 * v7] > 300 )
      building_12_walkerServiceAccess[64 * v7] = 300;
  }
  return 0;
*/
}
