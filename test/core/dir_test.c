#include "loki/loki.h"
#include "core/dir.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int string_compare_case_insensitive(const char *a, const char *b)
{
    return strcmp(a, b);
}

CREATE_MOCK2(int, file_has_extension, const char*, const char*)

INIT_MOCKS(
    INIT_MOCK(file_has_extension)
)

static int file_exists(const char *file)
{
    FILE *fp = fopen(file, "rb");
    if (fp) {
        fclose(fp);
        return 1;
    }
    return 0;
}

void test_dir_correct_simple_data()
{
    const char *result = dir_get_case_corrected_file("data/input.txt");
    assert_eq_string("data/input.txt", result);
}

void test_dir_correct_case_differs()
{
    const char *result = dir_get_case_corrected_file("DATA/INPUT.TXT");
    assert_true(result != NULL);
    assert_true(file_exists(result));
}

void test_dir_correct_case_differs_same_dir()
{
    const char *result = dir_get_case_corrected_file("Core_Dir_Test");
    assert_true(result != NULL);
    assert_true(file_exists(result));
}

void test_dir_correct_backslashes()
{
    const char *result = dir_get_case_corrected_file("DATA\\\\INPUT.TXT");
    assert_eq_string("data/input.txt", result);
}

void test_dir_correct_file_does_not_exist()
{
    const char *result = dir_get_case_corrected_file("data/non_existing");
    assert_true(NULL == result);
}

static int any_file(const char *file, const char *ext)
{
    return 1;
}

void test_dir_find_files_txt()
{
    chdir("data");
    when_file_has_extension_dynamic(any_file)->then_return = 1;

    const dir_listing *list = dir_find_files_with_extension("whatever");
    
    assert_eq(5, list->num_files);
    
    assert_eq_string(".", list->files[0]);
    assert_eq_string("..", list->files[1]);
    assert_eq_string("input.txt", list->files[2]);
    assert_eq_string("my.txt", list->files[3]);
    assert_eq_string("nontxt.dat", list->files[4]);

    verify_file_has_extension_times(5);
    
    chdir("..");
}

RUN_TESTS(dir,
    ADD_TEST(test_dir_correct_simple_data)
    ADD_TEST(test_dir_correct_case_differs)
    ADD_TEST(test_dir_correct_case_differs_same_dir)
    ADD_TEST(test_dir_correct_backslashes)
    ADD_TEST(test_dir_correct_file_does_not_exist)
    ADD_TEST(test_dir_find_files_txt)
)
