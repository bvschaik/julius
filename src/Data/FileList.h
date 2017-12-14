#ifndef DATA_FILELIST_H
#define DATA_FILELIST_H

#define FILE_MAX 500
#define FILENAME_LENGTH 100

extern struct _Data_FileList {
	char lastLoadedCity[FILENAME_LENGTH];
} Data_FileList;

#endif
