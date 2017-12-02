#ifndef DATA_FILELIST_H
#define DATA_FILELIST_H

#define FILE_MAX 500
#define FILENAME_LENGTH 100

extern struct _Data_FileList
{
    char selectedScenario[FILENAME_LENGTH];
    char selectedPlayer[FILENAME_LENGTH];
    char lastLoadedCity[FILENAME_LENGTH];
} Data_FileList;

#endif
