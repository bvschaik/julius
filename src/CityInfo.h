#ifndef CityInfo_H
#define CityInfo_H

void CityInfo_init();
void CityInfo_initGameTime();

void CityInfo_Imperial_calculateGiftCosts();

void CityInfo_Gods_reset();
void CityInfo_Gods_calculateMoods(int updateMood);
int CityInfo_Gods_calculateLeastHappy();
void CityInfo_Gods_checkFestival();

void CityInfo_Ratings_calculate(int isYearlyUpdate);
void CityInfo_Ratings_updateCultureExplanation();
void CityInfo_Ratings_updateFavorExplanation();
void CityInfo_Ratings_updateProsperityExplanation();
void CityInfo_Ratings_updatePeaceExplanation();
void CityInfo_Ratings_reduceProsperityAfterBailout();

void CityInfo_Ratings_sendGiftToCaesar();
void CityInfo_Ratings_changeFavor(int amount);
void CityInfo_Ratings_setMaxFavor(int maxFavor);

void CityInfo_Finance_decayTaxCollectorAccess();
void CityInfo_Finance_calculateTotals();
void CityInfo_Finance_calculateEstimatedWages();
void CityInfo_Finance_calculateEstimatedTaxes();

void CityInfo_Finance_updateInterest();
void CityInfo_Finance_updateSalary();

void CityInfo_Finance_handleMonthChange();
void CityInfo_Finance_handleYearChange();
void CityInfo_Finance_spendOnConstruction(int amount);

void CityInfo_Population_recordMonthlyPopulation();
int CityInfo_Population_getPeopleOfWorkingAge();
int CityInfo_Population_getNumberOfSchoolAgeChildren();
int CityInfo_Population_getNumberOfAcademyChildren();
void CityInfo_Population_changeHappiness(int amount);
void CityInfo_Population_setMaxHappiness(int amount);
void CityInfo_Population_changeHealthRate(int amount);
void CityInfo_Population_updateHealthRate();
void CityInfo_Population_calculateSentiment();
void CityInfo_Population_calculateMigrationSentiment();
void CityInfo_Population_requestYearlyUpdate();
void CityInfo_Population_yearlyUpdate();

void CityInfo_Population_addPeople(int numPeople);
void CityInfo_Population_addPeopleHomeless(int numPeople);

void CityInfo_Population_removePeopleHomeRemoved(int numPeople);
void CityInfo_Population_removePeopleHomeless(int numPeople);
void CityInfo_Population_removePeopleForTroopRequest(int amount);

void CityInfo_Labor_calculateWorkersNeededPerCategory();
void CityInfo_Labor_checkEmployment();
void CityInfo_Labor_allocateWorkersToCategories();
void CityInfo_Labor_allocateWorkersToBuildings();
void CityInfo_Labor_update();

void CityInfo_Resource_calculateAvailableResources();
void CityInfo_Resource_calculateFoodAndSupplyRomeWheat();
void CityInfo_Resource_calculateFood();
void CityInfo_Resource_housesConsumeFood();

void CityInfo_Culture_updateCoveragePercentages();
void CityInfo_Culture_calculateDemandsForAdvisors();
void CityInfo_Culture_calculateEntertainment();

void CityInfo_Victory_check();
void CityInfo_Victory_updateMonthsToGovern();

void CityInfo_Tick_countBuildingTypes();
void CityInfo_Tick_distributeTreasuryOverForumsAndSenates();

#endif
