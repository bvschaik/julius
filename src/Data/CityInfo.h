#ifndef DATA_CITYINFO_H
#define DATA_CITYINFO_H

#include <stdint.h>

extern struct _Data_CityInfo {
	int32_t riotersOrAttackingNativesInCity;
	int32_t nativeAttackDuration;
	int8_t distantBattleEnemyMonthsTraveled;
	int8_t distantBattleRomanMonthsTraveled;
} Data_CityInfo;

#endif
