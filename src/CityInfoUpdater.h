#ifndef CITYINFOUPDATER_H
#define CITYINFOUPDATER_H

void CityInfoUpdater_Imperial_calculateGiftCosts();

void CityInfoUpdater_Gods_reset();
void CityInfoUpdater_Gods_calculate();
void CityInfoUpdater_Gods_calculateLeastHappy();

void CityInfoUpdater_Entertainment_calculate();

void CityInfoUpdater_Ratings_calculate(int isYearlyUpdate);
void CityInfoUpdater_Ratings_updateCultureExplanation();
void CityInfoUpdater_Ratings_updateFavorExplanation();
void CityInfoUpdater_Ratings_updateProsperityExplanation();
void CityInfoUpdater_Ratings_updatePeaceExplanation();

void CityInfoUpdater_Ratings_sendGiftToCaesar();
void CityInfoUpdater_Ratings_increaseFavor(int amount);
void CityInfoUpdater_Ratings_setMaxFavor(int maxFavor);

void CityInfoUpdater_Finance_calculateTotals();
void CityInfoUpdater_Finance_calculateEstimatedWages();
void CityInfoUpdater_Finance_calculateEstimatedTaxes();

void CityInfoUpdater_Finance_updateInterest();
void CityInfoUpdater_Finance_updateSalary();

void CityInfoUpdater_Finance_handleMonthChange();
void CityInfoUpdater_Finance_handleYearChange();

void CityInfoUpdater_Population_recordMonthlyPopulation();

void CityInfoUpdater_Labor_calculateWorkersNeededPerCategory();
void CityInfoUpdater_Labor_checkEmployment();
void CityInfoUpdater_Labor_allocateWorkersToCategories();
void CityInfoUpdater_Labor_allocateWorkersToBuildings();

#endif
