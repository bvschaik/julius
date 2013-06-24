#include "Zip.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// TODO
static void debug_log(const char *msg, int a, int b)
{
	printf("%s %d %d\n", msg, a, b);
}

enum {
	PK_SUCCESS = 0,
	PK_INVALID_WINDOWSIZE = 1,
	PK_LITERAL_ENCODING_UNSUPPORTED = 2,
	PK_TOO_FEW_INPUT_BYTES = 3,
	PK_ERROR_DECODING = 4,
	PK_ERROR_VALUE = 774,
	PK_EOF = 773,
};

struct PKToken {
	int stop;
	
	const char *inputData;
	int inputPtr;
	int inputLength;
	
	char *outputData;
	int outputPtr;
	int outputLength;
};

struct PKCompBuffer {
	int (*inputFunc)(unsigned char*, int, struct PKToken*);
	void (*outputFunc)(unsigned char*, int, struct PKToken*);
	struct PKToken *token;

	int windowSize;
	int dictionarySize;
	int copyOffsetExtraMask;
	int currentOutputBitsUsed;

	unsigned char inputData[8708];
	unsigned char outputData[2050];
	int outputPtr;

	unsigned short analyzeOffsetTable[2304];
	unsigned short analyzeIndex[8708];

	unsigned short codewordValues[774];
	unsigned char codewordBits[774];
};

struct PKDecompBuffer {
	int (*inputFunc)(unsigned char*, int, struct PKToken*);
	void (*outputFunc)(unsigned char*, int, struct PKToken*);
	struct PKToken *token;

	int windowSize;
	int dictionarySize;
	unsigned int currentInputByte;
	int currentInputBitsAvailable;
	int inputBufferPtr;
	int inputBufferEnd;
	int outputBufferPtr;
	unsigned char inputBuffer[2048];
	unsigned char outputBuffer[8708]; // 2x 4096 (max dict size) + 516 for copying

	unsigned char copyOffsetJumpTable[256];
	unsigned char copyLengthJumpTable[256];
};

struct PKCopyLengthOffset {
	unsigned short length;
	unsigned short offset;
};

static const unsigned char pk_copyOffsetBits[64] = {
	2, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
};

static const unsigned char pk_copyOffsetCode[64] = {
	0x03, 0x0D, 0x05, 0x19, 0x09, 0x11, 0x01, 0x3E,
	0x1E, 0x2E, 0x0E, 0x36, 0x16, 0x26, 0x06, 0x3A,
	0x1A, 0x2A, 0x0A, 0x32, 0x12, 0x22, 0x42, 0x02,
	0x7C, 0x3C, 0x5C, 0x1C, 0x6C, 0x2C, 0x4C, 0x0C,
	0x74, 0x34, 0x54, 0x14, 0x64, 0x24, 0x44, 0x04,
	0x78, 0x38, 0x58, 0x18, 0x68, 0x28, 0x48, 0x08,
	0xF0, 0x70, 0xB0, 0x30, 0xD0, 0x50, 0x90, 0x10,
	0xE0, 0x60, 0xA0, 0x20, 0xC0, 0x40, 0x80, 0x00,
};

static const unsigned char pk_copyLengthBaseBits[16] = {
	3, 2, 3, 3, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 7, 7,
};

static const unsigned short pk_copyLengthBaseValue[16] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x0A, 0x0E, 0x16, 0x26, 0x46, 0x86, 0x106,
};

static const unsigned char pk_copyLengthBaseCode[16] = {
	0x05, 0x03, 0x01, 0x06, 0x0A, 0x02, 0x0C, 0x14,
	0x04, 0x18, 0x08, 0x30, 0x10, 0x20, 0x40, 0x00,
};

static const unsigned char pk_copyLengthExtraBits[16] = {
	0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
};

// Function declarations
static int pk_implode(int (*inputFunc)(unsigned char*, int, struct PKToken*), void (*outputFunc)(unsigned char*, int, struct PKToken*), struct PKCompBuffer *buf, struct PKToken *token, int dictionarySize);
static void pk_implodeData(struct PKCompBuffer *buf);
static void pk_implodeDetermineCopy(struct PKCompBuffer *buf, int offset, struct PKCopyLengthOffset *copy);
static void pk_implodeWriteBits(struct PKCompBuffer *buf, unsigned char numBits, unsigned int value);
static void pk_implodeFlushFullBuffer(struct PKCompBuffer *buf);
static void pk_implodeAnalyzeInput(struct PKCompBuffer *buf, int inputStart, int inputEnd);

static int pk_explode(int (*inputFunc)(unsigned char*, int, struct PKToken*), void (*outputFunc)(unsigned char*, int, struct PKToken*), struct PKDecompBuffer *buf, struct PKToken *token);
static int pk_explodeData(struct PKDecompBuffer *buf);
static int pk_explodeDecodeNextToken(struct PKDecompBuffer *buf);
static int pk_explodeGetCopyOffset(struct PKDecompBuffer *buf, int copyLength);
static int pk_explodeSetBitsUsed(struct PKDecompBuffer *buf, int numBits);
static void pk_explodeConstructJumpTable(int size, const unsigned char *bits, const unsigned char *codes, unsigned char *jump);

static void pk_memset(void *buffer, unsigned char fillChar, int length);
static void pk_memcpy(unsigned char *dst, const unsigned char *src, int length);

static int zip_inputFunc(unsigned char *buffer, int length, struct PKToken *token)
{
	if (token->stop) {
		return 0;
	}
	if (token->inputPtr >= token->inputLength) {
		return 0;
	}

	if (token->inputLength - token->inputPtr < length) {
		length = token->inputLength - token->inputPtr;
	}
	memcpy(buffer, &token->inputData[token->inputPtr], length);
	token->inputPtr += length;
	return length;
}

static void zip_outputFunc(unsigned char *buffer, int length, struct PKToken *token)
{
	if (token->stop) {
		return;
	}
	if (token->outputPtr >= token->outputLength) {
		debug_log("ERR:COMP2 Out of buffer space.", 0, 0);
		token->stop = 1;
		return;
	}

	if (token->outputLength - token->outputPtr >= length) {
		memcpy(&token->outputData[token->outputPtr], buffer, length);
		token->outputPtr += length;
	} else {
		debug_log("ERR:COMP1 Corrupt.", 0, 0);
		token->stop = 1;
	}
}

int Zip_compress(const void *inputBuffer, int inputLength, void *outputBuffer, int *outputLength)
{
	struct PKToken token;
	struct PKCompBuffer *buf = (struct PKCompBuffer *) malloc(sizeof(struct PKCompBuffer));

	if (!buf) {
		return 0;
	}

	memset(buf, 0, sizeof(struct PKCompBuffer));
	memset(&token, 0, sizeof(struct PKToken));
	token.inputData = (const char*) inputBuffer;
	token.inputLength = inputLength;
	token.outputData = (char *) outputBuffer;
	token.outputLength = *outputLength;

	int ok = 1;
	int pkError = pk_implode(zip_inputFunc, zip_outputFunc, buf, &token, 4096);
	if (pkError || token.stop) {
		debug_log("ERR:COMP Error occurred while compressing.", 0, 0);
        ok = 0;
	} else {
		*outputLength = token.outputPtr;
	}
	free(buf);
	return ok;
}

int Zip_decompress(const void *inputBuffer, int inputLength, void *outputBuffer, int *outputLength)
{
	struct PKDecompBuffer *buf;
	struct PKToken token;

	buf = (struct PKDecompBuffer *) malloc(sizeof(struct PKDecompBuffer));
	if (!buf) {
		return 0;
	}
	memset(buf, 0, sizeof(struct PKDecompBuffer));
	memset(&token, 0, sizeof(struct PKToken));
	token.inputData = (const char*) inputBuffer;
	token.inputLength = inputLength;
	token.outputData = (char*) outputBuffer;
	token.outputLength = *outputLength;

	int ok = 1;
	int pkError = pk_explode(zip_inputFunc, zip_outputFunc, buf, &token);
	if (pkError || token.stop) {
		debug_log("ERR:COMP Error uncompressing.", 0, 0);
		ok = 0;
	} else {
		*outputLength = token.outputPtr;
	}
	free(buf);
	return ok;
}

static int pk_implode(int (*inputFunc)(unsigned char*, int, struct PKToken*), void (*outputFunc)(unsigned char*, int, struct PKToken*), struct PKCompBuffer *buf, struct PKToken *token, int dictionarySize)
{
	buf->inputFunc = inputFunc;
	buf->outputFunc = outputFunc;
	buf->dictionarySize = dictionarySize;
	buf->token = token;
	if (dictionarySize == 1024) {
		buf->windowSize = 4;
		buf->copyOffsetExtraMask = 0xf;
	} else if (dictionarySize == 2048) {
		buf->windowSize = 5;
		buf->copyOffsetExtraMask = 0x1f;
	} else if (dictionarySize == 4096) {
		buf->windowSize = 6;
		buf->copyOffsetExtraMask = 0x3f;
	} else {
		return PK_INVALID_WINDOWSIZE;
	}

	for (int i = 0; i < 256; i++) {
		buf->codewordBits[i] = 9; // 8 + 1 for leading zero
		buf->codewordValues[i] = i << 1; // include leading zero to indicate literal byte
	}

	// prepare copy length values right after the literal bits
	int codeIndex = 256;
	for (int copy = 0; copy < 16; copy++) {
		int baseBits = pk_copyLengthBaseBits[copy];
		int extraBits = pk_copyLengthExtraBits[copy];
		int baseCode = pk_copyLengthBaseCode[copy];
		int max = 1 << extraBits;
		for (int i = 0; i < max; i++) {
			buf->codewordBits[codeIndex] = 1 + baseBits + extraBits;
			buf->codewordValues[codeIndex] = 1 | (baseCode << 1) | (i << (baseBits + 1));
			codeIndex++;
		}
	}

	pk_implodeData(buf);
	return PK_SUCCESS;
}

static int pk_implodeFillInputBuffer(struct PKCompBuffer *buf, int bytesToRead)
{
	int used = 0;
	int read;
	do {
		read = buf->inputFunc(&buf->inputData[buf->dictionarySize + 516 + used], bytesToRead, buf->token);
		used += read;
		bytesToRead -= read;
	} while (read && bytesToRead > 0);
	return used;
}

static void pk_implodeWriteCopyLengthOffset(struct PKCompBuffer *buf, struct PKCopyLengthOffset copy)
{
	pk_implodeWriteBits(buf, buf->codewordBits[copy.length + 254], buf->codewordValues[copy.length + 254]);

	if (copy.length == 2) {
		pk_implodeWriteBits(buf, pk_copyOffsetBits[copy.offset >> 2],
			pk_copyOffsetCode[copy.offset >> 2]);
		pk_implodeWriteBits(buf, 2, copy.offset & 3);
	} else {
		pk_implodeWriteBits(buf, pk_copyOffsetBits[copy.offset >> buf->windowSize],
			pk_copyOffsetCode[copy.offset >> buf->windowSize]);
		pk_implodeWriteBits(buf, buf->windowSize, copy.offset & buf->copyOffsetExtraMask);
	}
}

static int pk_implodeNextCopyIsBetter(struct PKCompBuffer *buf, int offset, const struct PKCopyLengthOffset *currentCopy)
{
	struct PKCopyLengthOffset nextCopy;
	pk_implodeDetermineCopy(buf, offset + 1, &nextCopy);
	if (currentCopy->length >= nextCopy.length) {
		return 0;
	}
	if (currentCopy->length + 1 == nextCopy.length && currentCopy->offset <= 128) {
		return 0;
	}
	return 1;
}

static void pk_implodeData(struct PKCompBuffer *buf)
{
	int eof = 0;
	int hasLeftoverData = 0;

	buf->outputData[0] = 0; // no literal encoding
	buf->outputData[1] = buf->windowSize;
	buf->outputPtr = 2;

	int inputPtr = buf->dictionarySize + 516;
	pk_memset(&buf->outputData[2], 0, 2048);

	buf->currentOutputBitsUsed = 0;

	while (!eof) {
		int bytesRead = pk_implodeFillInputBuffer(buf, 4096);
		if (bytesRead != 4096) {
			eof = 1;
			if (!bytesRead && hasLeftoverData == 0) {
				break;
			}
		}
		int inputEnd = buf->dictionarySize + bytesRead; // keep 516 bytes leftover
		if (eof) {
			inputEnd += 516; // eat the 516 leftovers anyway
		}

		if (hasLeftoverData == 0) {
			pk_implodeAnalyzeInput(buf, inputPtr, inputEnd + 1);
			hasLeftoverData++;
			if (buf->dictionarySize != 4096) {
				hasLeftoverData++;
			}
		} else if (hasLeftoverData == 1) {
			pk_implodeAnalyzeInput(buf, inputPtr - buf->dictionarySize + 516, inputEnd + 1);
			hasLeftoverData++;
		} else if (hasLeftoverData == 2) {
			pk_implodeAnalyzeInput(buf, inputPtr - buf->dictionarySize, inputEnd + 1);
		}

		while (inputPtr < inputEnd) {
			int writeLiteral = 0;
			int writeCopy = 0;
			struct PKCopyLengthOffset copy;
			pk_implodeDetermineCopy(buf, inputPtr, &copy);
			
			if (copy.length == 0) {
				writeLiteral = 1;
			} else if (copy.length == 2 && copy.offset >= 256) {
				writeLiteral = 1;
			} else if (eof && inputPtr + copy.length > inputEnd) {
				copy.length = inputEnd - inputPtr;
				if (inputEnd - inputPtr > 2 || (inputEnd - inputPtr == 2 && copy.offset < 256)) {
					writeCopy = 1;
				} else {
					writeLiteral = 1;
				}
			} else if (copy.length >= 8 || inputPtr + 1 >= inputEnd) {
				writeCopy = 1;
			} else if (pk_implodeNextCopyIsBetter(buf, inputPtr, &copy)) {
				writeLiteral = 1;
			} else {
				writeCopy = 1;
			}

			if (writeCopy) {
				pk_implodeWriteCopyLengthOffset(buf, copy);
				inputPtr += copy.length;
			} else if (writeLiteral) {
				// Write literal
				pk_implodeWriteBits(buf, buf->codewordBits[buf->inputData[inputPtr]],
					buf->codewordValues[buf->inputData[inputPtr]]);
				inputPtr++;
			}
		}

		if (!eof) {
			inputPtr -= 4096;
			pk_memcpy(buf->inputData, &buf->inputData[4096], buf->dictionarySize + 516);
		}
	}

	// Write EOF
	pk_implodeWriteBits(buf, buf->codewordBits[PK_EOF], buf->codewordValues[PK_EOF]);
	if (buf->currentOutputBitsUsed) {
		buf->outputPtr++;
	}
	buf->outputFunc(buf->outputData, buf->outputPtr, buf->token);
}

static void pk_implodeDetermineCopy(struct PKCompBuffer *buf, int inputIndex, struct PKCopyLengthOffset *copy)
{
	int hashValue = 4 * buf->inputData[inputIndex] + 5 * buf->inputData[inputIndex + 1];
	int minInputIndex = inputIndex - buf->dictionarySize + 1;
	
	int analyzeOffset = buf->analyzeOffsetTable[hashValue];
	while (buf->analyzeIndex[analyzeOffset] < minInputIndex) {
		analyzeOffset++;
	}
	buf->analyzeOffsetTable[hashValue] = analyzeOffset;

	if (buf->analyzeIndex[analyzeOffset] >= inputIndex - 1) {
		copy->length = 0;
		return;
	}


	int maxMatchedBytes = 1;
	unsigned char *inputPtr = &buf->inputData[inputIndex];
	for (; buf->analyzeIndex[analyzeOffset] < inputIndex - 1; analyzeOffset++) {
		int matchIndex = buf->analyzeIndex[analyzeOffset];
		unsigned char *matchPtr = &buf->inputData[matchIndex];
		// Check first character (one char is enough to verify byte 1 and 2 [hash])
		// Then check character at last position from longest match to quickly disregard shorter matches
		if (matchPtr[0] != inputPtr[0] || matchPtr[maxMatchedBytes - 1] != inputPtr[maxMatchedBytes - 1]) {
			continue;
		}

		int matchedBytes = 2; // 2 bytes from hashcode
		while (matchedBytes < 516 && matchPtr[matchedBytes] == inputPtr[matchedBytes]) {
			matchedBytes++;
		}
		if (matchedBytes >= maxMatchedBytes) {
			copy->offset = inputIndex - matchIndex - 1;
			maxMatchedBytes = matchedBytes;
			if (matchedBytes == 516) {
				break;
			}
		}
	}

	copy->length = maxMatchedBytes < 2 ? 0 : maxMatchedBytes;
	return;
}

static void pk_implodeWriteBits(struct PKCompBuffer *buf, unsigned char numBits, unsigned int value)
{
	if (numBits > 8) { // but never more than 16
		numBits -= 8;
		pk_implodeWriteBits(buf, 8u, value);
		value >>= 8;
	}
	int currentBitsUsed = buf->currentOutputBitsUsed;
	//int totalBits = buf->currentOutputBitsUsed + numBits;
	buf->outputData[buf->outputPtr] |= value << buf->currentOutputBitsUsed;
	buf->currentOutputBitsUsed += numBits;
	if (buf->currentOutputBitsUsed == 8) {
		buf->outputPtr++;
		buf->currentOutputBitsUsed = 0;
	} else if (buf->currentOutputBitsUsed > 8) {
		buf->outputPtr++;
		buf->outputData[buf->outputPtr] = value >> (8 - currentBitsUsed);
		buf->currentOutputBitsUsed -= 8;
	}
	if (buf->outputPtr >= 2048) {
		pk_implodeFlushFullBuffer(buf);
	}
}

static void pk_implodeFlushFullBuffer(struct PKCompBuffer *buf)
{
	buf->outputFunc(buf->outputData, 2048, buf->token);
	unsigned char newFirstByte = buf->outputData[2048];
	unsigned char lastByte = buf->outputData[buf->outputPtr];
	buf->outputPtr -= 2048;
	memset(buf->outputData, 0, 2050);
	if (buf->outputPtr) {
		buf->outputData[0] = newFirstByte;
	}
	if (buf->currentOutputBitsUsed) {
		buf->outputData[buf->outputPtr] = lastByte;
	}
}

static void pk_implodeAnalyzeInput(struct PKCompBuffer *buf, int inputStart, int inputEnd)
{
	memset(buf->analyzeOffsetTable, 0, sizeof(buf->analyzeOffsetTable));
	for (int index = inputStart; index < inputEnd; index++) {
		buf->analyzeOffsetTable[4 * buf->inputData[index] + 5 * buf->inputData[index + 1]]++;
	}

	int runningTotal = 0;
	for (int i = 0; i < 2304; i++) {
		runningTotal += buf->analyzeOffsetTable[i];
		buf->analyzeOffsetTable[i] = runningTotal;
	}

	for (int index = inputEnd - 1; index >= inputStart; index--) {
		int hashValue = 4 * buf->inputData[index] + 5 * buf->inputData[index + 1];
		unsigned short value = --buf->analyzeOffsetTable[hashValue];
		buf->analyzeIndex[value] = index;
	}
}

static int pk_explode(int (*inputFunc)(unsigned char*, int, struct PKToken*), void (*outputFunc)(unsigned char*, int, struct PKToken*), struct PKDecompBuffer *buf, struct PKToken *token)
{
	buf->inputFunc = inputFunc;
	buf->outputFunc = outputFunc;
	buf->token = token;
	buf->inputBufferPtr = 2048;
	int bytesRead = buf->inputFunc(buf->inputBuffer, buf->inputBufferPtr, buf->token);
	buf->inputBufferEnd = bytesRead;
	if (bytesRead <= 4) {
		return PK_TOO_FEW_INPUT_BYTES; // 3
	}
	int hasLiteralEncoding = buf->inputBuffer[0];
	buf->windowSize = buf->inputBuffer[1];
	buf->currentInputByte = buf->inputBuffer[2];
	buf->currentInputBitsAvailable = 0;
	buf->inputBufferPtr = 3;
	
	if (buf->windowSize < 4 || buf->windowSize > 6) {
		return PK_INVALID_WINDOWSIZE; // 1
	}
	
	buf->dictionarySize = 0xFFFF >> (16 - buf->windowSize);
	if (hasLiteralEncoding) {
		return PK_LITERAL_ENCODING_UNSUPPORTED; // 2
	}
	
	// Decode data for copying bytes
	pk_explodeConstructJumpTable(16, pk_copyLengthBaseBits, pk_copyLengthBaseCode, buf->copyLengthJumpTable);
	pk_explodeConstructJumpTable(64, pk_copyOffsetBits, pk_copyOffsetCode, buf->copyOffsetJumpTable);
	
	int result = pk_explodeData(buf);
	if (result != PK_EOF) {
		return PK_ERROR_DECODING; // 4
	}
	return PK_SUCCESS;
}

static int pk_explodeData(struct PKDecompBuffer *buf)
{
	int token;
	buf->outputBufferPtr = 4096;
	while (1) {
		token = pk_explodeDecodeNextToken(buf);
		if (token >= PK_ERROR_VALUE - 1) {
			break;
		}
		if (token >= 256) {
			// copy offset
			int length = token - 254;
			int offset = pk_explodeGetCopyOffset(buf, length);
			if (!offset) {
				token = PK_ERROR_VALUE;
				break;
			}
			unsigned char *src = &buf->outputBuffer[buf->outputBufferPtr - offset];
			unsigned char *dst = &buf->outputBuffer[buf->outputBufferPtr];
			buf->outputBufferPtr += length;
			do {
				*dst = *src;
				src++;
				dst++;
			} while (--length > 0);
		} else {
			// literal byte
			buf->outputBuffer[buf->outputBufferPtr++] = token;
		}
		if (buf->outputBufferPtr >= 8192) {
			// Flush buffer
			buf->outputFunc(&buf->outputBuffer[4096], 4096, buf->token);
			pk_memcpy(buf->outputBuffer, &buf->outputBuffer[4096], buf->outputBufferPtr - 4096);
			buf->outputBufferPtr -= 4096;
		}
	}
	// Flush buffer
	buf->outputFunc(&buf->outputBuffer[4096], buf->outputBufferPtr - 4096, buf->token);
	return token;
}

static int pk_explodeDecodeNextToken(struct PKDecompBuffer *buf)
{
	if (buf->currentInputByte & 1) {
		// copy
		if (pk_explodeSetBitsUsed(buf, 1)) {
			return PK_ERROR_VALUE;
		}
		int index = buf->copyLengthJumpTable[buf->currentInputByte & 0xff];
		if (pk_explodeSetBitsUsed(buf, pk_copyLengthBaseBits[index])) {
			return PK_ERROR_VALUE;
		}
		int extraBits = pk_copyLengthExtraBits[index];
		if (extraBits) {
			int extraBitsValue = buf->currentInputByte & ((1 << extraBits) - 1);
			if (pk_explodeSetBitsUsed(buf, extraBits) && index + extraBitsValue != 270) {
				return PK_ERROR_VALUE;
			}
			index = pk_copyLengthBaseValue[index] + extraBitsValue;
		}
		return index + 256;
	} else {
		// literal token
		if (pk_explodeSetBitsUsed(buf, 1)) {
			return PK_ERROR_VALUE;
		}
		int result = buf->currentInputByte & 0xff;
		if (pk_explodeSetBitsUsed(buf, 8)) {
			return PK_ERROR_VALUE;
		}
		return result;
	}
}

static int pk_explodeGetCopyOffset(struct PKDecompBuffer *buf, int copyLength)
{
	int index = buf->copyOffsetJumpTable[buf->currentInputByte & 0xff];
	if (pk_explodeSetBitsUsed(buf, pk_copyOffsetBits[index])) {
		return 0;
	}
	int offset;
	if (copyLength == 2) {
		offset = (buf->currentInputByte & 3) | (index << 2);
		if (pk_explodeSetBitsUsed(buf, 2)) {
			return 0;
		}
	} else {
		offset = (buf->currentInputByte & buf->dictionarySize) | (index << buf->windowSize);
		if (pk_explodeSetBitsUsed(buf, buf->windowSize)) {
			return 0;
		}
	}
	return offset + 1;
}

static int pk_explodeSetBitsUsed(struct PKDecompBuffer *buf, int numBits)
{
	if (buf->currentInputBitsAvailable >= numBits) {
		buf->currentInputBitsAvailable -= numBits;
		buf->currentInputByte = buf->currentInputByte >> numBits;
		return 0;
	}
	buf->currentInputByte = buf->currentInputByte >> buf->currentInputBitsAvailable;
	if (buf->inputBufferPtr == buf->inputBufferEnd) {
		// Fill buffer
		buf->inputBufferPtr = 2048;
		buf->inputBufferEnd = buf->inputFunc(buf->inputBuffer, buf->inputBufferPtr, buf->token);
		if (!buf->inputBufferEnd) {
			return 1;
		}
		buf->inputBufferPtr = 0;
	}

	buf->currentInputByte |= buf->inputBuffer[buf->inputBufferPtr++] << 8;
	buf->currentInputByte >>= numBits - buf->currentInputBitsAvailable;
	buf->currentInputBitsAvailable += 8 - numBits;
	return 0;
}

static void pk_explodeConstructJumpTable(int size, const unsigned char *bits, const unsigned char *codes, unsigned char *jump)
{
	for (int i = size - 1; i >= 0; i--) {
		unsigned char bit = bits[i];
		int code = codes[i];
		do {
			jump[code] = i;
			code += 1 << bit;
		} while (code < 0x100);
	}
}

static void pk_memcpy(unsigned char *dst, unsigned const char *src, int length)
{
	for (int i = 0; i < length; i++) {
		dst[i] = src[i];
	}
}

static void pk_memset(void *buffer, unsigned char fillChar, int length)
{
	memset(buffer, fillChar, length);
}

