#include "Language.h"

#include "Data/FileList.h"
#include "Data/Language.h"
#include "Data/Settings.h"

#include <stdio.h>
#include <string.h>

#define TEXT_HEADER_SIZE 28
#define TEXT_INDEX_ENTRIES 1000
#define TEXT_INDEX_SIZE 8000
#define TEXT_DATA_SIZE_MAX 200000

#define MESSAGE_INDEX_SIZE 32024
#define MESSAGE_DATA_SIZE_MAX 360000

static int fileLength(FILE *fp)
{
	fseek(fp, 0, SEEK_END);
	int length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	return length;
}

int Language_load(const char *textfile, const char *msgfile)
{
	FILE *fp = fopen(textfile, "rb");
	if (!fp) {
		return 0;
	}
	int textDataLength = fileLength(fp) - (TEXT_HEADER_SIZE + TEXT_INDEX_SIZE);
	if (textDataLength > TEXT_DATA_SIZE_MAX) {
		fclose(fp);
		return 0;
	}
	int bytesRead = 0;
	bytesRead += fread(&Data_Language_Text.header, 1, TEXT_HEADER_SIZE, fp);
	bytesRead += fread(&Data_Language_Text.index, 1, TEXT_INDEX_SIZE, fp);
	bytesRead += fread(Data_Language_Text.data, 1, textDataLength, fp);
	fclose(fp);
	if (bytesRead == 0) {
		return 0;
	}

	fp = fopen(msgfile, "rb");
	if (!fp) {
		return 0;
	}
	int msgDataLength = fileLength(fp) - MESSAGE_INDEX_SIZE;
	if (msgDataLength > MESSAGE_DATA_SIZE_MAX) {
		fclose(fp);
		return 0;
	}
	bytesRead = 0;
	bytesRead += fread(&Data_Language_Message.header, 1, MESSAGE_INDEX_SIZE, fp);
	bytesRead += fread(Data_Language_Message.data, 1, msgDataLength, fp);
	fclose(fp);
	if (bytesRead == 0) {
		return 0;
	}

	return 1;
}

void Language_loadDefaultNames()
{
	strcpy(Data_Settings.playerName, Language_getString(9, 5));
	strcpy(Data_FileList.selectedCity, Language_getString(9, 6));
	strcpy(Data_FileList.lastLoadedCity, Language_getString(9, 6));
	strcpy(Data_FileList.selectedScenario, Language_getString(9, 7));
}

const char *Language_getString(int group, int number)
{
	const char *str = &Data_Language_Text.data[Data_Language_Text.index[group].offset];
	char prev = 0;
	while (number > 0) {
		if (!*str && (prev >= ' ' || prev == 0)) { // nul byte
			--number;
		}
		prev = *str;
		++str;
	}
	while (*str < ' ') { // skip non-printables
		++str;
	}
	return str;
}
