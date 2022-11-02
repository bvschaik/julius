#ifndef CORE_ZLIB_HELPER_H
#define CORE_ZLIB_HELPER_H

int zlib_helper_decompress(void *input_buffer, const int input_length, void *output_buffer, const int output_buffer_length, int *output_length);

int zlib_helper_compress(void *input_buffer, const int input_length, void *output_buffer, const int output_buffer_length, int *output_length);

#endif // CORE_ZLIB_HELPER_H
