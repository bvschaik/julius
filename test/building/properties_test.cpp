#include "loki/loki.h"
#include "building/properties.hpp"

NO_MOCKS()

void test_building_properties_by_type()
{
    const building_properties *engineer = building_properties_for_type(BUILDING_ENGINEERS_POST);

    assert_true(engineer->image_group);
}

RUN_TESTS(building.properties,
          ADD_TEST(test_building_properties_by_type)
         )
