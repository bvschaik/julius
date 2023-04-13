#include "comparison_helper.h"

#include "scenario/scenario_event_data.h"

int comparison_helper_compare_values(int compare_type, int value1, int value2)
{
    switch (compare_type) {
        case COMPARISON_TYPE_EQUAL:
            return value1 == value2;
        case COMPARISON_TYPE_EQUAL_OR_LESS:
            return value1 <= value2;
        case COMPARISON_TYPE_EQUAL_OR_MORE:
            return value1 >= value2;
        default:
            return 0;
    }
}
