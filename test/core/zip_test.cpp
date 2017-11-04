#include "loki/loki.h"
#include "core/zip.hpp"

#include <stdint.h>

CREATE_VMOCK3(debug_log, const char*, const char*, int)

INIT_MOCKS(
    INIT_MOCK(debug_log)
)

void test_zip_decompress_sample()
{
    uint8_t input[] = {0x00, 0x04, 0x82, 0x24, 0x25, 0x8f, 0x80, 0x7f};
    int input_size = 8;
    char output[20];
    int output_size = 20;

    int ok = zip_decompress(input, input_size, output, &output_size);

    assert_true(ok);
    assert_eq(13, output_size);
    output[13] = 0;
    assert_eq_string("AIAIAIAIAIAIA", output);
}

void test_zip_compress_sample()
{
    char input[] = "AIAIAIAIAIAIA";
    int input_size = (int) strlen(input);

    uint8_t output[20];
    int output_size = 20;

    int ok = zip_compress(input, input_size, output, &output_size);

    assert_true(ok);

    assert_eq(9, output_size);
    uint8_t expected_output[] = {0x00, 0x06, 0x82, 0x24, 0x25, 0x0f, 0x02, 0xfe, 0x01};
    for (int i = 0; i < 9; i++)
    {
        assert_eq(expected_output[i], output[i]);
    }
}

void test_zip_compress_can_decompress()
{
    const char *input = "hello world this is a test for the compression algorithm";
    int input_size = (int) strlen(input) + 1;
    char compressed[100];
    int compressed_size = 100;
    char decompressed[100];
    int decompressed_size = 100;

    int compress_ok = zip_compress(input, input_size, compressed, &compressed_size);
    int decompress_ok = zip_decompress(compressed, compressed_size, decompressed, &decompressed_size);

    assert_true(compress_ok);
    assert_true(decompress_ok);
    assert_eq(input_size, decompressed_size);
    assert_eq_string(input, decompressed);
}

RUN_TESTS(zip,
          ADD_TEST(test_zip_decompress_sample)
          ADD_TEST(test_zip_compress_sample)
          ADD_TEST(test_zip_compress_can_decompress)
         )
