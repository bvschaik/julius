#include "house_service.h"

#include "building/building.h"
#include "building/monument.h"
#include "city/culture.h"

static void decay(unsigned char *value)
{
    if (*value > 0) {
        *value = *value - 1;
    } else {
        *value = 0;
    }
}

void house_service_decay_culture(void)
{
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state != BUILDING_STATE_IN_USE || !b->house_size) {
                continue;
            }
            decay(&b->data.house.theater);
            decay(&b->data.house.amphitheater_actor);
            decay(&b->data.house.amphitheater_gladiator);
            decay(&b->data.house.colosseum_gladiator);
            decay(&b->data.house.colosseum_lion);
            decay(&b->house_arena_gladiator);
            decay(&b->house_arena_lion);
            decay(&b->house_tavern_meat_access);
            decay(&b->house_tavern_wine_access);
            decay(&b->data.house.hippodrome);
            decay(&b->data.house.school);
            decay(&b->data.house.library);
            decay(&b->data.house.academy);
            decay(&b->data.house.barber);
            decay(&b->data.house.clinic);
            decay(&b->data.house.bathhouse);
            decay(&b->data.house.hospital);
            decay(&b->data.house.temple_ceres);
            decay(&b->data.house.temple_neptune);
            decay(&b->data.house.temple_mercury);
            decay(&b->data.house.temple_mars);
            decay(&b->data.house.temple_venus);
            decay(&b->house_pantheon_access);
            if (b->days_since_offering < 125) {
                ++b->days_since_offering;
            }
        }
    }
}

void house_service_decay_tax_collector(void)
{
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state == BUILDING_STATE_IN_USE && b->house_tax_coverage) {
                b->house_tax_coverage--;
            }
        }
    }
}

void house_service_decay_houses_covered(void)
{
    for (int i = 1; i < building_count(); i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_UNUSED && b->type != BUILDING_TOWER && b->type != BUILDING_WATCHTOWER) {
            if (b->houses_covered <= 1) {
                b->houses_covered = 0;
            } else {
                b->houses_covered--;
            }
        }
    }
}

void house_service_calculate_culture_aggregates(void)
{
    int venus_module2 = building_monument_gt_module_is_active(VENUS_MODULE_2_DESIRABILITY_ENTERTAINMENT);
    int completed_colosseum = building_monument_working(BUILDING_COLOSSEUM);
    int completed_hippodrome = building_monument_working(BUILDING_HIPPODROME);

    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state != BUILDING_STATE_IN_USE || !b->house_size) {
                continue;
            }
            int arena_total = 0;
            int colosseum_total = 0;

            // Entertainment
            b->data.house.entertainment = 0;

            if (b->data.house.theater) {
                b->data.house.entertainment += 10;
            }

            if (b->house_tavern_wine_access) {
                b->data.house.entertainment += 10;
                if (b->house_tavern_meat_access) {
                    b->data.house.entertainment += 5;
                }
            }

            if (b->data.house.amphitheater_actor) {
                if (b->data.house.amphitheater_gladiator) {
                    b->data.house.entertainment += 15;
                } else {
                    b->data.house.entertainment += 10;
                }
            }

            if (b->house_arena_gladiator) {
                arena_total = b->house_arena_lion ? 20 : 10;
            }

            if (b->data.house.colosseum_gladiator) {
                colosseum_total = b->data.house.colosseum_lion ? 25 : 15;
            }

            b->data.house.entertainment += arena_total > colosseum_total ? arena_total : colosseum_total;

            if (b->data.house.hippodrome) {
                b->data.house.entertainment += 30;
            }

            if (completed_hippodrome) {
                b->data.house.entertainment += 5;
            }

            if (completed_colosseum) {
                b->data.house.entertainment += 5;
            }

            // Venus Module 2 Entertainment Bonus
            if (venus_module2 && b->data.house.temple_venus) {
                b->data.house.entertainment += 10;
            }

            // Education
            b->data.house.education = 0;
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
            b->data.house.num_gods = 0;
            if (b->data.house.temple_ceres) {
                ++b->data.house.num_gods;
            }
            if (b->data.house.temple_neptune) {
                ++b->data.house.num_gods;
            }
            if (b->data.house.temple_mercury) {
                ++b->data.house.num_gods;
            }
            if (b->data.house.temple_mars) {
                ++b->data.house.num_gods;
            }
            if (b->data.house.temple_venus) {
                ++b->data.house.num_gods;
            }

            // health
            b->data.house.health = 0;
            if (b->data.house.clinic) {
                ++b->data.house.health;
            }
            if (b->data.house.hospital) {
                ++b->data.house.health;
            }
        }
    }
}
