#ifndef DATA_FILELIST_H
#define DATA_FILELIST_H

extern struct Data_FileList {
	char files[100][100];
	int numFiles;

	char selectedScenario[100];
	char selectedCity[100];
	char selectedPlayer[100];
	char lastLoadedCity[100];
} Data_FileList;

#endif
