#include "government.h"

#include "building/building.h"
#include "building/count.h"
#include "city/finance.h"

void building_government_distribute_treasury(void)
{
    int units =
        5 * building_count_active(BUILDING_SENATE) +
        1 * building_count_active(BUILDING_FORUM) +
        8 * building_count_active(BUILDING_SENATE_UPGRADED) +
        2 * building_count_active(BUILDING_FORUM_UPGRADED);
    int amount_per_unit = 0;
    int remainder = 0;
    int treasury = city_finance_treasury();

    if (treasury > 0 && units > 0) {
        amount_per_unit = treasury / units;
        remainder = treasury - units * amount_per_unit;
    }

    building *senate = building_first_of_type(BUILDING_SENATE_UPGRADED);
    if (!senate) {
        senate = building_first_of_type(BUILDING_SENATE);
    }
    if (senate && senate->state == BUILDING_STATE_IN_USE && !senate->house_size && senate->num_workers > 0) {
        int multiplier = senate->type == BUILDING_SENATE ? 5 : 8;
        senate->tax_income_or_storage = multiplier * amount_per_unit + remainder;
        remainder = 0;
    }

    for (building *b = building_first_of_type(BUILDING_FORUM_UPGRADED); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE || b->house_size || b->num_workers <= 0) {
            continue;
        }
        b->tax_income_or_storage = 2 * amount_per_unit + remainder;
        remainder = 0;
    }

    for (building *b = building_first_of_type(BUILDING_FORUM); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE || b->house_size || b->num_workers <= 0) {
            continue;
        }
        b->tax_income_or_storage = amount_per_unit + remainder;
        remainder = 0;
    }
}
