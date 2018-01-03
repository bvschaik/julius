#include "HouseEvolution.h"

#include "Data/CityInfo.h"

#include "building/house.h"
#include "building/house_evolution.h"
#include "building/model.h"
#include "city/culture.h"
#include "game/resource.h"
#include "game/time.h"
#include "map/routing_terrain.h"

static void resetCityInfoServiceRequiredCounters();
static void consumeResources(building *b);

void HouseEvolution_Tick_evolveAndConsumeResources()
{
	resetCityInfoServiceRequiredCounters();
	int hasExpanded = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
		if (b->state == BUILDING_STATE_IN_USE && building_is_house(b->type)) {
			building_house_check_for_corruption(b);
			hasExpanded |= building_house_process_evolve(b);
			if (game_time_day() == 0 || game_time_day() == 7) {
				consumeResources(b);
			}
		}
	}
	if (hasExpanded) {
		map_routing_update_land();
	}
}

static void consume_resource(building *b, int inventory, int amount)
{
    if (amount > 0) {
        if (amount > b->data.house.inventory[inventory]) {
            b->data.house.inventory[inventory] = 0;
        } else {
            b->data.house.inventory[inventory] -= amount;
        }
    }
}

static void consumeResources(building *b)
{
    const model_house *model = model_get_house(b->subtype.houseLevel);
    consume_resource(b, INVENTORY_POTTERY, model->pottery);
    consume_resource(b, INVENTORY_FURNITURE, model->furniture);
    consume_resource(b, INVENTORY_OIL, model->oil);
    consume_resource(b, INVENTORY_WINE, model->wine);
}

static void resetCityInfoServiceRequiredCounters()
{
	Data_CityInfo.housesRequiringFountainToEvolve = 0;
	Data_CityInfo.housesRequiringWellToEvolve = 0;
	Data_CityInfo.housesRequiringEntertainmentToEvolve = 0;
	Data_CityInfo.housesRequiringMoreEntertainmentToEvolve = 0;
	Data_CityInfo.housesRequiringEducationToEvolve = 0;
	Data_CityInfo.housesRequiringMoreEducationToEvolve = 0;
	Data_CityInfo.housesRequiringReligionToEvolve = 0;
	Data_CityInfo.housesRequiringMoreReligionToEvolve = 0;
	Data_CityInfo.housesRequiringEvenMoreReligionToEvolve = 0;
	Data_CityInfo.housesRequiringBarberToEvolve = 0;
	Data_CityInfo.housesRequiringBathhouseToEvolve = 0;
	Data_CityInfo.housesRequiringClinicToEvolve = 0;
	Data_CityInfo.housesRequiringHospitalToEvolve = 0;
	Data_CityInfo.housesRequiringFoodToEvolve = 0;

	Data_CityInfo.housesRequiringSchool = 0;
	Data_CityInfo.housesRequiringLibrary = 0;
	Data_CityInfo.housesRequiringBarber = 0;
	Data_CityInfo.housesRequiringBathhouse = 0;
	Data_CityInfo.housesRequiringClinic = 0;
	Data_CityInfo.housesRequiringReligion = 0;
}
