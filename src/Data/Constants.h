#ifndef DATA_CONSTANTS_H
#define DATA_CONSTANTS_H


enum {
	Advisor_None = 0,
	Advisor_Labor = 1,
	Advisor_Military = 2,
	Advisor_Imperial = 3,
	Advisor_Ratings = 4,
	Advisor_Trade = 5,
	Advisor_Population = 6,
	Advisor_Health = 7,
	Advisor_Education = 8,
	Advisor_Entertainment = 9,
	Advisor_Religion = 10,
	Advisor_Financial = 11,
	Advisor_Chief = 12
};

enum {
	LaborCategory_IndustryCommerce = 0,
	LaborCategory_FoodProduction = 1,
	LaborCategory_Engineering = 2,
	LaborCategory_Water = 3,
	LaborCategory_Prefectures = 4,
	LaborCategory_Military = 5,
	LaborCategory_Entertainment = 6,
	LaborCategory_HealthEducation = 7,
	LaborCategory_GovernanceReligion = 8
};

enum {
	TradeStatus_None = 0,
	TradeStatus_Import = 1,
	TradeStatus_Export = 2
};

enum {
	EmigrationCause_None = 0,
	EmigrationCause_NoFood = 1,
	EmigrationCause_NoJobs = 2,
	EmigrationCause_HighTaxes = 3,
	EmigrationCause_LowWages = 4,
	EmigrationCause_ManyTents = 5,
};

enum {
	Festival_None = 0,
	Festival_Small = 1,
	Festival_Large = 2,
	Festival_Grand = 3
};

extern const int Constant_SalaryForRank[11];

struct MissionId {
	int peaceful;
	int military;
};

extern const struct MissionId Constant_MissionIds[12];

#endif
