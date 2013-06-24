#include "FileSystem.h"

#include <stdio.h>
#include <string.h>

int FileSystem_fileExists(const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		return 0;
	}
	fclose(fp);
	return 1;
}

int FileSystem_readFilePartIntoBuffer(const char *filename, void *buffer, int length, int offsetInFile)
{
	int bytesRead = 0;
	FILE *fp = fopen(filename, "rb");
	if (fp) {
		int seekResult = fseek(fp, offsetInFile, SEEK_SET);
		if (seekResult == 0) {
			bytesRead = fread(buffer, 1, length, fp);
		}
		fclose(fp);
	}
	if (bytesRead <= 0) {
		// Try again in 555 folder
		char path[64] = "555/";
		strncpy(&path[4], filename, 60);
		path[63] = 0;
		fp = fopen(path, "rb");
		if (fp) {
			int seekResult = fseek(fp, offsetInFile, SEEK_SET);
			if (seekResult == 0) {
				bytesRead = fread(buffer, 1, length, fp);
			}
			fclose(fp);
		}
	}
	return bytesRead;
}

int FileSystem_readPartialRecordDataIntoBuffer(const char *filename, void *buffer, int recordReadSize, int recordSkipSize, int numRecords, int startOffset)
{
	int bytesRead = 0;

	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		char filename555[100] = "555/";
		strcpy(&filename555[4], filename);
		fp = fopen(filename555, "rb");
		if (!fp) {
			return 0;
		}
	}

	if (fseek(fp, startOffset, SEEK_SET) == 0) {
		for (int i = 0; i < numRecords; i++) {
			bytesRead += fread(buffer, 1, recordReadSize, fp);
			fseek(fp, recordSkipSize, SEEK_CUR);
			buffer = &((char*)buffer)[recordReadSize];
		}
	}
	fclose(fp);
	return bytesRead;
}


int FileSystem_readFileIntoBuffer(const char *filename, void *buffer)
{
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fread(buffer, 1, size, fp);
	fclose(fp);
	return 1;
}

int FileSystem_getFileSize(const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fclose(fp);
	return size;
}

int FileSystem_writeBufferToFile(const char *filename, const void *buffer, int size)
{
	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		return 0;
	}
	fwrite(buffer, 1, size, fp);
	fclose(fp);
	return 1;
}

void FileSystem_changeExtension(char *filename, const char *newExtension)
{
	char c;
	do {
		c = *filename;
		filename++;
	} while (c != '.' && c);
	if (c == '.') {
		filename[0] = newExtension[0];
		filename[1] = newExtension[1];
		filename[2] = newExtension[2];
		filename[3] = 0;
	}
}

void FileSystem_appendExtension(char *filename, const char *extension)
{
	char c;
	do {
		c = *filename;
		filename++;
	} while (c);
	filename--;
	filename[0] = '.';
	filename[1] = extension[0];
	filename[2] = extension[1];
	filename[3] = extension[2];
	filename[4] = 0;
}

void FileSystem_removeExtension(char *filename)
{
	char c;
	do {
		c = *filename;
		filename++;
	} while (c != '.' && c);
	if (c == '.') {
		filename--;
		*filename = 0;
	}
}
