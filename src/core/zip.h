#ifndef ZIP_H
#define ZIP_H

/**
 * @file
 * Compression functions.
 */

/**
 * Compresses the input buffer.
 * @param input_buffer Input buffer to compress
 * @param input_length Length of input buffer
 * @param output_buffer Output buffer to write the compressed data to
 * @param output_length IN: available length of the output buffer, OUT: written bytes
 * @return boolean true on success, false on error
 */
int zip_compress(const void *input_buffer, int input_length, void *output_buffer, int *output_length);

/**
 * Decompresses the input buffer
 * @param input_buffer Inputbuffer to decompress
 * @param input_length Length of the input buffer
 * @param output_buffer Output buffer to write decompressed data to
 * @param output_length IN: available length of the output buffer, OUT: written bytes
 * @return boolean true on success, false on error
 */
int zip_decompress(const void *input_buffer, int input_length, void *output_buffer, int *output_length);

#endif // ZIP_H
