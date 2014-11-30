#ifndef DATA_FILELIST_H
#define DATA_FILELIST_H

#define FILENAME_LENGTH 100

extern struct _Data_FileList {
	char files[100][FILENAME_LENGTH];
	int numFiles;

	char selectedScenario[FILENAME_LENGTH];
	char selectedCity[FILENAME_LENGTH];
	char selectedPlayer[FILENAME_LENGTH];
	char lastLoadedCity[FILENAME_LENGTH];
} Data_FileList;

#endif
