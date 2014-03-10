#include "CityInfo.h"
#include "Calc.h"

#include "Data/CityInfo.h"
#include "Data/Building.h"
#include "Data/Model.h"
#include "Data/Settings.h"

static void collectMonthlyTaxes()
{
	Data_CityInfo.monthlyTaxedPlebs = 0;
	Data_CityInfo.monthlyTaxedPatricians = 0;
	Data_CityInfo.monthlyUntaxedPlebs = 0;
	Data_CityInfo.monthlyUntaxedPatricians = 0;
	Data_CityInfo.monthlyUncollectedTaxFromPlebs = 0;
	Data_CityInfo.monthlyCollectedTaxFromPlebs = 0;
	Data_CityInfo.monthlyUncollectedTaxFromPatricians = 0;
	Data_CityInfo.monthlyCollectedTaxFromPatricians = 0;

	for (int i = 0; i < 20; i++) {
		Data_CityInfo.populationPerLevel[i] = 0;
	}
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (!Data_Buildings[i].inUse || !Data_Buildings[i].houseSize) {
			continue;
		}

		int isPatrician = Data_Buildings[i].subtype.houseLevel >= 12; // TODO housing level constants
		int population = Data_Buildings[i].housePopulation;
		int trm = Calc_adjustWithPercentage(
			Data_Model_Houses[Data_Buildings[i].subtype.houseLevel].taxMultiplier,
			Data_Model_Difficulty.moneyPercentage[Data_Settings.difficulty]);
		Data_CityInfo.populationPerLevel[Data_Buildings[i].subtype.houseLevel] += population;

		int tax = population * trm;
		if (Data_Buildings[i].houseTaxCoverage) {
			if (isPatrician) {
				Data_CityInfo.monthlyTaxedPatricians += population;
				Data_CityInfo.monthlyCollectedTaxFromPatricians += tax;
			} else {
				Data_CityInfo.monthlyTaxedPlebs += population;
				Data_CityInfo.monthlyCollectedTaxFromPlebs += tax;
			}
			Data_Buildings[i].taxIncomeOrStorage += tax;
		} else {
			if (isPatrician) {
				Data_CityInfo.monthlyUntaxedPatricians += population;
				Data_CityInfo.monthlyUncollectedTaxFromPatricians += tax;
			} else {
				Data_CityInfo.monthlyUntaxedPlebs += population;
				Data_CityInfo.monthlyUncollectedTaxFromPlebs += tax;
			}
		}
	}

	int collectedPatricians = Calc_adjustWithPercentage(
		Data_CityInfo.monthlyCollectedTaxFromPatricians / 2,
		Data_CityInfo.taxPercentage);
	int collectedPlebs = Calc_adjustWithPercentage(
		Data_CityInfo.monthlyCollectedTaxFromPlebs / 2,
		Data_CityInfo.taxPercentage);
	int collectedTotal = collectedPatricians + collectedPlebs;

	Data_CityInfo.yearlyCollectedTaxFromPatricians += collectedPatricians;
	Data_CityInfo.yearlyCollectedTaxFromPlebs += collectedPlebs;
	Data_CityInfo.yearlyUncollectedTaxFromPatricians += Calc_adjustWithPercentage(
		Data_CityInfo.monthlyCollectedTaxFromPatricians / 2,
		Data_CityInfo.taxPercentage);
	Data_CityInfo.yearlyUncollectedTaxFromPlebs += Calc_adjustWithPercentage(
		Data_CityInfo.monthlyCollectedTaxFromPlebs / 2,
		Data_CityInfo.taxPercentage);

	Data_CityInfo.treasury += collectedTotal;

	int totalPatricians = Data_CityInfo.monthlyTaxedPatricians + Data_CityInfo.monthlyUntaxedPatricians;
	int totalPlebs = Data_CityInfo.monthlyTaxedPlebs + Data_CityInfo.monthlyUntaxedPlebs;
	Data_CityInfo.percentageTaxedPatricians = Calc_getPercentage(Data_CityInfo.monthlyTaxedPatricians, totalPatricians);
	Data_CityInfo.percentageTaxedPlebs = Calc_getPercentage(Data_CityInfo.monthlyTaxedPlebs, totalPlebs);
	Data_CityInfo.percentageTaxedPeople = Calc_getPercentage(
		Data_CityInfo.monthlyTaxedPatricians + Data_CityInfo.monthlyTaxedPlebs,
		totalPatricians + totalPlebs);
}

static void payMonthlyWages()
{
	int wages = Data_CityInfo.wages * Data_CityInfo.workersEmployed / 10 / 12;
	Data_CityInfo.treasury -= wages;
	Data_CityInfo.financeWagesPaidThisYear += wages;
	Data_CityInfo.wageRatePaidThisYear += Data_CityInfo.wages;
}

static void payMonthlyInterest()
{
	if (Data_CityInfo.treasury < 0) {
		int interest = Calc_adjustWithPercentage(-Data_CityInfo.treasury, 10) / 12;
		Data_CityInfo.treasury -= interest;
		Data_CityInfo.financeInterestPaidThisYear += interest;
	}
}

static void payMonthlySalary()
{
	if (Data_CityInfo.treasury > -5000) {
		Data_CityInfo.financeSalaryPaidThisYear += Data_CityInfo.salaryAmount;
		Data_CityInfo.personalSavings += Data_CityInfo.salaryAmount;
		Data_CityInfo.treasury -= Data_CityInfo.salaryAmount;
	}
}

void CityInfo_Finance_handleMonthChange()
{
	collectMonthlyTaxes();
	payMonthlyWages();
	payMonthlyInterest();
	payMonthlySalary();
}

static void payTribute()
{
	int income =
		Data_CityInfo.financeDonatedLastYear +
		Data_CityInfo.financeTaxesLastYear +
		Data_CityInfo.financeExportsLastYear;
	int expenses =
		Data_CityInfo.financeSundriesLastYear +
		Data_CityInfo.financeSalaryLastYear +
		Data_CityInfo.financeInterestLastYear +
		Data_CityInfo.financeConstructionLastYear +
		Data_CityInfo.financeWagesLastYear +
		Data_CityInfo.financeImportsLastYear;

	Data_CityInfo.tributeNotPaidLastYear = 0;
	if (Data_CityInfo.treasury <= 0) {
		// city is in debt
		Data_CityInfo.tributeNotPaidLastYear = 1;
		Data_CityInfo.tributeNotPaidTotalYears++;
		Data_CityInfo.financeTributeLastYear = 0;
	} else if (income <= expenses) {
		// city made a loss: fixed tribute based on population
		Data_CityInfo.tributeNotPaidTotalYears = 0;
		if (Data_CityInfo.population > 2000) {
			Data_CityInfo.financeTributeLastYear = 200;
		} else if (Data_CityInfo.population > 1000) {
			Data_CityInfo.financeTributeLastYear = 100;
		} else {
			Data_CityInfo.financeTributeLastYear = 0;
		}
	} else {
		// city made a profit
		Data_CityInfo.tributeNotPaidTotalYears = 0;
		if (Data_CityInfo.population > 5000) {
			Data_CityInfo.financeTributeLastYear = 500;
		} else if (Data_CityInfo.population > 3000) {
			Data_CityInfo.financeTributeLastYear = 400;
		} else if (Data_CityInfo.population > 2000) {
			Data_CityInfo.financeTributeLastYear = 300;
		} else if (Data_CityInfo.population > 1000) {
			Data_CityInfo.financeTributeLastYear = 225;
		} else if (Data_CityInfo.population > 500) {
			Data_CityInfo.financeTributeLastYear = 150;
		} else {
			Data_CityInfo.financeTributeLastYear = 50;
		}
		int pctProfit = Calc_adjustWithPercentage(income - expenses, 25);
		if (pctProfit > Data_CityInfo.financeTributeLastYear) {
			Data_CityInfo.financeTributeLastYear = pctProfit;
		}
	}

	Data_CityInfo.treasury -= Data_CityInfo.financeTributeLastYear;
	Data_CityInfo.financeTributeThisYear = 0;

	Data_CityInfo.financeBalanceLastYear = Data_CityInfo.treasury;
	Data_CityInfo.financeTotalIncomeLastYear = income;
	Data_CityInfo.financeTotalExpensesLastYear = Data_CityInfo.financeTributeLastYear + expenses;
}

void CityInfo_Finance_handleYearChange()
{
	// taxes
	Data_CityInfo.financeTaxesLastYear =
		Data_CityInfo.yearlyCollectedTaxFromPlebs + Data_CityInfo.yearlyCollectedTaxFromPatricians;
	Data_CityInfo.yearlyCollectedTaxFromPlebs = 0;
	Data_CityInfo.yearlyCollectedTaxFromPatricians = 0;
	Data_CityInfo.yearlyUncollectedTaxFromPlebs = 0;
	Data_CityInfo.yearlyUncollectedTaxFromPatricians = 0;
	
	// reset tax income in building list
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].inUse && Data_Buildings[i].houseSize) {
			Data_Buildings[i].taxIncomeOrStorage = 0;
		}
	}

	// wages
	Data_CityInfo.financeWagesLastYear = Data_CityInfo.financeWagesPaidThisYear;
	Data_CityInfo.financeWagesPaidThisYear = 0;
	Data_CityInfo.wageRatePaidLastYear = Data_CityInfo.wageRatePaidThisYear;
	Data_CityInfo.wageRatePaidThisYear = 0;

	// import/export
	Data_CityInfo.financeExportsLastYear = Data_CityInfo.financeExportsThisYear;
	Data_CityInfo.financeExportsThisYear = 0;
	Data_CityInfo.financeImportsLastYear = Data_CityInfo.financeImportsThisYear;
	Data_CityInfo.financeImportsThisYear = 0;

	// construction
	Data_CityInfo.financeConstructionLastYear = Data_CityInfo.financeConstructionThisYear;
	Data_CityInfo.financeConstructionThisYear = 0;

	// interest
	Data_CityInfo.financeInterestLastYear = Data_CityInfo.financeInterestPaidThisYear;
	Data_CityInfo.financeInterestPaidThisYear = 0;

	// salary
	Data_CityInfo.financeSalaryLastYear = Data_CityInfo.financeSalaryPaidThisYear;
	Data_CityInfo.financeSalaryPaidThisYear = 0;

	// sundries
	Data_CityInfo.financeSundriesLastYear = Data_CityInfo.financeSundriesThisYear;
	Data_CityInfo.financeSundriesThisYear = 0;
	Data_CityInfo.financeStolenLastYear = Data_CityInfo.financeStolenThisYear;
	Data_CityInfo.financeStolenThisYear = 0;

	// donations
	Data_CityInfo.financeDonatedLastYear = Data_CityInfo.financeDonatedThisYear;
	Data_CityInfo.financeDonatedThisYear = 0;

	payTribute();
}

void CityInfo_Finance_calculateTotals()
{
	Data_CityInfo.financeTotalIncomeThisYear =
		Data_CityInfo.financeDonatedThisYear +
		Data_CityInfo.financeTaxesThisYear +
		Data_CityInfo.financeExportsThisYear;

	Data_CityInfo.financeTotalExpensesThisYear =
		Data_CityInfo.financeSundriesThisYear +
		Data_CityInfo.financeSalaryThisYear +
		Data_CityInfo.financeInterestThisYear +
		Data_CityInfo.financeConstructionThisYear +
		Data_CityInfo.financeWagesThisYear +
		Data_CityInfo.financeImportsThisYear;

	Data_CityInfo.financeNetInOutLastYear =
		Data_CityInfo.financeTotalIncomeLastYear +
		Data_CityInfo.financeTotalExpensesLastYear;
	Data_CityInfo.financeNetInOutThisYear =
		Data_CityInfo.financeTotalIncomeThisYear +
		Data_CityInfo.financeTotalExpensesThisYear;
	Data_CityInfo.financeBalanceThisYear =
		Data_CityInfo.financeNetInOutThisYear +
		Data_CityInfo.financeBalanceLastYear;

	// NB the code in the game calculates tribute, and at the end of
	// the method, always sets it to zero... let's take a shortcut
	Data_CityInfo.financeTributeThisYear = 0;
}

void CityInfo_Finance_calculateEstimatedWages()
{
	int monthlyWages = Data_CityInfo.wages *
		Data_CityInfo.workersEmployed / 10 / 12;
	Data_CityInfo.financeWagesThisYear = Data_CityInfo.financeWagesPaidThisYear;
	Data_CityInfo.estimatedYearlyWages =
		(11 - Data_CityInfo_Extra.gameTimeMonth + 1) * monthlyWages +
		Data_CityInfo.financeWagesPaidThisYear;
}

void CityInfo_Finance_calculateEstimatedTaxes()
{
	Data_CityInfo.monthlyCollectedTaxFromPlebs = 0;
	Data_CityInfo.monthlyCollectedTaxFromPatricians = 0;
	for (int i = 0; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].inUse && Data_Buildings[i].houseSize && Data_Buildings[i].houseTaxCoverage) {
			int isPatrician = Data_Buildings[i].subtype.houseLevel >= 12; // TODO housing level constants
			int trm = Calc_adjustWithPercentage(
				Data_Model_Houses[Data_Buildings[i].subtype.houseLevel].taxMultiplier,
				Data_Model_Difficulty.moneyPercentage[Data_Settings.difficulty]);
			if (isPatrician) {
				Data_CityInfo.monthlyCollectedTaxFromPatricians += Data_Buildings[i].housePopulation * trm;
			} else {
				Data_CityInfo.monthlyCollectedTaxFromPlebs += Data_Buildings[i].housePopulation * trm;
			}
		}
	}
	int monthlyPatricians = Calc_adjustWithPercentage(
		Data_CityInfo.monthlyCollectedTaxFromPatricians / 2,
		Data_CityInfo.taxPercentage);
	int monthlyPlebs = Calc_adjustWithPercentage(
		Data_CityInfo.monthlyCollectedTaxFromPlebs / 2,
		Data_CityInfo.taxPercentage);
	int estimatedRestOfYear = (12 - Data_CityInfo_Extra.gameTimeMonth) * (monthlyPatricians + monthlyPlebs);

	Data_CityInfo.financeTaxesThisYear =
		Data_CityInfo.yearlyCollectedTaxFromPlebs + Data_CityInfo.yearlyCollectedTaxFromPatricians;
	Data_CityInfo.estimatedTaxIncome = Data_CityInfo.financeTaxesThisYear + estimatedRestOfYear;
}

void CityInfo_Finance_updateInterest()
{
	Data_CityInfo.financeInterestThisYear = Data_CityInfo.financeInterestPaidThisYear;
}

void CityInfo_Finance_updateSalary()
{
	Data_CityInfo.financeSalaryThisYear = Data_CityInfo.financeSalaryPaidThisYear;
}
