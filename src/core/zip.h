#ifndef CORE_ZIP_H
#define CORE_ZIP_H

/**
 * @file
 * Compression functions.
 */

/**
 * Decompresses the input buffer
 * @param input_buffer Inputbuffer to decompress
 * @param input_length Length of the input buffer
 * @param output_buffer Output buffer to write decompressed data to
 * @param output_length Available length of the output buffer
 * @return boolean true on success, false on error
 */
int zip_decompress(const void *input_buffer, int input_length, void *output_buffer, int output_length);

#endif // CORE_ZIP_H
