#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#define FILESYSTEM_MAX_FILENAME 100

const char *FileSystem_getCaseInsensitiveFile(const char *filename);

int FileSystem_fileExists(const char *filename);

int FileSystem_readFilePartIntoBuffer(const char *filename, void *buffer, int length, int offsetInFile);

int FileSystem_readPartialRecordDataIntoBuffer(const char *filename, void *buffer, int recordReadSize, int recordSkipSize, int numRecords, int startOffset);

int FileSystem_readFileIntoBuffer(const char *filename, void *buffer, int maxSize);

int FileSystem_getFileSize(const char *filename);

int FileSystem_writeBufferToFile(const char *filename, const void *buffer, int size);

void FileSystem_changeExtension(char *filename, const char *newExtension);

void FileSystem_appendExtension(char *filename, const char *extension);

void FileSystem_removeExtension(char *filename);

int FileSystem_hasExtension(const char *filename, const char *extension);

void FileSystem_findFilesWithExtension(const char *extension);

#endif
