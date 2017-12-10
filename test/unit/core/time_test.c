#include "loki/loki.h"

#include "core/time.h"

NO_MOCKS()

void test_time_set_correctly()
{
    time_set_millis(123456);
    assert_eq_u(123456, time_get_millis());
}

RUN_TESTS(time,
    ADD_TEST(test_time_set_correctly)
)
