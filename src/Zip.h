#ifndef PKZIP_H
#define PKZIP_H

/**
Compresses the input buffer
@param inputBuffer Input buffer to compress
@param inputLength Length of the input buffer
@param outputBuffer Output buffer to write the compressed data to
@param outputLength IN: available length of the output buffer, OUT: written bytes
*/
int Zip_compress(const void *inputBuffer, int inputLength, void *outputBuffer, int *outputLength);

/**
Decompresses the input buffer
@param inputBuffer Inputbuffer to decompress
@param inputLength Length of the input buffer
@param outputBuffer Output buffer to write decompressed data to
@param outputLengt IN: available length of the output buffer, OUT: written bytes
*/
int Zip_decompress(const void *inputBuffer, int inputLength, void *outputBuffer, int *outputLength);


#endif
