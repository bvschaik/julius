#include "loki/loki.h"
#include "core/file.h"

CREATE_MOCK1(const char*, dir_get_case_corrected_file, const char*)
CREATE_MOCK2(int, string_compare_case_insensitive, const char*, const char*)

INIT_MOCKS(
    INIT_MOCK(dir_get_case_corrected_file)
    INIT_MOCK(string_compare_case_insensitive)
)

void test_file_has_extension_normal_extension()
{
    const char *filename = "test.zip";
    const char *extension = "zip";
    when_string_compare_case_insensitive(&filename[5], extension)->then_return = 0;
    
    assert_true(file_has_extension(filename, extension));
    verify_string_compare_case_insensitive(&filename[5], extension);
}

void test_file_has_extension_double_extension()
{
    const char *filename = "test.tar.gz";
    const char *extension = "gz";
    when_string_compare_case_insensitive(&filename[5], extension)->then_return = 1;

    assert_false(file_has_extension(filename, extension));
    verify_string_compare_case_insensitive(&filename[5], extension);
}

void test_file_has_extension_no_extension()
{
    const char *filename = "test";
    const char *extension = "zip";
    when_string_compare_case_insensitive(&filename[4], extension)->then_return = -1;

    assert_false(file_has_extension(filename, extension));
    verify_string_compare_case_insensitive(&filename[4], extension);
}

void test_file_change_extension_no_extension_remains_unchanged()
{
    char filename[100] = "filename";
    file_change_extension(filename, "zip");
    assert_eq_string("filename", filename);
}

void test_file_change_extension_normal_extension()
{
    char filename[100] = "filename.txt";
    file_change_extension(filename, "zip");
    assert_eq_string("filename.zip", filename);
}

void test_file_change_extension_double_extension()
{
    char filename[100] = "filename.txt.zip";
    file_change_extension(filename, "map");
    assert_eq_string("filename.map", filename);
}

void test_file_append_extension()
{
    char filename[100] = "filename.txt";
    file_append_extension(filename, "zip");
    assert_eq_string("filename.txt.zip", filename);
}

void test_file_remove_extension_no_extension()
{
    char filename[100] = "filename";
    file_remove_extension(filename);
    assert_eq_string("filename", filename);
}

void test_file_remove_extension_normal_extension()
{
    char filename[100] = "filename.txt";
    file_remove_extension(filename);
    assert_eq_string("filename", filename);
}

void test_file_remove_extension_double_extension()
{
    char filename[100] = "filename.txt.zip";
    file_remove_extension(filename);
    assert_eq_string("filename", filename);
}

void test_file_exists_true()
{
    char filename[100] = "filename";
    when_dir_get_case_corrected_file(filename)->then_return = filename;
    
    assert_true(file_exists(filename));
}

void test_file_exists_false()
{
    char filename[100] = "filename";
    when_dir_get_case_corrected_file(filename)->then_return = NULL;
    
    assert_false(file_exists(filename));
}

RUN_TESTS(core/file,
    ADD_TEST(test_file_has_extension_normal_extension)
    ADD_TEST(test_file_has_extension_double_extension)
    ADD_TEST(test_file_has_extension_no_extension)
    ADD_TEST(test_file_change_extension_no_extension_remains_unchanged)
    ADD_TEST(test_file_change_extension_normal_extension)
    ADD_TEST(test_file_change_extension_double_extension)
    ADD_TEST(test_file_append_extension)
    ADD_TEST(test_file_remove_extension_no_extension)
    ADD_TEST(test_file_remove_extension_normal_extension)
    ADD_TEST(test_file_remove_extension_double_extension)
    ADD_TEST(test_file_exists_true)
    ADD_TEST(test_file_exists_false)
)
