#include "count.hpp"

#include <string.h>

struct record
{
    int active;
    int total;
};

static struct
{
    struct record buildings[BUILDING_TYPE_MAX];
    struct record industry[RESOURCE_MAX];
} data;

void building_count_clear()
{
    memset(&data, 0, sizeof(data));
}

void building_count_increase(building_type type, int active)
{
    ++data.buildings[type].total;
    if (active)
    {
        ++data.buildings[type].active;
    }
}

void building_count_industry_increase(resource_type resource, int active)
{
    ++data.industry[resource].total;
    if (active)
    {
        ++data.industry[resource].active;
    }
}

void building_count_limit_hippodrome()
{
    if (data.buildings[BUILDING_HIPPODROME].total > 1)
    {
        data.buildings[BUILDING_HIPPODROME].total = 1;
    }
    if (data.buildings[BUILDING_HIPPODROME].active > 1)
    {
        data.buildings[BUILDING_HIPPODROME].active = 1;
    }
}

int building_count_active(building_type type)
{
    return data.buildings[type].active;
}

int building_count_total(building_type type)
{
    return data.buildings[type].total;
}

int building_count_industry_active(resource_type resource)
{
    return data.industry[resource].active;
}

int building_count_industry_total(resource_type resource)
{
    return data.industry[resource].total;
}


void building_count_save_state(buffer *industry, buffer *culture1, buffer *culture2,
                               buffer *culture3, buffer *military, buffer *support)
{
    // industry
    for (int i = 0; i < RESOURCE_MAX; i++)
    {
        buffer_write_i32(industry, data.industry[i].total);
    }
    for (int i = 0; i < RESOURCE_MAX; i++)
    {
        buffer_write_i32(industry, data.industry[i].active);
    }

    // culture 1
    buffer_write_i32(culture1, data.buildings[BUILDING_THEATER].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_THEATER].active);
    buffer_write_i32(culture1, data.buildings[BUILDING_AMPHITHEATER].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_AMPHITHEATER].active);
    buffer_write_i32(culture1, data.buildings[BUILDING_COLOSSEUM].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_COLOSSEUM].active);
    buffer_write_i32(culture1, data.buildings[BUILDING_HIPPODROME].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_HIPPODROME].active);
    buffer_write_i32(culture1, data.buildings[BUILDING_SCHOOL].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_SCHOOL].active);
    buffer_write_i32(culture1, data.buildings[BUILDING_LIBRARY].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_LIBRARY].active);
    buffer_write_i32(culture1, data.buildings[BUILDING_ACADEMY].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_ACADEMY].active);
    buffer_write_i32(culture1, data.buildings[BUILDING_BARBER].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_BARBER].active);
    buffer_write_i32(culture1, data.buildings[BUILDING_BATHHOUSE].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_BATHHOUSE].active);
    buffer_write_i32(culture1, data.buildings[BUILDING_DOCTOR].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_DOCTOR].active);
    buffer_write_i32(culture1, data.buildings[BUILDING_HOSPITAL].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_HOSPITAL].active);
    buffer_write_i32(culture1, data.buildings[BUILDING_SMALL_TEMPLE_CERES].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_SMALL_TEMPLE_NEPTUNE].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_SMALL_TEMPLE_MERCURY].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_SMALL_TEMPLE_MARS].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_SMALL_TEMPLE_VENUS].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_LARGE_TEMPLE_CERES].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_LARGE_TEMPLE_NEPTUNE].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_LARGE_TEMPLE_MERCURY].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_LARGE_TEMPLE_MARS].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_LARGE_TEMPLE_VENUS].total);
    buffer_write_i32(culture1, data.buildings[BUILDING_ORACLE].total);

    // culture 2
    buffer_write_i32(culture2, data.buildings[BUILDING_ACTOR_COLONY].total);
    buffer_write_i32(culture2, data.buildings[BUILDING_ACTOR_COLONY].active);
    buffer_write_i32(culture2, data.buildings[BUILDING_GLADIATOR_SCHOOL].total);
    buffer_write_i32(culture2, data.buildings[BUILDING_GLADIATOR_SCHOOL].active);
    buffer_write_i32(culture2, data.buildings[BUILDING_LION_HOUSE].total);
    buffer_write_i32(culture2, data.buildings[BUILDING_LION_HOUSE].active);
    buffer_write_i32(culture2, data.buildings[BUILDING_CHARIOT_MAKER].total);
    buffer_write_i32(culture2, data.buildings[BUILDING_CHARIOT_MAKER].active);

    // culture 3
    buffer_write_i32(culture3, data.buildings[BUILDING_SMALL_TEMPLE_CERES].active);
    buffer_write_i32(culture3, data.buildings[BUILDING_SMALL_TEMPLE_NEPTUNE].active);
    buffer_write_i32(culture3, data.buildings[BUILDING_SMALL_TEMPLE_MERCURY].active);
    buffer_write_i32(culture3, data.buildings[BUILDING_SMALL_TEMPLE_MARS].active);
    buffer_write_i32(culture3, data.buildings[BUILDING_SMALL_TEMPLE_VENUS].active);
    buffer_write_i32(culture3, data.buildings[BUILDING_LARGE_TEMPLE_CERES].active);
    buffer_write_i32(culture3, data.buildings[BUILDING_LARGE_TEMPLE_NEPTUNE].active);
    buffer_write_i32(culture3, data.buildings[BUILDING_LARGE_TEMPLE_MERCURY].active);
    buffer_write_i32(culture3, data.buildings[BUILDING_LARGE_TEMPLE_MARS].active);
    buffer_write_i32(culture3, data.buildings[BUILDING_LARGE_TEMPLE_VENUS].active);

    // military
    buffer_write_i32(military, data.buildings[BUILDING_MILITARY_ACADEMY].total);
    buffer_write_i32(military, data.buildings[BUILDING_MILITARY_ACADEMY].active);
    buffer_write_i32(military, data.buildings[BUILDING_BARRACKS].total);
    buffer_write_i32(military, data.buildings[BUILDING_BARRACKS].active);

    // support
    buffer_write_i32(support, data.buildings[BUILDING_MARKET].total);
    buffer_write_i32(support, data.buildings[BUILDING_MARKET].active);
    buffer_write_i32(support, data.buildings[BUILDING_RESERVOIR].total);
    buffer_write_i32(support, data.buildings[BUILDING_RESERVOIR].active);
    buffer_write_i32(support, data.buildings[BUILDING_FOUNTAIN].total);
    buffer_write_i32(support, data.buildings[BUILDING_FOUNTAIN].active);
}

void building_count_load_state(buffer *industry, buffer *culture1, buffer *culture2,
                               buffer *culture3, buffer *military, buffer *support)
{
    // industry
    for (int i = 0; i < RESOURCE_MAX; i++)
    {
        data.industry[i].total = buffer_read_i32(industry);
    }
    for (int i = 0; i < RESOURCE_MAX; i++)
    {
        data.industry[i].active = buffer_read_i32(industry);
    }

    // culture 1
    data.buildings[BUILDING_THEATER].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_THEATER].active = buffer_read_i32(culture1);
    data.buildings[BUILDING_AMPHITHEATER].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_AMPHITHEATER].active = buffer_read_i32(culture1);
    data.buildings[BUILDING_COLOSSEUM].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_COLOSSEUM].active = buffer_read_i32(culture1);
    data.buildings[BUILDING_HIPPODROME].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_HIPPODROME].active = buffer_read_i32(culture1);
    data.buildings[BUILDING_SCHOOL].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_SCHOOL].active = buffer_read_i32(culture1);
    data.buildings[BUILDING_LIBRARY].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_LIBRARY].active = buffer_read_i32(culture1);
    data.buildings[BUILDING_ACADEMY].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_ACADEMY].active = buffer_read_i32(culture1);
    data.buildings[BUILDING_BARBER].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_BARBER].active = buffer_read_i32(culture1);
    data.buildings[BUILDING_BATHHOUSE].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_BATHHOUSE].active = buffer_read_i32(culture1);
    data.buildings[BUILDING_DOCTOR].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_DOCTOR].active = buffer_read_i32(culture1);
    data.buildings[BUILDING_HOSPITAL].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_HOSPITAL].active = buffer_read_i32(culture1);
    data.buildings[BUILDING_SMALL_TEMPLE_CERES].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_SMALL_TEMPLE_NEPTUNE].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_SMALL_TEMPLE_MERCURY].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_SMALL_TEMPLE_MARS].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_SMALL_TEMPLE_VENUS].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_LARGE_TEMPLE_CERES].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_LARGE_TEMPLE_NEPTUNE].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_LARGE_TEMPLE_MERCURY].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_LARGE_TEMPLE_MARS].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_LARGE_TEMPLE_VENUS].total = buffer_read_i32(culture1);
    data.buildings[BUILDING_ORACLE].total = buffer_read_i32(culture1);

    // culture 2
    data.buildings[BUILDING_ACTOR_COLONY].total = buffer_read_i32(culture2);
    data.buildings[BUILDING_ACTOR_COLONY].active = buffer_read_i32(culture2);
    data.buildings[BUILDING_GLADIATOR_SCHOOL].total = buffer_read_i32(culture2);
    data.buildings[BUILDING_GLADIATOR_SCHOOL].active = buffer_read_i32(culture2);
    data.buildings[BUILDING_LION_HOUSE].total = buffer_read_i32(culture2);
    data.buildings[BUILDING_LION_HOUSE].active = buffer_read_i32(culture2);
    data.buildings[BUILDING_CHARIOT_MAKER].total = buffer_read_i32(culture2);
    data.buildings[BUILDING_CHARIOT_MAKER].active = buffer_read_i32(culture2);

    // culture 3
    data.buildings[BUILDING_SMALL_TEMPLE_CERES].active = buffer_read_i32(culture3);
    data.buildings[BUILDING_SMALL_TEMPLE_NEPTUNE].active = buffer_read_i32(culture3);
    data.buildings[BUILDING_SMALL_TEMPLE_MERCURY].active = buffer_read_i32(culture3);
    data.buildings[BUILDING_SMALL_TEMPLE_MARS].active = buffer_read_i32(culture3);
    data.buildings[BUILDING_SMALL_TEMPLE_VENUS].active = buffer_read_i32(culture3);
    data.buildings[BUILDING_LARGE_TEMPLE_CERES].active = buffer_read_i32(culture3);
    data.buildings[BUILDING_LARGE_TEMPLE_NEPTUNE].active = buffer_read_i32(culture3);
    data.buildings[BUILDING_LARGE_TEMPLE_MERCURY].active = buffer_read_i32(culture3);
    data.buildings[BUILDING_LARGE_TEMPLE_MARS].active = buffer_read_i32(culture3);
    data.buildings[BUILDING_LARGE_TEMPLE_VENUS].active = buffer_read_i32(culture3);

    // military
    data.buildings[BUILDING_MILITARY_ACADEMY].total = buffer_read_i32(military);
    data.buildings[BUILDING_MILITARY_ACADEMY].active = buffer_read_i32(military);
    data.buildings[BUILDING_BARRACKS].total = buffer_read_i32(military);
    data.buildings[BUILDING_BARRACKS].active = buffer_read_i32(military);

    // support
    data.buildings[BUILDING_MARKET].total = buffer_read_i32(support);
    data.buildings[BUILDING_MARKET].active = buffer_read_i32(support);
    data.buildings[BUILDING_RESERVOIR].total = buffer_read_i32(support);
    data.buildings[BUILDING_RESERVOIR].active = buffer_read_i32(support);
    data.buildings[BUILDING_FOUNTAIN].total = buffer_read_i32(support);
    data.buildings[BUILDING_FOUNTAIN].active = buffer_read_i32(support);
}
