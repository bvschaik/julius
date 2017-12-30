#ifndef CityInfo_H
#define CityInfo_H

void CityInfo_init();
void CityInfo_initGameTime();

void CityInfo_Imperial_calculateGiftCosts();

void CityInfo_Ratings_calculate(int isYearlyUpdate);
void CityInfo_Ratings_updateCultureExplanation();
void CityInfo_Ratings_updateFavorExplanation();
void CityInfo_Ratings_updateProsperityExplanation();
void CityInfo_Ratings_updatePeaceExplanation();
void CityInfo_Ratings_reduceProsperityAfterBailout();

void CityInfo_Ratings_sendGiftToCaesar();
void CityInfo_Ratings_changeFavor(int amount);
void CityInfo_Ratings_setMaxFavor(int maxFavor);

void CityInfo_Finance_calculateTotals();
void CityInfo_Finance_calculateEstimatedWages();
void CityInfo_Finance_calculateEstimatedTaxes();

void CityInfo_Finance_updateInterest();
void CityInfo_Finance_updateSalary();

void CityInfo_Finance_handleMonthChange();
void CityInfo_Finance_handleYearChange();

void CityInfo_Finance_updateDebtState();

void CityInfo_Resource_calculateAvailableResources();
void CityInfo_Resource_calculateFoodAndSupplyRomeWheat();
void CityInfo_Resource_calculateFood();
void CityInfo_Resource_housesConsumeFood();

void CityInfo_Victory_check();
void CityInfo_Victory_updateMonthsToGovern();

void CityInfo_Tick_distributeTreasuryOverForumsAndSenates();

#endif
