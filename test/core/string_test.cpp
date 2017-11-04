#include "loki/loki.h"
#include "core/string.hpp"

NO_MOCKS()

void test_string_copy_short()
{
    char src[100] = "my short string";
    char dst[100];

    string_copy(src, dst, 100);

    assert_eq_string("my short string", (char *) dst);
}

void test_string_copy_long()
{
    char src[100] = "my long string that's too long for the destination";
    char dst[100];

    string_copy(src, dst, 10);

    assert_eq(dst[9], 0);
    assert_eq_string("my long s", (char *) dst);
}

void test_string_from_int_with_plus()
{
    char dst[100] = "my little pony";
    int length;

    length = string_from_int(dst, 1, 1);

    assert_eq_string("+1", (char *) dst);
    assert_eq(2, length);
}

void test_string_from_all_size_ints()
{
    char dst[100];
    int length;

    length = string_from_int(dst, 1, 0);
    assert_eq_string("1", (char *) dst);
    assert_eq(1, length);

    length = string_from_int(dst, 12, 0);
    assert_eq_string("12", (char *) dst);
    assert_eq(2, length);

    length = string_from_int(dst, 123, 0);
    assert_eq_string("123", (char *) dst);
    assert_eq(3, length);

    length = string_from_int(dst, 1234, 0);
    assert_eq_string("1234", (char *) dst);
    assert_eq(4, length);

    length = string_from_int(dst, 12345, 0);
    assert_eq_string("12345", (char *) dst);
    assert_eq(5, length);

    length = string_from_int(dst, 123456, 0);
    assert_eq_string("123456", (char *) dst);
    assert_eq(6, length);

    length = string_from_int(dst, 1234567, 0);
    assert_eq_string("1234567", (char *) dst);
    assert_eq(7, length);

    length = string_from_int(dst, 12345678, 0);
    assert_eq_string("12345678", (char *) dst);
    assert_eq(8, length);

    length = string_from_int(dst, 123456789, 0);
    assert_eq_string("123456789", (char *) dst);
    assert_eq(9, length);
}

void test_string_from_large_int_without_plus()
{
    char dst[100];
    string_from_int(dst, 123456789, 0);

    assert_eq_string("123456789", (char *) dst);
}

void test_string_from_large_negative_int()
{
    char dst[100];
    string_from_int(dst, -987654321, 0);

    assert_eq_string("-987654321", (char *) dst);
}

void test_string_from_too_large_int()
{
    char dst[100];
    string_from_int(dst, 1234567890, 0);

    assert_eq_string("", (char *) dst);
}

void test_string_to_int_negative()
{
    assert_eq(-1, string_to_int("-1"));
    assert_eq(-123, string_to_int("-123"));
}

void test_string_to_int_positive()
{
    assert_eq(1, string_to_int("1"));
    assert_eq(123, string_to_int("123"));
}

void test_string_to_int_too_large_value_returns_0()
{
    assert_eq(0, string_to_int("1234567890"));
}

void test_string_to_int_non_numeric_postfix_ignored()
{
    assert_eq(12345, string_to_int("12345bladibla"));
    assert_eq(12345, string_to_int("12345.56"));
}

void test_string_compare_less()
{
    assert_true(string_compare_case_insensitive("a first thing", "BE PREPARED") < 0);
    assert_true(string_compare_case_insensitive("a", "AA") < 0);
}

void test_string_compare_equal()
{
    assert_true(string_compare_case_insensitive("mY lItTlE pOnY", "MY little PONY") == 0);
}

void test_string_compare_greater()
{
    assert_true(string_compare_case_insensitive("foo", "bar") > 0);
    assert_true(string_compare_case_insensitive("bb", "b") > 0);
}

RUN_TESTS(core/string,
          ADD_TEST(test_string_copy_short)
          ADD_TEST(test_string_copy_long)
          ADD_TEST(test_string_from_int_with_plus)
          ADD_TEST(test_string_from_large_int_without_plus)
          ADD_TEST(test_string_from_large_negative_int)
          ADD_TEST(test_string_from_all_size_ints)
          ADD_TEST(test_string_from_too_large_int)
          ADD_TEST(test_string_to_int_negative)
          ADD_TEST(test_string_to_int_positive)
          ADD_TEST(test_string_to_int_too_large_value_returns_0)
          ADD_TEST(test_string_to_int_non_numeric_postfix_ignored)
          ADD_TEST(test_string_compare_less)
          ADD_TEST(test_string_compare_equal)
          ADD_TEST(test_string_compare_greater)
         )
