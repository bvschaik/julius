#include "CityInfoUpdater.h"
#include "Data/CityInfo.h"

void CityInfoUpdater_Finance_calculateTotals()
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
}

void CityInfoUpdater_Finance_calculateTribute()
{
	// NB the code in the game calculates tribute, and at the end of
	// the method, always sets it to zero... let's take a shortcut
	Data_CityInfo.financeTributeThisYear = 0;
}
