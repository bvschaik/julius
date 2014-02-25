#include "Sound.h"
#include "Data/Sound.h"
#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Time.h"

#include <string.h>

static int buildingIdToChannelId[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0-9
	1, 1, 1, 1, 1, 1, 2, 2, 2, 2, //10-19
	3, 3, 3, 3, 4, 4, 4, 4, 5, 5, //20-29
	6, 7, 8, 9, 10, 11, 12, 13, 0, 14, //30-39
	0, 0, 0, 0, 0, 0, 15, 16, 17, 18, //40-49
	0, 19, 20, 21, 0, 22, 0, 23, 24, 24, //50-59
	25, 26, 27, 28, 29, 25, 26, 27, 28, 29, //60-69
	30, 31, 32, 0, 33, 34, 35, 36, 36, 36, //70-79
	63, 37, 0, 0, 38, 38, 39, 39, 0, 0, // 80-89
	40, 0, 0, 0, 43, 0, 0, 0, 44, 45, //90-99
	46, 47, 48, 49, 50, 51, 52, 53, 54, 55, //100-109
	56, 57, 58, 59, 60, 0, 0, 0, 0, 0, //110-119
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //120-129
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //130-139
	0, 0, 0, 0, 0, 0 //140-145
};

static TimeMillis lastUpdateTime;

void Sound_City_init()
{
	lastUpdateTime = Time_getMillis();
	memset(Data_Sound_City, 0, 8960);
	for (int i = 0; i < 70; i++) {
		Data_Sound_City[i].lastPlayedTime = lastUpdateTime;
	}
	for (int i = 1; i < 63; i++) {
		Data_Sound_City[i].inUse = 1;
		Data_Sound_City[i].viewsThreshold = 200;
		Data_Sound_City[i].numChannels = 1;
		Data_Sound_City[i].delayMillis = 30000;
	}
	Data_Sound_City[1].channels[0] = SoundChannel_City_HouseSlum;
	Data_Sound_City[2].channels[0] = SoundChannel_City_HousePoor;
	Data_Sound_City[3].channels[0] = SoundChannel_City_HouseMedium;
	Data_Sound_City[4].channels[0] = SoundChannel_City_HouseGood;
	Data_Sound_City[5].channels[0] = SoundChannel_City_HousePosh;
	Data_Sound_City[6].channels[0] = SoundChannel_City_Amphitheater;
	Data_Sound_City[7].channels[0] = SoundChannel_City_Theater;
	Data_Sound_City[8].channels[0] = SoundChannel_City_Hippodrome;
	Data_Sound_City[9].channels[0] = SoundChannel_City_Colosseum;
	Data_Sound_City[10].channels[0] = SoundChannel_City_GladiatorSchool;
	Data_Sound_City[11].channels[0] = SoundChannel_City_LionPit;
	Data_Sound_City[12].channels[0] = SoundChannel_City_ActorColony;
	Data_Sound_City[13].channels[0] = SoundChannel_City_ChariotMaker;
	Data_Sound_City[14].channels[0] = SoundChannel_City_Garden;
	Data_Sound_City[15].channels[0] = SoundChannel_City_Clinic;
	Data_Sound_City[16].channels[0] = SoundChannel_City_Hospital;
	Data_Sound_City[17].channels[0] = SoundChannel_City_Bathhouse;
	Data_Sound_City[18].channels[0] = SoundChannel_City_Barber;
	Data_Sound_City[19].channels[0] = SoundChannel_City_School;
	Data_Sound_City[20].channels[0] = SoundChannel_City_Academy;
	Data_Sound_City[21].channels[0] = SoundChannel_City_Library;
	Data_Sound_City[22].channels[0] = SoundChannel_City_Prefecture;
	Data_Sound_City[23].channels[0] = SoundChannel_City_Fort;
	Data_Sound_City[24].channels[0] = SoundChannel_City_Tower;
	Data_Sound_City[25].channels[0] = SoundChannel_City_TempleCeres;
	Data_Sound_City[26].channels[0] = SoundChannel_City_TempleNeptune;
	Data_Sound_City[27].channels[0] = SoundChannel_City_TempleMercury;
	Data_Sound_City[28].channels[0] = SoundChannel_City_TempleMars;
	Data_Sound_City[29].channels[0] = SoundChannel_City_TempleVenus;
	Data_Sound_City[30].channels[0] = SoundChannel_City_Market;
	Data_Sound_City[31].channels[0] = SoundChannel_City_Granary;
	Data_Sound_City[32].channels[0] = SoundChannel_City_Warehouse;
	Data_Sound_City[33].channels[0] = SoundChannel_City_Shipyard;
	Data_Sound_City[34].channels[0] = SoundChannel_City_Dock;
	Data_Sound_City[35].channels[0] = SoundChannel_City_Wharf;
	Data_Sound_City[36].channels[0] = SoundChannel_City_Palace;
	Data_Sound_City[37].channels[0] = SoundChannel_City_EngineersPost;
	Data_Sound_City[38].channels[0] = SoundChannel_City_Senate;
	Data_Sound_City[39].channels[0] = SoundChannel_City_Forum;
	Data_Sound_City[40].channels[0] = SoundChannel_City_Reservoir;
	Data_Sound_City[41].channels[0] = SoundChannel_City_Fountain;
	Data_Sound_City[42].channels[0] = SoundChannel_City_Well;
	Data_Sound_City[43].channels[0] = SoundChannel_City_MilitaryAcademy;
	Data_Sound_City[44].channels[0] = SoundChannel_City_Oracle;
	Data_Sound_City[45].channels[0] = SoundChannel_City_BurningRuin;
	Data_Sound_City[46].channels[0] = SoundChannel_City_WheatFarm;
	Data_Sound_City[47].channels[0] = SoundChannel_City_VegetableFarm;
	Data_Sound_City[48].channels[0] = SoundChannel_City_FruitFarm;
	Data_Sound_City[49].channels[0] = SoundChannel_City_OliveFarm;
	Data_Sound_City[50].channels[0] = SoundChannel_City_VineFarm;
	Data_Sound_City[51].channels[0] = SoundChannel_City_PigFarm;
	Data_Sound_City[52].channels[0] = SoundChannel_City_Quarry;
	Data_Sound_City[53].channels[0] = SoundChannel_City_IronMine;
	Data_Sound_City[54].channels[0] = SoundChannel_City_TimberYard;
	Data_Sound_City[55].channels[0] = SoundChannel_City_ClayPit;
	Data_Sound_City[56].channels[0] = SoundChannel_City_WineWorkshop;
	Data_Sound_City[57].channels[0] = SoundChannel_City_OilWorkshop;
	Data_Sound_City[58].channels[0] = SoundChannel_City_WeaponsWorkshop;
	Data_Sound_City[59].channels[0] = SoundChannel_City_FurnitureWorkshop;
	Data_Sound_City[60].channels[0] = SoundChannel_City_PotteryWorkshop;
	Data_Sound_City[61].channels[0] = SoundChannel_City_EmptyLand;
	Data_Sound_City[62].channels[0] = SoundChannel_City_River;
	Data_Sound_City[63].channels[0] = SoundChannel_City_MissionPost;
}

void Sound_City_markBuildingView(int buildingId, int direction)
{
	if (!Data_Buildings[buildingId].inUse) {
		return;
	}
	int channel = buildingIdToChannelId[buildingId];
	if (!channel) {
		return;
	}
	int type = Data_Buildings[buildingId].type;
	if (type == Building_Theater || type == Building_Amphitheater ||
		type == Building_GladiatorSchool || type == Building_Hippodrome) {
		// entertainment is shut off when caesar invades
		if (Data_Buildings[buildingId].numWorkers <= 0 ||
			Data_CityInfo.numImperialSoldiersInCity > 0) {
			return;
		}
	}

	Data_Sound_City[channel].available = 1;
	++Data_Sound_City[channel].totalViews;
	++Data_Sound_City[channel].directionViews[direction];
}

void Sound_City_decayViews()
{
	for (int i = 0; i < 70; i++) {
		for (int d = 0; d < 5; d++) {
			Data_Sound_City[i].directionViews[d] = 0;
		}
		Data_Sound_City[i].totalViews /= 2;
	}
}

void Sound_City_play()
{
	TimeMillis now = Time_getMillis();
	for (int i = 1; i < 70; i++) {
		Data_Sound_City[i].shouldPlay = 0;
		if (Data_Sound_City[i].available) {
			Data_Sound_City[i].available = 0;
			if (Data_Sound_City[i].totalViews >= Data_Sound_City[i].viewsThreshold) {
				if (now - Data_Sound_City[i].lastPlayedTime >= Data_Sound_City[i].delayMillis) {
					Data_Sound_City[i].shouldPlay = 1;
				}
			}
		} else {
			Data_Sound_City[i].totalViews = 0;
			for (int d = 0; d < 5; d++) {
				Data_Sound_City[i].directionViews[d] = 0;
			}
		}
	}

	if (now - lastUpdateTime < 2000) {
		// Only play 1 sound every 2 seconds
		return;
	}
	TimeMillis maxDelay = 0;
	int maxSoundId = 0;
	for (int i = 1; i < 70; i++) {
		if (Data_Sound_City[i].shouldPlay) {
			if (now - Data_Sound_City[i].lastPlayedTime > maxDelay) {
				maxDelay = now - Data_Sound_City[i].lastPlayedTime;
				maxSoundId = i;
			}
		}
	}
	if (!maxSoundId) {
		return;
	}
	
	// always only one channel available... use it
	int channel = Data_Sound_City[maxSoundId].channels[0];
	int direction;
	if (Data_Sound_City[maxSoundId].directionViews[SoundDirectionCenter] > 10) {
		direction = SoundDirectionCenter;
	} else if (Data_Sound_City[maxSoundId].directionViews[SoundDirectionLeft] > 10) {
		direction = SoundDirectionLeft;
	} else if (Data_Sound_City[maxSoundId].directionViews[SoundDirectionRight] > 10) {
		direction = SoundDirectionRight;
	} else {
		direction = SoundDirectionCenter;
	}

	Sound_playCityChannel_internal(channel, direction);
	lastUpdateTime = now;
	Data_Sound_City[maxSoundId].lastPlayedTime = now;
	Data_Sound_City[maxSoundId].totalViews = 0;
	for (int d = 0; d < 5; d++) {
		Data_Sound_City[maxSoundId].directionViews[d] = 0;
	}
	Data_Sound_City[maxSoundId].timesPlayed++;
}

