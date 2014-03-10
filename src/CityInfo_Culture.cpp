#include "CityInfo.h"
#include "Calc.h"
#include "Data/CityInfo.h"

#define TOP(x) if (Data_CityInfo_CultureCoverage.x > 100) Data_CityInfo_CultureCoverage.x = 100;

void CityInfo_Culture_updateCoveragePercentages()
{
	// entertainment
	Data_CityInfo_CultureCoverage.theater = Calc_getPercentage(
		500 * Data_CityInfo_Buildings.theater.working, Data_CityInfo.population);
	Data_CityInfo_CultureCoverage.amphitheater = Calc_getPercentage(
		800 * Data_CityInfo_Buildings.amphitheater.working, Data_CityInfo.population);
	Data_CityInfo_CultureCoverage.colosseum = Calc_getPercentage(
		1500 * Data_CityInfo_Buildings.colosseum.working, Data_CityInfo.population);
	if (Data_CityInfo_Buildings.hippodrome.working <= 0) {
		Data_CityInfo_CultureCoverage.hippodrome = 0;
	} else {
		Data_CityInfo_CultureCoverage.hippodrome = 100;
	}
	TOP(theater);
	TOP(amphitheater);
	TOP(colosseum);

	// religion
	Data_CityInfo_CultureCoverage.religionCeres = Calc_getPercentage(
			500 * Data_CityInfo_Buildings.oracle.total +
			750 * Data_CityInfo_Buildings.smallTempleCeres.working +
			1500 * Data_CityInfo_Buildings.largeTempleCeres.working,
		Data_CityInfo.population);
	Data_CityInfo_CultureCoverage.religionNeptune = Calc_getPercentage(
			500 * Data_CityInfo_Buildings.oracle.total +
			750 * Data_CityInfo_Buildings.smallTempleNeptune.working +
			1500 * Data_CityInfo_Buildings.largeTempleNeptune.working,
		Data_CityInfo.population);
	Data_CityInfo_CultureCoverage.religionMercury = Calc_getPercentage(
			500 * Data_CityInfo_Buildings.oracle.total +
			750 * Data_CityInfo_Buildings.smallTempleMercury.working +
			1500 * Data_CityInfo_Buildings.largeTempleMercury.working,
		Data_CityInfo.population);
	Data_CityInfo_CultureCoverage.religionMars = Calc_getPercentage(
			500 * Data_CityInfo_Buildings.oracle.total +
			750 * Data_CityInfo_Buildings.smallTempleMars.working +
			1500 * Data_CityInfo_Buildings.largeTempleMars.working,
		Data_CityInfo.population);
	Data_CityInfo_CultureCoverage.religionVenus = Calc_getPercentage(
			500 * Data_CityInfo_Buildings.oracle.total +
			750 * Data_CityInfo_Buildings.smallTempleVenus.working +
			1500 * Data_CityInfo_Buildings.largeTempleVenus.working,
		Data_CityInfo.population);
	Data_CityInfo_CultureCoverage.oracle = Calc_getPercentage(
		500 * Data_CityInfo_Buildings.oracle.total, Data_CityInfo.population);
	TOP(religionCeres);
	TOP(religionNeptune);
	TOP(religionMercury);
	TOP(religionMars);
	TOP(religionVenus);

	// education
	Data_CityInfo_CultureCoverage.school = Calc_getPercentage(
		75 * Data_CityInfo_Buildings.school.working, Data_CityInfo.populationSchoolAge);
	Data_CityInfo_CultureCoverage.library = Calc_getPercentage(
		800 * Data_CityInfo_Buildings.library.working, Data_CityInfo.population);
	Data_CityInfo_CultureCoverage.academy = Calc_getPercentage(
		100 * Data_CityInfo_Buildings.academy.working, Data_CityInfo.populationAcademyAge);
	TOP(school);
	TOP(library);
	TOP(academy);

	// health
	Data_CityInfo_CultureCoverage.hospital = Calc_getPercentage(
		1000 * Data_CityInfo_Buildings.hospital.working, Data_CityInfo.population);
	TOP(hospital);
}
