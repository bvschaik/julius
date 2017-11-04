#include "loki/loki.h"
#include "figure/properties.hpp"

NO_MOCKS()

void test_figure_properties_by_type()
{
    const figure_properties *zebra = figure_properties_for_type(FIGURE_ZEBRA);

    assert_eq(FIGURE_CATEGORY_ANIMAL, zebra->category);
}

RUN_TESTS(figure.properties,
          ADD_TEST(test_figure_properties_by_type)
         )
