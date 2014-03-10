#include "CityInfo.h"
#include "Data/CityInfo.h"

void CityInfo_Gods_calculate()
{
	// TODO implement
}

void CityInfo_Gods_calculateLeastHappy()
{
	int maxGod = 0;
	int maxWrath = 0;
	if (Data_CityInfo.godWrathCeres > maxWrath) {
		maxGod = 1;
		maxWrath = Data_CityInfo.godWrathCeres;
	}
	if (Data_CityInfo.godWrathNeptune > maxWrath) {
		maxGod = 2;
		maxWrath = Data_CityInfo.godWrathNeptune;
	}
	if (Data_CityInfo.godWrathMercury > maxWrath) {
		maxGod = 3;
		maxWrath = Data_CityInfo.godWrathMercury;
	}
	if (Data_CityInfo.godWrathMars > maxWrath) {
		maxGod = 4;
		maxWrath = Data_CityInfo.godWrathMars;
	}
	if (Data_CityInfo.godWrathVenus > maxWrath) {
		maxGod = 5;
		maxWrath = Data_CityInfo.godWrathVenus;
	}
	if (maxGod <= 0) {
		int minHappiness = 40;
		if (Data_CityInfo.godHappinessCeres < minHappiness) {
			maxGod = 1;
			minHappiness = Data_CityInfo.godHappinessCeres;
		}
		if (Data_CityInfo.godHappinessNeptune < minHappiness) {
			maxGod = 2;
			minHappiness = Data_CityInfo.godHappinessNeptune;
		}
		if (Data_CityInfo.godHappinessMercury < minHappiness) {
			maxGod = 3;
			minHappiness = Data_CityInfo.godHappinessMercury;
		}
		if (Data_CityInfo.godHappinessMars < minHappiness) {
			maxGod = 4;
			minHappiness = Data_CityInfo.godHappinessMars;
		}
		if (Data_CityInfo.godHappinessVenus < minHappiness) {
			maxGod = 5;
			minHappiness = Data_CityInfo.godHappinessVenus;
		}
	}
	Data_CityInfo.godLeastHappy = maxGod;
}