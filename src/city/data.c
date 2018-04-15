#include "data.h"

#include "city/constants.h"
#include "city/data_private.h"
#include "city/gods.h"
#include "game/difficulty.h"
#include "scenario/property.h"

#include "Data/CityInfo.h"

#include <string.h>

void city_data_init()
{
    memset(&Data_CityInfo, 0, sizeof(struct _Data_CityInfo));

    city_data.unused.faction_bytes[0] = 0;
    city_data.unused.faction_bytes[1] = 0;

    Data_CityInfo.citySentiment = 60;
    Data_CityInfo.healthRateTarget = 50;
    Data_CityInfo.healthRate = 50;
    city_data.unused.unknown_00c0 = 3;
    Data_CityInfo.wagesRome = 30;
    Data_CityInfo.wages = 30;
    city_data.finance.tax_percentage = 7;
    Data_CityInfo.tradeNextImportResourceCaravan = 1;
    Data_CityInfo.tradeNextImportResourceCaravanBackup = 1;
    city_data.population.monthly.next_index = 0;
    city_data.population.monthly.count = 0;
    Data_CityInfo.monthsSinceFestival = 1;
    Data_CityInfo.festivalSize = FESTIVAL_SMALL;
    city_data.emperor.gifts[GIFT_MODEST].cost = 0;
    city_data.emperor.gifts[GIFT_GENEROUS].cost = 0;
    city_data.emperor.gifts[GIFT_LAVISH].cost = 0;

    city_gods_reset();
}

void city_data_init_scenario()
{
    Data_CityInfo_Extra.ciid = 1;
    city_data.unused.unknown_00a2 = 1;
    city_data.unused.unknown_00a3 = 1;
    city_data.finance.treasury = difficulty_adjust_money(scenario_initial_funds());
    city_data.finance.last_year.balance = city_data.finance.treasury;
}

void city_data_init_campaign_mission()
{
    city_data.finance.treasury = difficulty_adjust_money(city_data.finance.treasury);
}

static void save_main_data(buffer *main)
{
    buffer_write_raw(main, city_data.unused.other_player, 18068);
    buffer_write_i8(main, city_data.unused.unknown_00a0);
    buffer_write_i8(main, city_data.unused.unknown_00a1);
    buffer_write_i8(main, city_data.unused.unknown_00a2);
    buffer_write_i8(main, city_data.unused.unknown_00a3);
    buffer_write_i8(main, city_data.unused.unknown_00a4);
    buffer_write_i8(main, Data_CityInfo.__unknown_00a5);
    buffer_write_i8(main, city_data.unused.unknown_00a6);
    buffer_write_i8(main, city_data.unused.unknown_00a7);
    buffer_write_i32(main, city_data.finance.tax_percentage);
    buffer_write_i32(main, city_data.finance.treasury);
    buffer_write_i32(main, Data_CityInfo.citySentiment);
    buffer_write_i32(main, Data_CityInfo.healthRateTarget);
    buffer_write_i32(main, Data_CityInfo.healthRate);
    buffer_write_i32(main, Data_CityInfo.numHospitalWorkers);
    buffer_write_i32(main, city_data.unused.unknown_00c0);
    buffer_write_i32(main, city_data.population.population);
    buffer_write_i32(main, city_data.population.population_last_year);
    buffer_write_i32(main, city_data.population.school_age);
    buffer_write_i32(main, city_data.population.academy_age);
    buffer_write_i32(main, Data_CityInfo.populationMaxSupported);
    buffer_write_i32(main, Data_CityInfo.populationRoomInHouses);
    for (int i = 0; i < 2400; i++) {
        buffer_write_i32(main, city_data.population.monthly.values[i]);
    }
    buffer_write_i32(main, city_data.population.monthly.next_index);
    buffer_write_i32(main, city_data.population.monthly.count);
    for (int i = 0; i < 100; i++) {
        buffer_write_i16(main, city_data.population.at_age[i]);
    }
    for (int i = 0; i < 20; i++) {
        buffer_write_i32(main, city_data.population.at_level[i]);
    }
    buffer_write_i32(main, city_data.population.yearly_births);
    buffer_write_i32(main, city_data.population.yearly_deaths);
    buffer_write_i32(main, city_data.population.lost_removal);
    buffer_write_i32(main, Data_CityInfo.populationImmigrationAmountPerBatch);
    buffer_write_i32(main, Data_CityInfo.populationEmigrationAmountPerBatch);
    buffer_write_i32(main, Data_CityInfo.populationEmigrationQueueSize);
    buffer_write_i32(main, Data_CityInfo.populationImmigrationQueueSize);
    buffer_write_i32(main, city_data.population.lost_homeless);
    buffer_write_i32(main, city_data.population.last_change);
    buffer_write_i32(main, city_data.population.average_per_year);
    buffer_write_i32(main, city_data.population.total_all_years);
    buffer_write_i32(main, Data_CityInfo.populationPeopleInTentsShacks);
    buffer_write_i32(main, Data_CityInfo.populationPeopleInVillasPalaces);
    buffer_write_i32(main, city_data.population.total_years);
    buffer_write_i32(main, city_data.population.yearly_update_requested);
    buffer_write_i32(main, Data_CityInfo.populationLastTargetHouseAdd);
    buffer_write_i32(main, Data_CityInfo.populationLastTargetHouseRemove);
    buffer_write_i32(main, Data_CityInfo.populationImmigratedToday);
    buffer_write_i32(main, Data_CityInfo.populationEmigratedToday);
    buffer_write_i32(main, Data_CityInfo.populationRefusedImmigrantsNoRoom);
    buffer_write_i32(main, Data_CityInfo.populationMigrationPercentage);
    buffer_write_i32(main, city_data.unused.unused_27d0);
    buffer_write_i32(main, city_data.population.immigration_duration);
    buffer_write_i32(main, city_data.population.emigration_duration);
    buffer_write_i32(main, Data_CityInfo.populationNewcomersThisMonth);
    for (int i = 0; i < 4; i++) {
        buffer_write_i32(main, city_data.unused.unknown_27e0[i]);
    }
    buffer_write_i16(main, city_data.unused.unknown_27f0);
    buffer_write_i16(main, Data_CityInfo.resourceLastTargetWarehouse);
    for (int i = 0; i < 18; i++) {
        buffer_write_i16(main, city_data.unused.unknown_27f4[i]);
    }
    buffer_write_u8(main, Data_CityInfo.entryPointX);
    buffer_write_u8(main, Data_CityInfo.entryPointY);
    buffer_write_i16(main, Data_CityInfo.entryPointGridOffset);
    buffer_write_u8(main, Data_CityInfo.exitPointX);
    buffer_write_u8(main, Data_CityInfo.exitPointY);
    buffer_write_i16(main, Data_CityInfo.exitPointGridOffset);
    buffer_write_u8(main, city_data.building.senate_x);
    buffer_write_u8(main, city_data.building.senate_y);
    buffer_write_i16(main, city_data.building.senate_grid_offset);
    buffer_write_i32(main, city_data.building.senate_building_id);
    buffer_write_i16(main, city_data.unused.unknown_2828);
    for (int i = 0; i < RESOURCE_MAX; i++) {
        buffer_write_i16(main, city_data.resource.space_in_warehouses[i]);
    }
    for (int i = 0; i < RESOURCE_MAX; i++) {
        buffer_write_i16(main, city_data.resource.stored_in_warehouses[i]);
    }
    for (int i = 0; i < RESOURCE_MAX; i++) {
        buffer_write_i16(main, city_data.resource.trade_status[i]);
    }
    for (int i = 0; i < RESOURCE_MAX; i++) {
        buffer_write_i16(main, city_data.resource.export_over[i]);
    }
    for (int i = 0; i < RESOURCE_MAX; i++) {
        buffer_write_i16(main, city_data.resource.mothballed[i]);
    }
    buffer_write_i16(main, city_data.unused.unused_28ca);
    for (int i = 0; i < RESOURCE_MAX_FOOD; i++) {
        buffer_write_i32(main, city_data.resource.granary_food_stored[i]);
    }
    for (int i = 0; i < 6; i++) {
        buffer_write_i32(main, city_data.resource.stored_in_workshops[i]);
    }
    for (int i = 0; i < 6; i++) {
        buffer_write_i32(main, city_data.resource.space_in_workshops[i]);
    }
    buffer_write_i32(main, Data_CityInfo.foodInfoFoodStoredInGranaries);
    buffer_write_i32(main, Data_CityInfo.foodInfoFoodTypesAvailable);
    buffer_write_i32(main, Data_CityInfo.foodInfoFoodTypesEaten);
    for (int i = 0; i < 272; i++) {
        buffer_write_i8(main, city_data.unused.unknown_2924[i]);
    }
    for (int i = 0; i < RESOURCE_MAX; i++) {
        buffer_write_i32(main, city_data.resource.stockpiled[i]);
    }
    buffer_write_i32(main, Data_CityInfo.foodInfoFoodSupplyMonths);
    buffer_write_i32(main, Data_CityInfo.foodInfoGranariesOperating);
    buffer_write_i32(main, Data_CityInfo.populationPercentagePlebs);
    buffer_write_i32(main, Data_CityInfo.populationWorkingAge);
    buffer_write_i32(main, Data_CityInfo.workersAvailable);
    for (int i = 0; i < 10; i++) {
        buffer_write_i32(main, Data_CityInfo.laborCategory[i].workersNeeded);
        buffer_write_i32(main, Data_CityInfo.laborCategory[i].workersAllocated);
        buffer_write_i32(main, Data_CityInfo.laborCategory[i].totalHousesCovered);
        buffer_write_i32(main, Data_CityInfo.laborCategory[i].buildings);
        buffer_write_i32(main, Data_CityInfo.laborCategory[i].priority);
    }
    buffer_write_i32(main, Data_CityInfo.workersEmployed);
    buffer_write_i32(main, Data_CityInfo.workersUnemployed);
    buffer_write_i32(main, Data_CityInfo.unemploymentPercentage);
    buffer_write_i32(main, Data_CityInfo.unemploymentPercentageForSenate);
    buffer_write_i32(main, Data_CityInfo.workersNeeded);
    buffer_write_i32(main, Data_CityInfo.wages);
    buffer_write_i32(main, Data_CityInfo.wagesRome);
    buffer_write_i32(main, city_data.unused.unknown_2b6c);
    buffer_write_i32(main, city_data.finance.wages_so_far);
    buffer_write_i32(main, city_data.finance.this_year.expenses.wages);
    buffer_write_i32(main, city_data.finance.last_year.expenses.wages);
    buffer_write_i32(main, city_data.taxes.taxed_plebs);
    buffer_write_i32(main, city_data.taxes.taxed_patricians);
    buffer_write_i32(main, city_data.taxes.untaxed_plebs);
    buffer_write_i32(main, city_data.taxes.untaxed_patricians);
    buffer_write_i32(main, city_data.taxes.percentage_taxed_plebs);
    buffer_write_i32(main, city_data.taxes.percentage_taxed_patricians);
    buffer_write_i32(main, city_data.taxes.percentage_taxed_people);
    buffer_write_i32(main, city_data.taxes.yearly.collected_plebs);
    buffer_write_i32(main, city_data.taxes.yearly.collected_patricians);
    buffer_write_i32(main, city_data.taxes.yearly.uncollected_plebs);
    buffer_write_i32(main, city_data.taxes.yearly.uncollected_patricians);
    buffer_write_i32(main, city_data.finance.this_year.income.taxes);
    buffer_write_i32(main, city_data.finance.last_year.income.taxes);
    buffer_write_i32(main, city_data.taxes.monthly.collected_plebs);
    buffer_write_i32(main, city_data.taxes.monthly.uncollected_plebs);
    buffer_write_i32(main, city_data.taxes.monthly.collected_patricians);
    buffer_write_i32(main, city_data.taxes.monthly.uncollected_patricians);
    buffer_write_i32(main, city_data.finance.this_year.income.exports);
    buffer_write_i32(main, city_data.finance.last_year.income.exports);
    buffer_write_i32(main, city_data.finance.this_year.expenses.imports);
    buffer_write_i32(main, city_data.finance.last_year.expenses.imports);
    buffer_write_i32(main, city_data.finance.interest_so_far);
    buffer_write_i32(main, city_data.finance.last_year.expenses.interest);
    buffer_write_i32(main, city_data.finance.this_year.expenses.interest);
    buffer_write_i32(main, city_data.finance.last_year.expenses.sundries);
    buffer_write_i32(main, city_data.finance.this_year.expenses.sundries);
    buffer_write_i32(main, city_data.finance.last_year.expenses.construction);
    buffer_write_i32(main, city_data.finance.this_year.expenses.construction);
    buffer_write_i32(main, city_data.finance.last_year.expenses.salary);
    buffer_write_i32(main, city_data.finance.this_year.expenses.salary);
    buffer_write_i32(main, Data_CityInfo.salaryAmount);
    buffer_write_i32(main, Data_CityInfo.salaryRank);
    buffer_write_i32(main, city_data.finance.salary_so_far);
    buffer_write_i32(main, city_data.finance.last_year.income.total);
    buffer_write_i32(main, city_data.finance.this_year.income.total);
    buffer_write_i32(main, city_data.finance.last_year.expenses.total);
    buffer_write_i32(main, city_data.finance.this_year.expenses.total);
    buffer_write_i32(main, city_data.finance.last_year.net_in_out);
    buffer_write_i32(main, city_data.finance.this_year.net_in_out);
    buffer_write_i32(main, city_data.finance.last_year.balance);
    buffer_write_i32(main, city_data.finance.this_year.balance);
    for (int i = 0; i < 1400; i++) {
        buffer_write_i32(main, city_data.unused.unknown_2c20[i]);
    }
    for (int i = 0; i < 8; i++) {
        buffer_write_i32(main, city_data.unused.housesRequiringUnknownToEvolve[i]);
    }
    buffer_write_i32(main, Data_CityInfo.tradeNextImportResourceCaravan);
    buffer_write_i32(main, Data_CityInfo.tradeNextImportResourceCaravanBackup);
    buffer_write_i32(main, city_data.ratings.culture);
    buffer_write_i32(main, city_data.ratings.prosperity);
    buffer_write_i32(main, city_data.ratings.peace);
    buffer_write_i32(main, city_data.ratings.favor);
    for (int i = 0; i < 4; i++) {
        buffer_write_i32(main, city_data.unused.unknown_4238[i]);
    }
    buffer_write_i32(main, city_data.ratings.prosperity_treasury_last_year);
    buffer_write_i32(main, city_data.ratings.culture_points.theater);
    buffer_write_i32(main, city_data.ratings.culture_points.religion);
    buffer_write_i32(main, city_data.ratings.culture_points.school);
    buffer_write_i32(main, city_data.ratings.culture_points.library);
    buffer_write_i32(main, city_data.ratings.culture_points.academy);
    buffer_write_i32(main, city_data.ratings.peace_num_criminals);
    buffer_write_i32(main, city_data.ratings.peace_num_rioters);
    buffer_write_i32(main, Data_CityInfo.housesRequiringFountainToEvolve);
    buffer_write_i32(main, Data_CityInfo.housesRequiringWellToEvolve);
    buffer_write_i32(main, Data_CityInfo.housesRequiringMoreEntertainmentToEvolve);
    buffer_write_i32(main, Data_CityInfo.housesRequiringMoreEducationToEvolve);
    buffer_write_i32(main, Data_CityInfo.housesRequiringEducationToEvolve);
    buffer_write_i32(main, Data_CityInfo.housesRequiringSchool);
    buffer_write_i32(main, Data_CityInfo.housesRequiringLibrary);
    buffer_write_i32(main, city_data.unused.unknown_4284);
    buffer_write_i32(main, Data_CityInfo.housesRequiringBarberToEvolve);
    buffer_write_i32(main, Data_CityInfo.housesRequiringBathhouseToEvolve);
    buffer_write_i32(main, Data_CityInfo.housesRequiringFoodToEvolve);
    for (int i = 0; i < 2; i++) {
        buffer_write_i32(main, city_data.unused.unknown_4294[i]);
    }
    buffer_write_i32(main, city_data.building.hippodrome_placed);
    buffer_write_i32(main, Data_CityInfo.housesRequiringClinicToEvolve);
    buffer_write_i32(main, Data_CityInfo.housesRequiringHospitalToEvolve);
    buffer_write_i32(main, Data_CityInfo.housesRequiringBarber);
    buffer_write_i32(main, Data_CityInfo.housesRequiringBathhouse);
    buffer_write_i32(main, Data_CityInfo.housesRequiringClinic);
    buffer_write_i32(main, Data_CityInfo.housesRequiringReligionToEvolve);
    buffer_write_i32(main, Data_CityInfo.housesRequiringMoreReligionToEvolve);
    buffer_write_i32(main, Data_CityInfo.housesRequiringEvenMoreReligionToEvolve);
    buffer_write_i32(main, Data_CityInfo.housesRequiringReligion);
    buffer_write_i32(main, Data_CityInfo.entertainmentTheaterShows);
    buffer_write_i32(main, Data_CityInfo.entertainmentTheaterNoShowsWeighted);
    buffer_write_i32(main, Data_CityInfo.entertainmentAmphitheaterShows);
    buffer_write_i32(main, Data_CityInfo.entertainmentAmphitheaterNoShowsWeighted);
    buffer_write_i32(main, Data_CityInfo.entertainmentColosseumShows);
    buffer_write_i32(main, Data_CityInfo.entertainmentColosseumNoShowsWeighted);
    buffer_write_i32(main, Data_CityInfo.entertainmentHippodromeShows);
    buffer_write_i32(main, Data_CityInfo.entertainmentHippodromeNoShowsWeighted);
    buffer_write_i32(main, Data_CityInfo.entertainmentNeedingShowsMost);
    buffer_write_i32(main, Data_CityInfo.citywideAverageEntertainment);
    buffer_write_i32(main, Data_CityInfo.housesRequiringEntertainmentToEvolve);
    buffer_write_i32(main, Data_CityInfo.monthsSinceFestival);
    for (int i = 0; i < MAX_GODS; i++) {
        buffer_write_i8(main, city_data.religion.gods[i].target_happiness);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        buffer_write_i8(main, city_data.religion.gods[i].happiness);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        buffer_write_i8(main, Data_CityInfo.godWrathBolts[i]);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        buffer_write_i8(main, city_data.religion.gods[i].blessing_done);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        buffer_write_i8(main, city_data.religion.gods[i].small_curse_done);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        buffer_write_i8(main, city_data.religion.gods[i].unused1);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        buffer_write_i8(main, city_data.religion.gods[i].unused2);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        buffer_write_i8(main, city_data.religion.gods[i].unused3);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        buffer_write_i32(main, Data_CityInfo.godMonthsSinceFestival[i]);
    }
    buffer_write_i32(main, Data_CityInfo.godLeastHappy);
    buffer_write_i32(main, city_data.unused.unknown_4334);
    buffer_write_i32(main, Data_CityInfo.populationEmigrationCauseTextId);
    buffer_write_i32(main, Data_CityInfo.numProtestersThisMonth);
    buffer_write_i32(main, Data_CityInfo.numCriminalsThisMonth);
    buffer_write_i32(main, Data_CityInfo.healthDemand);
    buffer_write_i32(main, Data_CityInfo.religionDemand);
    buffer_write_i32(main, Data_CityInfo.educationDemand);
    buffer_write_i32(main, Data_CityInfo.entertainmentDemand);
    buffer_write_i32(main, Data_CityInfo.numRiotersInCity);
    buffer_write_i32(main, Data_CityInfo.ratingAdvisorSelection);
    buffer_write_i32(main, Data_CityInfo.ratingAdvisorExplanationCulture);
    buffer_write_i32(main, Data_CityInfo.ratingAdvisorExplanationProsperity);
    buffer_write_i32(main, Data_CityInfo.ratingAdvisorExplanationPeace);
    buffer_write_i32(main, Data_CityInfo.ratingAdvisorExplanationFavor);
    buffer_write_i32(main, Data_CityInfo.playerRank);
    buffer_write_i32(main, Data_CityInfo.personalSavings);
    for (int i = 0; i < 2; i++) {
        buffer_write_i32(main, city_data.unused.unknown_4374[i]);
    }
    buffer_write_i32(main, city_data.finance.last_year.income.donated);
    buffer_write_i32(main, city_data.finance.this_year.income.donated);
    buffer_write_i32(main, Data_CityInfo.donateAmount);
    for (int i = 0; i < 10; i++) {
        buffer_write_i16(main, Data_CityInfo.workingDockBuildingIds[i]);
    }
    for (int i = 0; i < 3; i++) {
        buffer_write_i16(main, city_data.unused.unknown_439c[i]);
    }
    buffer_write_i16(main, Data_CityInfo.numAnimalsInCity);
    buffer_write_i16(main, Data_CityInfo.tradeNumOpenSeaRoutes);
    buffer_write_i16(main, Data_CityInfo.tradeNumOpenLandRoutes);
    buffer_write_i16(main, Data_CityInfo.tradeSeaProblemDuration);
    buffer_write_i16(main, Data_CityInfo.tradeLandProblemDuration);
    buffer_write_i16(main, Data_CityInfo.numWorkingDocks);
    buffer_write_i16(main, city_data.building.senate_placed);
    buffer_write_i16(main, Data_CityInfo.numWorkingWharfs);
    for (int i = 0; i < 2; i++) {
        buffer_write_i8(main, city_data.unused.padding_43b2[i]);
    }
    buffer_write_i16(main, city_data.finance.stolen_this_year);
    buffer_write_i16(main, city_data.finance.stolen_last_year);
    buffer_write_i32(main, Data_CityInfo.tradeNextImportResourceDocker);
    buffer_write_i32(main, Data_CityInfo.tradeNextExportResourceDocker);
    buffer_write_i32(main, Data_CityInfo.debtState);
    buffer_write_i32(main, Data_CityInfo.monthsInDebt);
    buffer_write_i32(main, Data_CityInfo.cheatedMoney);
    buffer_write_i8(main, city_data.building.barracks_x);
    buffer_write_i8(main, city_data.building.barracks_y);
    buffer_write_i16(main, city_data.building.barracks_grid_offset);
    buffer_write_i32(main, city_data.building.barracks_building_id);
    buffer_write_i32(main, city_data.building.barracks_placed);
    for (int i = 0; i < 5; i++) {
        buffer_write_i32(main, city_data.unused.unknown_43d8[i]);
    }
    buffer_write_i32(main, Data_CityInfo.populationLostTroopRequest);
    buffer_write_i32(main, city_data.unused.unknown_43f0);
    buffer_write_i32(main, Data_CityInfo.victoryHasWonScenario);
    buffer_write_i32(main, Data_CityInfo.victoryContinueMonths);
    buffer_write_i32(main, Data_CityInfo.victoryContinueMonthsChosen);
    buffer_write_i32(main, Data_CityInfo.wageRatePaidThisYear);
    buffer_write_i32(main, city_data.finance.this_year.expenses.tribute);
    buffer_write_i32(main, city_data.finance.last_year.expenses.tribute);
    buffer_write_i32(main, Data_CityInfo.tributeNotPaidLastYear);
    buffer_write_i32(main, Data_CityInfo.tributeNotPaidTotalYears);
    buffer_write_i32(main, Data_CityInfo.festivalGod);
    buffer_write_i32(main, Data_CityInfo.festivalSize);
    buffer_write_i32(main, Data_CityInfo.plannedFestivalSize);
    buffer_write_i32(main, Data_CityInfo.plannedFestivalMonthsToGo);
    buffer_write_i32(main, Data_CityInfo.plannedFestivalGod);
    buffer_write_i32(main, Data_CityInfo.festivalCostSmall);
    buffer_write_i32(main, Data_CityInfo.festivalCostLarge);
    buffer_write_i32(main, Data_CityInfo.festivalCostGrand);
    buffer_write_i32(main, Data_CityInfo.festivalWineGrand);
    buffer_write_i32(main, Data_CityInfo.festivalNotEnoughWine);
    buffer_write_i32(main, Data_CityInfo.citywideAverageReligion);
    buffer_write_i32(main, Data_CityInfo.citywideAverageEducation);
    buffer_write_i32(main, Data_CityInfo.citywideAverageHealth);
    buffer_write_i32(main, Data_CityInfo.cultureCoverageReligion);
    buffer_write_i32(main, Data_CityInfo.festivalEffectMonthsDelayFirst);
    buffer_write_i32(main, Data_CityInfo.festivalEffectMonthsDelaySecond);
    buffer_write_i32(main, city_data.unused.unused_4454);
    buffer_write_i32(main, Data_CityInfo.populationSentimentUnemployment);
    buffer_write_i32(main, Data_CityInfo.citySentimentLastTime);
    buffer_write_i32(main, Data_CityInfo.citySentimentChangeMessageDelay);
    buffer_write_i32(main, Data_CityInfo.populationEmigrationCause);
    buffer_write_i32(main, Data_CityInfo.riotersOrAttackingNativesInCity);
    for (int i = 0; i < 4; i++) {
        buffer_write_i32(main, city_data.unused.unknown_446c[i]);
    }
    buffer_write_i32(main, city_data.emperor.selected_gift_size);
    buffer_write_i32(main, city_data.emperor.months_since_gift);
    buffer_write_i32(main, city_data.emperor.gift_overdose_penalty);
    buffer_write_i32(main, city_data.unused.unused_4488);
    buffer_write_i32(main, city_data.emperor.gifts[GIFT_MODEST].id);
    buffer_write_i32(main, city_data.emperor.gifts[GIFT_GENEROUS].id);
    buffer_write_i32(main, city_data.emperor.gifts[GIFT_LAVISH].id);
    buffer_write_i32(main, city_data.emperor.gifts[GIFT_MODEST].cost);
    buffer_write_i32(main, city_data.emperor.gifts[GIFT_GENEROUS].cost);
    buffer_write_i32(main, city_data.emperor.gifts[GIFT_LAVISH].cost);
    buffer_write_i32(main, city_data.ratings.favor_salary_penalty);
    buffer_write_i32(main, city_data.ratings.favor_milestone_penalty);
    buffer_write_i32(main, city_data.ratings.favor_ignored_request_penalty);
    buffer_write_i32(main, city_data.ratings.favor_last_year);
    buffer_write_i32(main, city_data.ratings.favor_change);
    buffer_write_i32(main, Data_CityInfo.nativeAttackDuration);
    buffer_write_i32(main, city_data.unused.unused_nativeForceAttack);
    buffer_write_i32(main, Data_CityInfo.nativeMissionPostOperational);
    buffer_write_i32(main, Data_CityInfo.nativeMainMeetingCenterX);
    buffer_write_i32(main, Data_CityInfo.nativeMainMeetingCenterY);
    buffer_write_i32(main, Data_CityInfo.wageRatePaidLastYear);
    buffer_write_i32(main, Data_CityInfo.foodInfoFoodNeededPerMonth);
    buffer_write_i32(main, Data_CityInfo.foodInfoGranariesUnderstaffed);
    buffer_write_i32(main, Data_CityInfo.foodInfoGranariesNotOperating);
    buffer_write_i32(main, Data_CityInfo.foodInfoGranariesNotOperatingWithFood);
    for (int i = 0; i < 2; i++) {
        buffer_write_i32(main, city_data.unused.unused_44e0[i]);
    }
    buffer_write_i32(main, Data_CityInfo.godCurseVenusActive);
    buffer_write_i32(main, city_data.unused.unused_44ec);
    buffer_write_i32(main, Data_CityInfo.godBlessingNeptuneDoubleTrade);
    buffer_write_i32(main, Data_CityInfo.godBlessingMarsEnemiesToKill);
    buffer_write_i32(main, city_data.unused.unused_44f8);
    buffer_write_i32(main, Data_CityInfo.godAngryMessageDelay);
    buffer_write_i32(main, Data_CityInfo.foodInfoFoodConsumedLastMonth);
    buffer_write_i32(main, Data_CityInfo.foodInfoFoodStoredLastMonth);
    buffer_write_i32(main, Data_CityInfo.foodInfoFoodStoredSoFarThisMonth);
    buffer_write_i32(main, city_data.ratings.peace_riot_cause);
    buffer_write_i32(main, Data_CityInfo.estimatedTaxIncome);
    buffer_write_i32(main, Data_CityInfo.tutorial1SenateBuilt);
    buffer_write_i8(main, city_data.building.distribution_center_x);
    buffer_write_i8(main, city_data.building.distribution_center_y);
    buffer_write_i16(main, city_data.building.distribution_center_grid_offset);
    buffer_write_i32(main, city_data.building.distribution_center_building_id);
    buffer_write_i32(main, city_data.building.distribution_center_placed);
    for (int i = 0; i < 11; i++) {
        buffer_write_i32(main, city_data.unused.unused_4524[i]);
    }
    buffer_write_i32(main, Data_CityInfo.shipyardBoatsRequested);
    buffer_write_i32(main, Data_CityInfo.numEnemiesInCity);
    buffer_write_i32(main, Data_CityInfo.populationSentimentWages);
    buffer_write_i32(main, Data_CityInfo.populationPeopleInTents);
    buffer_write_i32(main, Data_CityInfo.populationPeopleInLargeInsulaAndAbove);
    buffer_write_i32(main, Data_CityInfo.numImperialSoldiersInCity);
    buffer_write_i32(main, Data_CityInfo.caesarInvasionDurationDayCountdown);
    buffer_write_i32(main, Data_CityInfo.caesarInvasionWarningsGiven);
    buffer_write_i32(main, Data_CityInfo.caesarInvasionDaysUntilInvasion);
    buffer_write_i32(main, Data_CityInfo.caesarInvasionRetreatMessageShown);
    buffer_write_i32(main, city_data.ratings.peace_destroyed_buildings);
    buffer_write_i32(main, city_data.ratings.peace_years_of_peace);
    buffer_write_u8(main, Data_CityInfo.distantBattleCityId);
    buffer_write_u8(main, Data_CityInfo.distantBattleEnemyStrength);
    buffer_write_u8(main, Data_CityInfo.distantBattleRomanStrength);
    buffer_write_i8(main, Data_CityInfo.distantBattleMonthsToBattle);
    buffer_write_i8(main, Data_CityInfo.distantBattleRomanMonthsToReturn);
    buffer_write_i8(main, Data_CityInfo.distantBattleRomanMonthsToTravel);
    buffer_write_i8(main, Data_CityInfo.distantBattleCityMonthsUntilRoman);
    buffer_write_i8(main, Data_CityInfo.triumphalArchesAvailable);
    buffer_write_i8(main, Data_CityInfo.distantBattleTotalCount);
    buffer_write_i8(main, Data_CityInfo.distantBattleWonCount);
    buffer_write_i8(main, Data_CityInfo.distantBattleEnemyMonthsTraveled);
    buffer_write_i8(main, Data_CityInfo.distantBattleRomanMonthsTraveled);
    buffer_write_i8(main, Data_CityInfo.militaryTotalLegions);
    buffer_write_i8(main, Data_CityInfo.militaryTotalLegionsEmpireService);
    buffer_write_i8(main, city_data.unused.unknown_458e);
    buffer_write_i8(main, Data_CityInfo.militaryTotalSoldiers);
    buffer_write_i8(main, Data_CityInfo.triumphalArchesPlaced);
    buffer_write_i8(main, city_data.sound.die_citizen);
    buffer_write_i8(main, city_data.sound.die_soldier);
    buffer_write_i8(main, city_data.sound.shoot_arrow);
    buffer_write_i32(main, city_data.building.trade_center_building_id);
    buffer_write_i32(main, Data_CityInfo.numSoldiersInCity);
    buffer_write_i8(main, city_data.sound.hit_soldier);
    buffer_write_i8(main, city_data.sound.hit_spear);
    buffer_write_i8(main, city_data.sound.hit_club);
    buffer_write_i8(main, city_data.sound.march_enemy);
    buffer_write_i8(main, city_data.sound.march_horse);
    buffer_write_i8(main, city_data.sound.hit_elephant);
    buffer_write_i8(main, city_data.sound.hit_axe);
    buffer_write_i8(main, city_data.sound.hit_wolf);
    buffer_write_i8(main, city_data.sound.march_wolf);
    for (int i = 0; i < 6; i++) {
        buffer_write_i8(main, city_data.unused.unused_45a5[i]);
    }
    buffer_write_i8(main, Data_CityInfo.populationSentimentIncludeTents);
    buffer_write_i32(main, Data_CityInfo.caesarInvasionCount);
    buffer_write_i32(main, Data_CityInfo.caesarInvasionSize);
    buffer_write_i32(main, Data_CityInfo.caesarInvasionSoldiersDied);
    buffer_write_i32(main, Data_CityInfo.militaryLegionaryLegions);
    buffer_write_i32(main, Data_CityInfo.populationHighestEver);
    buffer_write_i32(main, Data_CityInfo.estimatedYearlyWages);
    buffer_write_i32(main, city_data.resource.wine_types_available);
    buffer_write_i32(main, city_data.ratings.prosperity_max);
    for (int i = 0; i < 10; i++) {
        buffer_write_i32(main, Data_CityInfo.largestRoadNetworks[i].id);
        buffer_write_i32(main, Data_CityInfo.largestRoadNetworks[i].size);
    }
    buffer_write_i32(main, Data_CityInfo.housesRequiringSecondWineToEvolve);
    buffer_write_i32(main, Data_CityInfo.godCurseNeptuneSankShips);
    buffer_write_i32(main, Data_CityInfo.entertainmentHippodromeHasShow);
    buffer_write_i32(main, Data_CityInfo.messageShownHippodrome);
    buffer_write_i32(main, Data_CityInfo.messageShownColosseum);
    buffer_write_i32(main, Data_CityInfo.messageShownEmigration);
    buffer_write_i32(main, Data_CityInfo.messageShownFired);
    buffer_write_i32(main, Data_CityInfo.messageShownVictory);
    buffer_write_i32(main, Data_CityInfo.missionSavedGameWritten);
    buffer_write_i32(main, Data_CityInfo.tutorial1FireMessageShown);
    buffer_write_i32(main, Data_CityInfo.tutorial3DiseaseMessageShown);
    buffer_write_i32(main, Data_CityInfo.numAttackingNativesInCity);
    for (int i = 0; i < 232; i++) {
        buffer_write_i8(main, city_data.unused.unknown_464c[i]);
    }
}

static void load_main_data(buffer *main)
{
    buffer_read_raw(main, city_data.unused.other_player, 18068);
    city_data.unused.unknown_00a0 = buffer_read_i8(main);
    city_data.unused.unknown_00a1 = buffer_read_i8(main);
    city_data.unused.unknown_00a2 = buffer_read_i8(main);
    city_data.unused.unknown_00a3 = buffer_read_i8(main);
    city_data.unused.unknown_00a4 = buffer_read_i8(main);
    Data_CityInfo.__unknown_00a5 = buffer_read_i8(main);
    city_data.unused.unknown_00a7 = buffer_read_i8(main);
    city_data.unused.unknown_00a6 = buffer_read_i8(main);
    city_data.finance.tax_percentage = buffer_read_i32(main);
    city_data.finance.treasury = buffer_read_i32(main);
    Data_CityInfo.citySentiment = buffer_read_i32(main);
    Data_CityInfo.healthRateTarget = buffer_read_i32(main);
    Data_CityInfo.healthRate = buffer_read_i32(main);
    Data_CityInfo.numHospitalWorkers = buffer_read_i32(main);
    city_data.unused.unknown_00c0 = buffer_read_i32(main);
    city_data.population.population = buffer_read_i32(main);
    city_data.population.population_last_year = buffer_read_i32(main);
    city_data.population.school_age = buffer_read_i32(main);
    city_data.population.academy_age = buffer_read_i32(main);
    Data_CityInfo.populationMaxSupported = buffer_read_i32(main);
    Data_CityInfo.populationRoomInHouses = buffer_read_i32(main);
    for (int i = 0; i < 2400; i++) {
        city_data.population.monthly.values[i] = buffer_read_i32(main);
    }
    city_data.population.monthly.next_index = buffer_read_i32(main);
    city_data.population.monthly.count = buffer_read_i32(main);
    for (int i = 0; i < 100; i++) {
        city_data.population.at_age[i] = buffer_read_i16(main);
    }
    for (int i = 0; i < 20; i++) {
        city_data.population.at_level[i] = buffer_read_i32(main);
    }
    city_data.population.yearly_births = buffer_read_i32(main);
    city_data.population.yearly_deaths = buffer_read_i32(main);
    city_data.population.lost_removal = buffer_read_i32(main);
    Data_CityInfo.populationImmigrationAmountPerBatch = buffer_read_i32(main);
    Data_CityInfo.populationEmigrationAmountPerBatch = buffer_read_i32(main);
    Data_CityInfo.populationEmigrationQueueSize = buffer_read_i32(main);
    Data_CityInfo.populationImmigrationQueueSize = buffer_read_i32(main);
    city_data.population.lost_homeless = buffer_read_i32(main);
    city_data.population.last_change = buffer_read_i32(main);
    city_data.population.average_per_year = buffer_read_i32(main);
    city_data.population.total_all_years = buffer_read_i32(main);
    Data_CityInfo.populationPeopleInTentsShacks = buffer_read_i32(main);
    Data_CityInfo.populationPeopleInVillasPalaces = buffer_read_i32(main);
    city_data.population.total_years = buffer_read_i32(main);
    city_data.population.yearly_update_requested = buffer_read_i32(main);
    Data_CityInfo.populationLastTargetHouseAdd = buffer_read_i32(main);
    Data_CityInfo.populationLastTargetHouseRemove = buffer_read_i32(main);
    Data_CityInfo.populationImmigratedToday = buffer_read_i32(main);
    Data_CityInfo.populationEmigratedToday = buffer_read_i32(main);
    Data_CityInfo.populationRefusedImmigrantsNoRoom = buffer_read_i32(main);
    Data_CityInfo.populationMigrationPercentage = buffer_read_i32(main);
    city_data.unused.unused_27d0 = buffer_read_i32(main);
    city_data.population.immigration_duration = buffer_read_i32(main);
    city_data.population.emigration_duration = buffer_read_i32(main);
    Data_CityInfo.populationNewcomersThisMonth = buffer_read_i32(main);
    for (int i = 0; i < 4; i++) {
        city_data.unused.unknown_27e0[i] = buffer_read_i32(main);
    }
    city_data.unused.unknown_27f0 = buffer_read_i16(main);
    Data_CityInfo.resourceLastTargetWarehouse = buffer_read_i16(main);
    for (int i = 0; i < 18; i++) {
        city_data.unused.unknown_27f4[i] = buffer_read_i16(main);
    }
    Data_CityInfo.entryPointX = buffer_read_u8(main);
    Data_CityInfo.entryPointY = buffer_read_u8(main);
    Data_CityInfo.entryPointGridOffset = buffer_read_i16(main);
    Data_CityInfo.exitPointX = buffer_read_u8(main);
    Data_CityInfo.exitPointY = buffer_read_u8(main);
    Data_CityInfo.exitPointGridOffset = buffer_read_i16(main);
    city_data.building.senate_x = buffer_read_u8(main);
    city_data.building.senate_y = buffer_read_u8(main);
    city_data.building.senate_grid_offset = buffer_read_i16(main);
    city_data.building.senate_building_id = buffer_read_i32(main);
    city_data.unused.unknown_2828 = buffer_read_i16(main);
    for (int i = 0; i < RESOURCE_MAX; i++) {
        city_data.resource.space_in_warehouses[i] = buffer_read_i16(main);
    }
    for (int i = 0; i < RESOURCE_MAX; i++) {
        city_data.resource.stored_in_warehouses[i] = buffer_read_i16(main);
    }
    for (int i = 0; i < RESOURCE_MAX; i++) {
        city_data.resource.trade_status[i] = buffer_read_i16(main);
    }
    for (int i = 0; i < RESOURCE_MAX; i++) {
        city_data.resource.export_over[i] = buffer_read_i16(main);
    }
    for (int i = 0; i < RESOURCE_MAX; i++) {
        city_data.resource.mothballed[i] = buffer_read_i16(main);
    }
    city_data.unused.unused_28ca = buffer_read_i16(main);
    for (int i = 0; i < RESOURCE_MAX_FOOD; i++) {
        city_data.resource.granary_food_stored[i] = buffer_read_i32(main);
    }
    for (int i = 0; i < 6; i++) {
        city_data.resource.stored_in_workshops[i] = buffer_read_i32(main);
    }
    for (int i = 0; i < 6; i++) {
        city_data.resource.space_in_workshops[i] = buffer_read_i32(main);
    }
    Data_CityInfo.foodInfoFoodStoredInGranaries = buffer_read_i32(main);
    Data_CityInfo.foodInfoFoodTypesAvailable = buffer_read_i32(main);
    Data_CityInfo.foodInfoFoodTypesEaten = buffer_read_i32(main);
    for (int i = 0; i < 272; i++) {
        city_data.unused.unknown_2924[i] = buffer_read_i8(main);
    }
    for (int i = 0; i < RESOURCE_MAX; i++) {
        city_data.resource.stockpiled[i] = buffer_read_i32(main);
    }
    Data_CityInfo.foodInfoFoodSupplyMonths = buffer_read_i32(main);
    Data_CityInfo.foodInfoGranariesOperating = buffer_read_i32(main);
    Data_CityInfo.populationPercentagePlebs = buffer_read_i32(main);
    Data_CityInfo.populationWorkingAge = buffer_read_i32(main);
    Data_CityInfo.workersAvailable = buffer_read_i32(main);
    for (int i = 0; i < 10; i++) {
        Data_CityInfo.laborCategory[i].workersNeeded = buffer_read_i32(main);
        Data_CityInfo.laborCategory[i].workersAllocated = buffer_read_i32(main);
        Data_CityInfo.laborCategory[i].totalHousesCovered = buffer_read_i32(main);
        Data_CityInfo.laborCategory[i].buildings = buffer_read_i32(main);
        Data_CityInfo.laborCategory[i].priority = buffer_read_i32(main);
    }
    Data_CityInfo.workersEmployed = buffer_read_i32(main);
    Data_CityInfo.workersUnemployed = buffer_read_i32(main);
    Data_CityInfo.unemploymentPercentage = buffer_read_i32(main);
    Data_CityInfo.unemploymentPercentageForSenate = buffer_read_i32(main);
    Data_CityInfo.workersNeeded = buffer_read_i32(main);
    Data_CityInfo.wages = buffer_read_i32(main);
    Data_CityInfo.wagesRome = buffer_read_i32(main);
    city_data.unused.unknown_2b6c = buffer_read_i32(main);
    city_data.finance.wages_so_far = buffer_read_i32(main);
    city_data.finance.this_year.expenses.wages = buffer_read_i32(main);
    city_data.finance.last_year.expenses.wages = buffer_read_i32(main);
    city_data.taxes.taxed_plebs = buffer_read_i32(main);
    city_data.taxes.taxed_patricians = buffer_read_i32(main);
    city_data.taxes.untaxed_plebs = buffer_read_i32(main);
    city_data.taxes.untaxed_patricians = buffer_read_i32(main);
    city_data.taxes.percentage_taxed_plebs = buffer_read_i32(main);
    city_data.taxes.percentage_taxed_patricians = buffer_read_i32(main);
    city_data.taxes.percentage_taxed_people = buffer_read_i32(main);
    city_data.taxes.yearly.collected_plebs = buffer_read_i32(main);
    city_data.taxes.yearly.collected_patricians = buffer_read_i32(main);
    city_data.taxes.yearly.uncollected_plebs = buffer_read_i32(main);
    city_data.taxes.yearly.uncollected_patricians = buffer_read_i32(main);
    city_data.finance.this_year.income.taxes = buffer_read_i32(main);
    city_data.finance.last_year.income.taxes = buffer_read_i32(main);
    city_data.taxes.monthly.collected_plebs = buffer_read_i32(main);
    city_data.taxes.monthly.uncollected_plebs = buffer_read_i32(main);
    city_data.taxes.monthly.collected_patricians = buffer_read_i32(main);
    city_data.taxes.monthly.uncollected_patricians = buffer_read_i32(main);
    city_data.finance.this_year.income.exports = buffer_read_i32(main);
    city_data.finance.last_year.income.exports = buffer_read_i32(main);
    city_data.finance.this_year.expenses.imports = buffer_read_i32(main);
    city_data.finance.last_year.expenses.imports = buffer_read_i32(main);
    city_data.finance.interest_so_far = buffer_read_i32(main);
    city_data.finance.last_year.expenses.interest = buffer_read_i32(main);
    city_data.finance.this_year.expenses.interest = buffer_read_i32(main);
    city_data.finance.last_year.expenses.sundries = buffer_read_i32(main);
    city_data.finance.this_year.expenses.sundries = buffer_read_i32(main);
    city_data.finance.last_year.expenses.construction = buffer_read_i32(main);
    city_data.finance.this_year.expenses.construction = buffer_read_i32(main);
    city_data.finance.last_year.expenses.salary = buffer_read_i32(main);
    city_data.finance.this_year.expenses.salary = buffer_read_i32(main);
    Data_CityInfo.salaryAmount = buffer_read_i32(main);
    Data_CityInfo.salaryRank = buffer_read_i32(main);
    city_data.finance.salary_so_far = buffer_read_i32(main);
    city_data.finance.last_year.income.total = buffer_read_i32(main);
    city_data.finance.this_year.income.total = buffer_read_i32(main);
    city_data.finance.last_year.expenses.total = buffer_read_i32(main);
    city_data.finance.this_year.expenses.total = buffer_read_i32(main);
    city_data.finance.last_year.net_in_out = buffer_read_i32(main);
    city_data.finance.this_year.net_in_out = buffer_read_i32(main);
    city_data.finance.last_year.balance = buffer_read_i32(main);
    city_data.finance.this_year.balance = buffer_read_i32(main);
    for (int i = 0; i < 1400; i++) {
        city_data.unused.unknown_2c20[i] = buffer_read_i32(main);
    }
    for (int i = 0; i < 8; i++) {
        city_data.unused.housesRequiringUnknownToEvolve[i] = buffer_read_i32(main);
    }
    Data_CityInfo.tradeNextImportResourceCaravan = buffer_read_i32(main);
    Data_CityInfo.tradeNextImportResourceCaravanBackup = buffer_read_i32(main);
    city_data.ratings.culture = buffer_read_i32(main);
    city_data.ratings.prosperity = buffer_read_i32(main);
    city_data.ratings.peace = buffer_read_i32(main);
    city_data.ratings.favor = buffer_read_i32(main);
    for (int i = 0; i < 4; i++) {
        city_data.unused.unknown_4238[i] = buffer_read_i32(main);
    }
    city_data.ratings.prosperity_treasury_last_year = buffer_read_i32(main);
    city_data.ratings.culture_points.theater = buffer_read_i32(main);
    city_data.ratings.culture_points.religion = buffer_read_i32(main);
    city_data.ratings.culture_points.school = buffer_read_i32(main);
    city_data.ratings.culture_points.library = buffer_read_i32(main);
    city_data.ratings.culture_points.academy = buffer_read_i32(main);
    city_data.ratings.peace_num_criminals = buffer_read_i32(main);
    city_data.ratings.peace_num_rioters = buffer_read_i32(main);
    Data_CityInfo.housesRequiringFountainToEvolve = buffer_read_i32(main);
    Data_CityInfo.housesRequiringWellToEvolve = buffer_read_i32(main);
    Data_CityInfo.housesRequiringMoreEntertainmentToEvolve = buffer_read_i32(main);
    Data_CityInfo.housesRequiringMoreEducationToEvolve = buffer_read_i32(main);
    Data_CityInfo.housesRequiringEducationToEvolve = buffer_read_i32(main);
    Data_CityInfo.housesRequiringSchool = buffer_read_i32(main);
    Data_CityInfo.housesRequiringLibrary = buffer_read_i32(main);
    city_data.unused.unknown_4284 = buffer_read_i32(main);
    Data_CityInfo.housesRequiringBarberToEvolve = buffer_read_i32(main);
    Data_CityInfo.housesRequiringBathhouseToEvolve = buffer_read_i32(main);
    Data_CityInfo.housesRequiringFoodToEvolve = buffer_read_i32(main);
    for (int i = 0; i < 2; i++) {
        city_data.unused.unknown_4294[i] = buffer_read_i32(main);
    }
    city_data.building.hippodrome_placed = buffer_read_i32(main);
    Data_CityInfo.housesRequiringClinicToEvolve = buffer_read_i32(main);
    Data_CityInfo.housesRequiringHospitalToEvolve = buffer_read_i32(main);
    Data_CityInfo.housesRequiringBarber = buffer_read_i32(main);
    Data_CityInfo.housesRequiringBathhouse = buffer_read_i32(main);
    Data_CityInfo.housesRequiringClinic = buffer_read_i32(main);
    Data_CityInfo.housesRequiringReligionToEvolve = buffer_read_i32(main);
    Data_CityInfo.housesRequiringMoreReligionToEvolve = buffer_read_i32(main);
    Data_CityInfo.housesRequiringEvenMoreReligionToEvolve = buffer_read_i32(main);
    Data_CityInfo.housesRequiringReligion = buffer_read_i32(main);
    Data_CityInfo.entertainmentTheaterShows = buffer_read_i32(main);
    Data_CityInfo.entertainmentTheaterNoShowsWeighted = buffer_read_i32(main);
    Data_CityInfo.entertainmentAmphitheaterShows = buffer_read_i32(main);
    Data_CityInfo.entertainmentAmphitheaterNoShowsWeighted = buffer_read_i32(main);
    Data_CityInfo.entertainmentColosseumShows = buffer_read_i32(main);
    Data_CityInfo.entertainmentColosseumNoShowsWeighted = buffer_read_i32(main);
    Data_CityInfo.entertainmentHippodromeShows = buffer_read_i32(main);
    Data_CityInfo.entertainmentHippodromeNoShowsWeighted = buffer_read_i32(main);
    Data_CityInfo.entertainmentNeedingShowsMost = buffer_read_i32(main);
    Data_CityInfo.citywideAverageEntertainment = buffer_read_i32(main);
    Data_CityInfo.housesRequiringEntertainmentToEvolve = buffer_read_i32(main);
    Data_CityInfo.monthsSinceFestival = buffer_read_i32(main);
    for (int i = 0; i < MAX_GODS; i++) {
        city_data.religion.gods[i].target_happiness = buffer_read_i8(main);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        city_data.religion.gods[i].happiness = buffer_read_i8(main);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        Data_CityInfo.godWrathBolts[i] = buffer_read_i8(main);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        city_data.religion.gods[i].blessing_done = buffer_read_i8(main);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        city_data.religion.gods[i].small_curse_done = buffer_read_i8(main);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        city_data.religion.gods[i].unused1 = buffer_read_i8(main);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        city_data.religion.gods[i].unused2 = buffer_read_i8(main);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        city_data.religion.gods[i].unused3 = buffer_read_i8(main);
    }
    for (int i = 0; i < MAX_GODS; i++) {
        Data_CityInfo.godMonthsSinceFestival[i] = buffer_read_i32(main);
    }
    Data_CityInfo.godLeastHappy = buffer_read_i32(main);
    city_data.unused.unknown_4334 = buffer_read_i32(main);
    Data_CityInfo.populationEmigrationCauseTextId = buffer_read_i32(main);
    Data_CityInfo.numProtestersThisMonth = buffer_read_i32(main);
    Data_CityInfo.numCriminalsThisMonth = buffer_read_i32(main);
    Data_CityInfo.healthDemand = buffer_read_i32(main);
    Data_CityInfo.religionDemand = buffer_read_i32(main);
    Data_CityInfo.educationDemand = buffer_read_i32(main);
    Data_CityInfo.entertainmentDemand = buffer_read_i32(main);
    Data_CityInfo.numRiotersInCity = buffer_read_i32(main);
    Data_CityInfo.ratingAdvisorSelection = buffer_read_i32(main);
    Data_CityInfo.ratingAdvisorExplanationCulture = buffer_read_i32(main);
    Data_CityInfo.ratingAdvisorExplanationProsperity = buffer_read_i32(main);
    Data_CityInfo.ratingAdvisorExplanationPeace = buffer_read_i32(main);
    Data_CityInfo.ratingAdvisorExplanationFavor = buffer_read_i32(main);
    Data_CityInfo.playerRank = buffer_read_i32(main);
    Data_CityInfo.personalSavings = buffer_read_i32(main);
    for (int i = 0; i < 2; i++) {
        city_data.unused.unknown_4374[i] = buffer_read_i32(main);
    }
    city_data.finance.last_year.income.donated = buffer_read_i32(main);
    city_data.finance.this_year.income.donated = buffer_read_i32(main);
    Data_CityInfo.donateAmount = buffer_read_i32(main);
    for (int i = 0; i < 10; i++) {
        Data_CityInfo.workingDockBuildingIds[i] = buffer_read_i16(main);
    }
    for (int i = 0; i < 3; i++) {
        city_data.unused.unknown_439c[i] = buffer_read_i16(main);
    }
    Data_CityInfo.numAnimalsInCity = buffer_read_i16(main);
    Data_CityInfo.tradeNumOpenSeaRoutes = buffer_read_i16(main);
    Data_CityInfo.tradeNumOpenLandRoutes = buffer_read_i16(main);
    Data_CityInfo.tradeSeaProblemDuration = buffer_read_i16(main);
    Data_CityInfo.tradeLandProblemDuration = buffer_read_i16(main);
    Data_CityInfo.numWorkingDocks = buffer_read_i16(main);
    city_data.building.senate_placed = buffer_read_i16(main);
    Data_CityInfo.numWorkingWharfs = buffer_read_i16(main);
    for (int i = 0; i < 2; i++) {
        city_data.unused.padding_43b2[i] = buffer_read_i8(main);
    }
    city_data.finance.stolen_this_year = buffer_read_i16(main);
    city_data.finance.stolen_last_year = buffer_read_i16(main);
    Data_CityInfo.tradeNextImportResourceDocker = buffer_read_i32(main);
    Data_CityInfo.tradeNextExportResourceDocker = buffer_read_i32(main);
    Data_CityInfo.debtState = buffer_read_i32(main);
    Data_CityInfo.monthsInDebt = buffer_read_i32(main);
    Data_CityInfo.cheatedMoney = buffer_read_i32(main);
    city_data.building.barracks_x = buffer_read_i8(main);
    city_data.building.barracks_y = buffer_read_i8(main);
    city_data.building.barracks_grid_offset = buffer_read_i16(main);
    city_data.building.barracks_building_id = buffer_read_i32(main);
    city_data.building.barracks_placed = buffer_read_i32(main);
    for (int i = 0; i < 5; i++) {
        city_data.unused.unknown_43d8[i] = buffer_read_i32(main);
    }
    Data_CityInfo.populationLostTroopRequest = buffer_read_i32(main);
    city_data.unused.unknown_43f0 = buffer_read_i32(main);
    Data_CityInfo.victoryHasWonScenario = buffer_read_i32(main);
    Data_CityInfo.victoryContinueMonths = buffer_read_i32(main);
    Data_CityInfo.victoryContinueMonthsChosen = buffer_read_i32(main);
    Data_CityInfo.wageRatePaidThisYear = buffer_read_i32(main);
    city_data.finance.this_year.expenses.tribute = buffer_read_i32(main);
    city_data.finance.last_year.expenses.tribute = buffer_read_i32(main);
    Data_CityInfo.tributeNotPaidLastYear = buffer_read_i32(main);
    Data_CityInfo.tributeNotPaidTotalYears = buffer_read_i32(main);
    Data_CityInfo.festivalGod = buffer_read_i32(main);
    Data_CityInfo.festivalSize = buffer_read_i32(main);
    Data_CityInfo.plannedFestivalSize = buffer_read_i32(main);
    Data_CityInfo.plannedFestivalMonthsToGo = buffer_read_i32(main);
    Data_CityInfo.plannedFestivalGod = buffer_read_i32(main);
    Data_CityInfo.festivalCostSmall = buffer_read_i32(main);
    Data_CityInfo.festivalCostLarge = buffer_read_i32(main);
    Data_CityInfo.festivalCostGrand = buffer_read_i32(main);
    Data_CityInfo.festivalWineGrand = buffer_read_i32(main);
    Data_CityInfo.festivalNotEnoughWine = buffer_read_i32(main);
    Data_CityInfo.citywideAverageReligion = buffer_read_i32(main);
    Data_CityInfo.citywideAverageEducation = buffer_read_i32(main);
    Data_CityInfo.citywideAverageHealth = buffer_read_i32(main);
    Data_CityInfo.cultureCoverageReligion = buffer_read_i32(main);
    Data_CityInfo.festivalEffectMonthsDelayFirst = buffer_read_i32(main);
    Data_CityInfo.festivalEffectMonthsDelaySecond = buffer_read_i32(main);
    city_data.unused.unused_4454 = buffer_read_i32(main);
    Data_CityInfo.populationSentimentUnemployment = buffer_read_i32(main);
    Data_CityInfo.citySentimentLastTime = buffer_read_i32(main);
    Data_CityInfo.citySentimentChangeMessageDelay = buffer_read_i32(main);
    Data_CityInfo.populationEmigrationCause = buffer_read_i32(main);
    Data_CityInfo.riotersOrAttackingNativesInCity = buffer_read_i32(main);
    for (int i = 0; i < 4; i++) {
        city_data.unused.unknown_446c[i] = buffer_read_i32(main);
    }
    city_data.emperor.selected_gift_size = buffer_read_i32(main);
    city_data.emperor.months_since_gift = buffer_read_i32(main);
    city_data.emperor.gift_overdose_penalty = buffer_read_i32(main);
    city_data.unused.unused_4488 = buffer_read_i32(main);
    city_data.emperor.gifts[GIFT_MODEST].id = buffer_read_i32(main);
    city_data.emperor.gifts[GIFT_GENEROUS].id = buffer_read_i32(main);
    city_data.emperor.gifts[GIFT_LAVISH].id = buffer_read_i32(main);
    city_data.emperor.gifts[GIFT_MODEST].cost = buffer_read_i32(main);
    city_data.emperor.gifts[GIFT_GENEROUS].cost = buffer_read_i32(main);
    city_data.emperor.gifts[GIFT_LAVISH].cost = buffer_read_i32(main);
    city_data.ratings.favor_salary_penalty = buffer_read_i32(main);
    city_data.ratings.favor_milestone_penalty = buffer_read_i32(main);
    city_data.ratings.favor_ignored_request_penalty = buffer_read_i32(main);
    city_data.ratings.favor_last_year = buffer_read_i32(main);
    city_data.ratings.favor_change = buffer_read_i32(main);
    Data_CityInfo.nativeAttackDuration = buffer_read_i32(main);
    city_data.unused.unused_nativeForceAttack = buffer_read_i32(main);
    Data_CityInfo.nativeMissionPostOperational = buffer_read_i32(main);
    Data_CityInfo.nativeMainMeetingCenterX = buffer_read_i32(main);
    Data_CityInfo.nativeMainMeetingCenterY = buffer_read_i32(main);
    Data_CityInfo.wageRatePaidLastYear = buffer_read_i32(main);
    Data_CityInfo.foodInfoFoodNeededPerMonth = buffer_read_i32(main);
    Data_CityInfo.foodInfoGranariesUnderstaffed = buffer_read_i32(main);
    Data_CityInfo.foodInfoGranariesNotOperating = buffer_read_i32(main);
    Data_CityInfo.foodInfoGranariesNotOperatingWithFood = buffer_read_i32(main);
    for (int i = 0; i < 2; i++) {
        city_data.unused.unused_44e0[i] = buffer_read_i32(main);
    }
    Data_CityInfo.godCurseVenusActive = buffer_read_i32(main);
    city_data.unused.unused_44ec = buffer_read_i32(main);
    Data_CityInfo.godBlessingNeptuneDoubleTrade = buffer_read_i32(main);
    Data_CityInfo.godBlessingMarsEnemiesToKill = buffer_read_i32(main);
    city_data.unused.unused_44f8 = buffer_read_i32(main);
    Data_CityInfo.godAngryMessageDelay = buffer_read_i32(main);
    Data_CityInfo.foodInfoFoodConsumedLastMonth = buffer_read_i32(main);
    Data_CityInfo.foodInfoFoodStoredLastMonth = buffer_read_i32(main);
    Data_CityInfo.foodInfoFoodStoredSoFarThisMonth = buffer_read_i32(main);
    city_data.ratings.peace_riot_cause = buffer_read_i32(main);
    Data_CityInfo.estimatedTaxIncome = buffer_read_i32(main);
    Data_CityInfo.tutorial1SenateBuilt = buffer_read_i32(main);
    city_data.building.distribution_center_x = buffer_read_i8(main);
    city_data.building.distribution_center_y = buffer_read_i8(main);
    city_data.building.distribution_center_grid_offset = buffer_read_i16(main);
    city_data.building.distribution_center_building_id = buffer_read_i32(main);
    city_data.building.distribution_center_placed = buffer_read_i32(main);
    for (int i = 0; i < 11; i++) {
        city_data.unused.unused_4524[i] = buffer_read_i32(main);
    }
    Data_CityInfo.shipyardBoatsRequested = buffer_read_i32(main);
    Data_CityInfo.numEnemiesInCity = buffer_read_i32(main);
    Data_CityInfo.populationSentimentWages = buffer_read_i32(main);
    Data_CityInfo.populationPeopleInTents = buffer_read_i32(main);
    Data_CityInfo.populationPeopleInLargeInsulaAndAbove = buffer_read_i32(main);
    Data_CityInfo.numImperialSoldiersInCity = buffer_read_i32(main);
    Data_CityInfo.caesarInvasionDurationDayCountdown = buffer_read_i32(main);
    Data_CityInfo.caesarInvasionWarningsGiven = buffer_read_i32(main);
    Data_CityInfo.caesarInvasionDaysUntilInvasion = buffer_read_i32(main);
    Data_CityInfo.caesarInvasionRetreatMessageShown = buffer_read_i32(main);
    city_data.ratings.peace_destroyed_buildings = buffer_read_i32(main);
    city_data.ratings.peace_years_of_peace = buffer_read_i32(main);
    Data_CityInfo.distantBattleCityId = buffer_read_u8(main);
    Data_CityInfo.distantBattleEnemyStrength = buffer_read_u8(main);
    Data_CityInfo.distantBattleRomanStrength = buffer_read_u8(main);
    Data_CityInfo.distantBattleMonthsToBattle = buffer_read_i8(main);
    Data_CityInfo.distantBattleRomanMonthsToReturn = buffer_read_i8(main);
    Data_CityInfo.distantBattleRomanMonthsToTravel = buffer_read_i8(main);
    Data_CityInfo.distantBattleCityMonthsUntilRoman = buffer_read_i8(main);
    Data_CityInfo.triumphalArchesAvailable = buffer_read_i8(main);
    Data_CityInfo.distantBattleTotalCount = buffer_read_i8(main);
    Data_CityInfo.distantBattleWonCount = buffer_read_i8(main);
    Data_CityInfo.distantBattleEnemyMonthsTraveled = buffer_read_i8(main);
    Data_CityInfo.distantBattleRomanMonthsTraveled = buffer_read_i8(main);
    Data_CityInfo.militaryTotalLegions = buffer_read_i8(main);
    Data_CityInfo.militaryTotalLegionsEmpireService = buffer_read_i8(main);
    city_data.unused.unknown_458e = buffer_read_i8(main);
    Data_CityInfo.militaryTotalSoldiers = buffer_read_i8(main);
    Data_CityInfo.triumphalArchesPlaced = buffer_read_i8(main);
    city_data.sound.die_citizen = buffer_read_i8(main);
    city_data.sound.die_soldier = buffer_read_i8(main);
    city_data.sound.shoot_arrow = buffer_read_i8(main);
    city_data.building.trade_center_building_id = buffer_read_i32(main);
    Data_CityInfo.numSoldiersInCity = buffer_read_i32(main);
    city_data.sound.hit_soldier = buffer_read_i8(main);
    city_data.sound.hit_spear = buffer_read_i8(main);
    city_data.sound.hit_club = buffer_read_i8(main);
    city_data.sound.march_enemy = buffer_read_i8(main);
    city_data.sound.march_horse = buffer_read_i8(main);
    city_data.sound.hit_elephant = buffer_read_i8(main);
    city_data.sound.hit_axe = buffer_read_i8(main);
    city_data.sound.hit_wolf = buffer_read_i8(main);
    city_data.sound.march_wolf = buffer_read_i8(main);
    for (int i = 0; i < 6; i++) {
        city_data.unused.unused_45a5[i] = buffer_read_i8(main);
    }
    Data_CityInfo.populationSentimentIncludeTents = buffer_read_i8(main);
    Data_CityInfo.caesarInvasionCount = buffer_read_i32(main);
    Data_CityInfo.caesarInvasionSize = buffer_read_i32(main);
    Data_CityInfo.caesarInvasionSoldiersDied = buffer_read_i32(main);
    Data_CityInfo.militaryLegionaryLegions = buffer_read_i32(main);
    Data_CityInfo.populationHighestEver = buffer_read_i32(main);
    Data_CityInfo.estimatedYearlyWages = buffer_read_i32(main);
    city_data.resource.wine_types_available = buffer_read_i32(main);
    city_data.ratings.prosperity_max = buffer_read_i32(main);
    for (int i = 0; i < 10; i++) {
        Data_CityInfo.largestRoadNetworks[i].id = buffer_read_i32(main);
        Data_CityInfo.largestRoadNetworks[i].size = buffer_read_i32(main);
    }
    Data_CityInfo.housesRequiringSecondWineToEvolve = buffer_read_i32(main);
    Data_CityInfo.godCurseNeptuneSankShips = buffer_read_i32(main);
    Data_CityInfo.entertainmentHippodromeHasShow = buffer_read_i32(main);
    Data_CityInfo.messageShownHippodrome = buffer_read_i32(main);
    Data_CityInfo.messageShownColosseum = buffer_read_i32(main);
    Data_CityInfo.messageShownEmigration = buffer_read_i32(main);
    Data_CityInfo.messageShownFired = buffer_read_i32(main);
    Data_CityInfo.messageShownVictory = buffer_read_i32(main);
    Data_CityInfo.missionSavedGameWritten = buffer_read_i32(main);
    Data_CityInfo.tutorial1FireMessageShown = buffer_read_i32(main);
    Data_CityInfo.tutorial3DiseaseMessageShown = buffer_read_i32(main);
    Data_CityInfo.numAttackingNativesInCity = buffer_read_i32(main);
    for (int i = 0; i < 232; i++) {
        city_data.unused.unknown_464c[i] = buffer_read_i8(main);
    }
}

static void save_entry_exit(buffer *entry_exit_xy, buffer *entry_exit_grid_offset)
{
    buffer_write_i32(entry_exit_xy, Data_CityInfo_Extra.entryPointFlag.x);
    buffer_write_i32(entry_exit_xy, Data_CityInfo_Extra.entryPointFlag.y);
    buffer_write_i32(entry_exit_xy, Data_CityInfo_Extra.exitPointFlag.x);
    buffer_write_i32(entry_exit_xy, Data_CityInfo_Extra.exitPointFlag.y);
    
    buffer_write_i32(entry_exit_grid_offset, Data_CityInfo_Extra.entryPointFlag.gridOffset);
    buffer_write_i32(entry_exit_grid_offset, Data_CityInfo_Extra.exitPointFlag.gridOffset);
}

static void load_entry_exit(buffer *entry_exit_xy, buffer *entry_exit_grid_offset)
{
    Data_CityInfo_Extra.entryPointFlag.x = buffer_read_i32(entry_exit_xy);
    Data_CityInfo_Extra.entryPointFlag.y = buffer_read_i32(entry_exit_xy);
    Data_CityInfo_Extra.exitPointFlag.x = buffer_read_i32(entry_exit_xy);
    Data_CityInfo_Extra.exitPointFlag.y = buffer_read_i32(entry_exit_xy);
    
    Data_CityInfo_Extra.entryPointFlag.gridOffset = buffer_read_i32(entry_exit_grid_offset);
    Data_CityInfo_Extra.exitPointFlag.gridOffset = buffer_read_i32(entry_exit_grid_offset);
}

void city_data_save_state(buffer *main, buffer *faction, buffer *faction_unknown, buffer *graph_order,
                          buffer *entry_exit_xy, buffer *entry_exit_grid_offset)
{
    save_main_data(main);
    
    buffer_write_i32(faction, Data_CityInfo_Extra.ciid);
    buffer_write_i8(faction_unknown, city_data.unused.faction_bytes[0]);
    buffer_write_i8(faction_unknown, city_data.unused.faction_bytes[1]);
    buffer_write_i32(graph_order, Data_CityInfo_Extra.populationGraphOrder);
    buffer_write_i32(graph_order, city_data.unused.unknown_order);

    save_entry_exit(entry_exit_xy, entry_exit_grid_offset);
}

void city_data_load_state(buffer *main, buffer *faction, buffer *faction_unknown, buffer *graph_order,
                          buffer *entry_exit_xy, buffer *entry_exit_grid_offset)
{
    load_main_data(main);

    Data_CityInfo_Extra.ciid = buffer_read_i32(faction);
    city_data.unused.faction_bytes[0] = buffer_read_i8(faction_unknown);
    city_data.unused.faction_bytes[1] = buffer_read_i8(faction_unknown);
    Data_CityInfo_Extra.populationGraphOrder = buffer_read_i32(graph_order);
    city_data.unused.unknown_order = buffer_read_i32(graph_order);

    load_entry_exit(entry_exit_xy, entry_exit_grid_offset);
}
