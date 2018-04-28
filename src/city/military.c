#include "military.h"

#include "city/data_private.h"
#include "figure/formation.h"

#include "Data/CityInfo.h"

void city_military_clear_legionary_legions()
{
    city_data.military.legionary_legions = 0;
}

void city_military_add_legionary_legion()
{
    city_data.military.legionary_legions++;
}

int city_military_has_legionary_legions()
{
    return city_data.military.legionary_legions > 0;
}

int city_military_total_legions()
{
    return city_data.military.total_legions;
}

int city_military_total_soldiers()
{
    return city_data.military.total_soldiers;
}

int city_military_empire_service_legions()
{
    return city_data.military.empire_service_legions;
}

void city_military_clear_empire_service_legions()
{
    city_data.military.empire_service_legions = 0;
}

void city_military_update_totals()
{
    city_data.military.empire_service_legions = 0;
    city_data.military.total_soldiers = 0;
    city_data.military.total_legions = 0;
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        const formation *m = formation_get(i);
        if (m->in_use && m->is_legion) {
            city_data.military.total_legions++;
            city_data.military.total_soldiers += m->num_figures;
            if (m->empire_service && m->num_figures > 0) {
                city_data.military.empire_service_legions++;
            }
        }
    }
}
