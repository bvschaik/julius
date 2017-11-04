#include "loki/loki.h"
#include "core/io.hpp"

#include <unistd.h>

CREATE_MOCK1(const char*, dir_get_case_corrected_file, const char*)

INIT_MOCKS(
    INIT_MOCK(dir_get_case_corrected_file)
)

static const char *input_file = "data/input.txt";
static const char *output_file = "data/output.txt";

void test_io_read_file_into_buffer()
{
    char buffer[100];

    when_dir_get_case_corrected_file(input_file)->then_return = input_file;
    int bytes_read = io_read_file_into_buffer(input_file, buffer, 100);

    assert_eq(48, bytes_read);
    buffer[48] = 0;
    assert_eq_string("A0123456789,B0123456789,C0123456789,D0123456789,", buffer);
}

void test_io_read_file_into_small_buffer()
{
    char buffer[11];

    when_dir_get_case_corrected_file(input_file)->then_return = input_file;
    int bytes_read = io_read_file_into_buffer(input_file, buffer, 10);

    assert_eq(10, bytes_read);
    buffer[10] = 0;
    assert_eq_string("A012345678", buffer);
}

void test_io_read_file_nonexisting()
{
    char buffer[11];

    when_dir_get_case_corrected_file(input_file)->then_return = NULL;
    int bytes_read = io_read_file_into_buffer(input_file, buffer, 10);

    assert_eq(0, bytes_read);
}

void test_io_read_file_cannot_open()
{
    char buffer[11];

    when_dir_get_case_corrected_file(input_file)->then_return = "nonexisting";
    int bytes_read = io_read_file_into_buffer(input_file, buffer, 10);

    assert_eq(0, bytes_read);
}

void test_io_read_file_part_into_buffer()
{
    char buffer[11];

    when_dir_get_case_corrected_file(input_file)->then_return = input_file;
    int bytes_read = io_read_file_part_into_buffer(input_file, buffer, 10, 12);

    assert_eq(10, bytes_read);
    buffer[10] = 0;
    assert_eq_string("B012345678", buffer);
}

void test_io_read_file_part_into_buffer_nonexisting()
{
    char buffer[11];

    when_dir_get_case_corrected_file(input_file)->then_return = NULL;
    int bytes_read = io_read_file_part_into_buffer(input_file, buffer, 10, 12);

    assert_eq(0, bytes_read);
}

void test_io_write_buffer_to_file_new()
{
    const char *buffer = "hello world";

    when_dir_get_case_corrected_file(output_file)->then_return = NULL;
    int bytes_written = io_write_buffer_to_file(output_file, buffer, 11);

    assert_eq(11, bytes_written);

    remove(output_file);
}

void test_io_write_buffer_to_file_existing()
{
    const char *buffer = "hello world";

    when_dir_get_case_corrected_file(output_file)->then_return = output_file;
    int bytes_written = io_write_buffer_to_file(output_file, buffer, 11);

    assert_eq(11, bytes_written);

    remove(output_file);
}

RUN_TESTS(core/io,
          ADD_TEST(test_io_read_file_into_buffer)
          ADD_TEST(test_io_read_file_into_small_buffer)
          ADD_TEST(test_io_read_file_nonexisting)
          ADD_TEST(test_io_read_file_cannot_open)
          ADD_TEST(test_io_read_file_part_into_buffer)
          ADD_TEST(test_io_read_file_part_into_buffer_nonexisting)
          ADD_TEST(test_io_write_buffer_to_file_new)
          ADD_TEST(test_io_write_buffer_to_file_existing)
          unlink(output_file);
         )
