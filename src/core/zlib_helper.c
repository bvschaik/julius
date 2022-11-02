#include "zlib.h"

int zlib_helper_decompress(void *input_buffer, const int input_length, void *output_buffer, const int output_buffer_length, int *output_length)
{
    z_stream strm;

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    if (inflateInit(&strm) != Z_OK) {
        return 0;
    }

    strm.avail_in = input_length;
    strm.next_in = input_buffer;
    strm.avail_out = output_buffer_length;
    strm.next_out = output_buffer;
    int result = inflate(&strm, Z_NO_FLUSH);
    inflateEnd(&strm);
    if (result != Z_STREAM_END || strm.avail_out != 0) {
        return 0;
    }
    *output_length = output_buffer_length - strm.avail_out;
    return 1;
}

int zlib_helper_compress(void *input_buffer, const int input_length, void *output_buffer, const int output_buffer_length, int *output_length)
{
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    if (deflateInit(&strm, Z_BEST_SPEED) != Z_OK) {
        return 0;
    }

    strm.avail_in = input_length;
    strm.next_in = input_buffer;
    strm.avail_out = output_buffer_length;
    strm.next_out = output_buffer;
    int result = deflate(&strm, Z_FINISH);
    deflateEnd(&strm);
    if (result != Z_STREAM_END || strm.avail_in != 0) {
        return 0;
    }

    *output_length = output_buffer_length - strm.avail_out;
    return 1;
}
