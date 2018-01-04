#include "house_service.h"

#include "building/building.h"
#include "city/culture.h"

static void decay(unsigned char *value)
{
    if (*value > 0) {
        *value = *value - 1;
    } else {
        *value = 0;
    }
}

void house_service_decay_culture()
{
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE || !b->houseSize) {
            continue;
        }
        decay(&b->data.house.theater);
        decay(&b->data.house.amphitheaterActor);
        decay(&b->data.house.amphitheaterGladiator);
        decay(&b->data.house.colosseumGladiator);
        decay(&b->data.house.colosseumLion);
        decay(&b->data.house.hippodrome);
        decay(&b->data.house.school);
        decay(&b->data.house.library);
        decay(&b->data.house.academy);
        decay(&b->data.house.barber);
        decay(&b->data.house.clinic);
        decay(&b->data.house.bathhouse);
        decay(&b->data.house.hospital);
        decay(&b->data.house.templeCeres);
        decay(&b->data.house.templeNeptune);
        decay(&b->data.house.templeMercury);
        decay(&b->data.house.templeMars);
        decay(&b->data.house.templeVenus);
    }
}

void house_service_decay_tax_collector()
{
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->houseTaxCoverage) {
            b->houseTaxCoverage--;
        }
    }
}

void house_service_decay_houses_covered()
{
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_UNUSED && b->type != BUILDING_TOWER) {
            if (b->housesCovered <= 1) {
                b->housesCovered = 0;
            } else {
                b->housesCovered--;
            }
        }
    }
}

void house_service_calculate_culture_aggregates()
{
    int baseEntertainment = city_culture_coverage_average_entertainment() / 5;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE || !b->houseSize) {
            continue;
        }

        b->data.house.entertainment = 0;
        b->data.house.education = 0;
        b->data.house.health = 0;
        b->data.house.numGods = 0;

        // entertainment
        b->data.house.entertainment = baseEntertainment;
        if (b->data.house.theater) {
            b->data.house.entertainment += 10;
        }
        if (b->data.house.amphitheaterActor) {
            if (b->data.house.amphitheaterGladiator) {
                b->data.house.entertainment += 15;
            } else {
                b->data.house.entertainment += 10;
            }
        }
        if (b->data.house.colosseumGladiator) {
            if (b->data.house.colosseumLion) {
                b->data.house.entertainment += 25;
            } else {
                b->data.house.entertainment += 15;
            }
        }
        if (b->data.house.hippodrome) {
            b->data.house.entertainment += 30;
        }

        // education
        if (b->data.house.school || b->data.house.library) {
            b->data.house.education = 1;
            if (b->data.house.school && b->data.house.library) {
                b->data.house.education = 2;
                if (b->data.house.academy) {
                    b->data.house.education = 3;
                }
            }
        }

        // religion
        if (b->data.house.templeCeres) {
            ++b->data.house.numGods;
        }
        if (b->data.house.templeNeptune) {
            ++b->data.house.numGods;
        }
        if (b->data.house.templeMercury) {
            ++b->data.house.numGods;
        }
        if (b->data.house.templeMars) {
            ++b->data.house.numGods;
        }
        if (b->data.house.templeVenus) {
            ++b->data.house.numGods;
        }

        // health
        if (b->data.house.clinic) {
            ++b->data.house.health;
        }
        if (b->data.house.hospital) {
            ++b->data.house.health;
        }
    }
}
